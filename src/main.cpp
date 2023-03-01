#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <flatbuffers/reflection.h>
#include <algorithm>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <boost/algorithm/string.hpp>

std::string GetCppTypeName(
    std::string name
)
{
    boost::replace_all(
        name,
        ".",
        "::"
    );
    return name;
}

std::string GetCppTypeName(
    const reflection::Object* object
)
{
    return GetCppTypeName(
        object->name()->str());
}

std::string GetFieldType(
    reflection::BaseType type);

std::string GetNatvis(const reflection::Schema* schema);

std::string GetIndirectFieldType(
    const reflection::Schema* schema,
    const reflection::Type* type)
{
    switch (type->base_type())
    {
    case reflection::BaseType::Union:
        return "void";
        break;

    case reflection::BaseType::UType:
        return GetCppTypeName(
            schema->enums()->Get(type->index())->name()->str());
        break;

    case reflection::BaseType::Obj:
        return GetCppTypeName(
            schema->objects()->Get(type->index()));

    case reflection::BaseType::String:
        return "flatbuffers::String";

    case reflection::BaseType::Vector:
        if (type->element() == reflection::BaseType::Obj)
        {
            return std::format(
                "flatbuffers::Vector&lt;flatbuffers::Offset&lt;{0}&gt; &gt;",
                GetCppTypeName(
                    schema->objects()->Get(type->index()))
            );
        }

        return std::format(
            "flatbuffers::Vector&lt;{0}&gt;",
            GetFieldType(type->element())
        );
    }
    return "UnknownType";
}

bool IsIndirect(
    const reflection::BaseType type)
{
    switch (type)
    {
    case reflection::BaseType::Obj:
    case reflection::BaseType::String:
    case reflection::BaseType::Union:
    case reflection::BaseType::Vector:
        return true;
    default:
        return false;
    }
}

std::string GetFieldType(
    reflection::BaseType type)
{
    switch (type)
    {
    case reflection::BaseType::Bool:
    case reflection::BaseType::Byte:
        return "signed char";
        break;
    case reflection::BaseType::Double:
        return "double";
        break;
    case reflection::BaseType::Float:
        return "float";
        break;
    case reflection::BaseType::Int:
        return "signed int";
        break;
    case reflection::BaseType::Long:
        return "signed long long";
        break;
    case reflection::BaseType::Obj:
        return "signed long";
        break;
    case reflection::BaseType::Short:
        return "signed short";
        break;
    case reflection::BaseType::String:
        return "signed int";
        break;
    case reflection::BaseType::UByte:
        return "unsigned char";
        break;
    case reflection::BaseType::UInt:
        return "unsigned int";
        break;
    case reflection::BaseType::Union:
        return "signed long";
        break;
    case reflection::BaseType::UType:
        return "unsigned char";
        break;
    case reflection::BaseType::ULong:
        return "unsigned long long";
        break;
    case reflection::BaseType::UShort:
        return "unsigned short";
        break;
    case reflection::BaseType::Vector:
        return "signed long";
        break;
    }

    return "UnknownType";
}

const std::string vtableExpression = "reinterpret_cast&lt;uint16_t*&gt;(data_ - *reinterpret_cast&lt;int32_t*&gt;(&amp;data_[0]))";
const std::string vtableLengthExpression = vtableExpression + "[0]";


struct FieldExpressions
{
    std::string fieldName;
    std::string fieldOffsetExpression;
    std::string fieldPresentExpression;
    std::string fieldValueExpression;
    std::string fieldType;
    bool isStruct = false;

    FieldExpressions(
        const reflection::Schema* schema,
        const reflection::Field* field
    )
    {
        fieldName = field->name()->str();
        fieldOffsetExpression = std::format("({0}[{1}])", vtableExpression, field->id() + 2);
        fieldPresentExpression = std::format(
            "({0} &gt; ({1} * 2 + 4) &amp;&amp; {2} != 0)",
            vtableLengthExpression,
            field->id(),
            fieldOffsetExpression);

        isStruct = field->type()->base_type() == reflection::BaseType::Obj
            && schema->objects()->Get(field->type()->index())->is_struct();

        std::string fieldType = GetFieldType(field->type()->base_type());
        if (isStruct)
        {
            fieldType = GetCppTypeName(schema->objects()->Get(field->type()->index())->name()->str());
        }
        fieldValueExpression = std::format(
            "*reinterpret_cast&lt;{0}*&gt;(data_ + {1})",
            fieldType,
            fieldOffsetExpression
        );

        if (field->type()->base_type() == reflection::BaseType::UType)
        {
            auto unionEnumerationType = GetIndirectFieldType(
                schema,
                field->type());

            fieldValueExpression = std::format(
                "static_cast&lt;{0}&gt;({1})",
                unionEnumerationType,
                fieldValueExpression);
        }
        else if (IsIndirect(field->type()->base_type()) && !isStruct)
        {
            auto indirectFieldType = GetIndirectFieldType(schema, field->type());

            fieldValueExpression = std::format(
                "reinterpret_cast&lt;{0}*&gt;(data_ + {1} + {2})",
                indirectFieldType,
                fieldOffsetExpression,
                fieldValueExpression);
        }

    }
};

std::string GetNatvis(
    const reflection::Schema* schema
)
{
    std::ostringstream result;

    result << R"(<AutoVisualizer xmlns="http://schemas.microsoft.com/vstudio/debugger/natvis/2010">
)";

    if (schema->objects())
    {
        for (auto object : *schema->objects())
        {
            if (object->is_struct())
            {
                continue;
            }

            auto typeName = GetCppTypeName(
                object);
            
            result << std::format(R"(    <Type Name="{0}">
        <Expand>
)",
                typeName);

            if (object->fields())
            {
                std::vector<const reflection::Field*> sortedFields
                { 
                    object->fields()->begin(),
                    object->fields()->end(),
                };

                std::ranges::sort(
                    sortedFields.begin(),
                    sortedFields.end(),
                    [](auto left, auto right)
                    {
                        return left->id() < right->id();
                    });

                for (auto field : sortedFields)
                {
                    FieldExpressions fieldExpressions(schema, field);

                    if (field->type()->base_type() != reflection::BaseType::Union)
                    {
                        auto nonUnionItem = std::format(
                            R"(            <!-- If {0} is not present -->
            <Item Optional="true" Name="{0}" Condition="!{1}">"Not Present"</Item>
            <!-- If {0} present -->
            <Item Optional="true" Name="{0}" Condition="{1}">{2}</Item>
)",
                            field->name()->str(),
                            fieldExpressions.fieldPresentExpression,
                            fieldExpressions.fieldValueExpression);
                        result << nonUnionItem;
                    }
                    else
                    {
                        auto typeField = *std::ranges::find_if(
                            *object->fields(),
                            [&](const reflection::Field* typeField)
                        {
                            return typeField->id() == field->id() - 1;
                        });

                        auto typeFieldName = typeField->name()->str();

                        FieldExpressions typeFieldExpressions(schema, typeField);

                        auto unionEnum = schema->enums()->Get(typeField->type()->index());

                        // Write one item for each enumeration value.
                        for (auto enumValue : *unionEnum->values())
                        {
                            auto enumValueCheckExpression = std::format(
                                "({0} &amp;&amp; ({1} == {2}::{3}))",
                                typeFieldExpressions.fieldPresentExpression,
                                typeFieldExpressions.fieldValueExpression,
                                GetCppTypeName(unionEnum->name()->str()),
                                enumValue->name()->str()
                            );

                            auto conditionExpression = std::format(
                                "{0} &amp;&amp; {1}",
                                fieldExpressions.fieldPresentExpression,
                                enumValueCheckExpression
                            );

                            auto unionType = enumValue->union_type();
                            auto fieldType = GetFieldType(unionType->base_type());
                            auto fieldValueExpression = std::format(
                                "reinterpret_cast&lt;{0}*&gt;({1})",
                                fieldType,
                                fieldExpressions.fieldValueExpression
                            );
                            
                            if (IsIndirect(unionType->base_type()))
                            {
                                fieldValueExpression = std::format(
                                    "reinterpret_cast&lt;{0}*&gt;({1})",
                                    GetIndirectFieldType(schema, unionType),
                                    fieldValueExpression);
                            }

                            if (enumValue->name()->str() == "NONE")
                            {
                                fieldValueExpression = R"("Not Present")";
                                conditionExpression = std::format(
                                    "!{0} || {1}",
                                    typeFieldExpressions.fieldPresentExpression,
                                    conditionExpression
                                );
                            }

                            auto item = std::format(
                                R"(            <Item Optional="true" Name="{0}" Condition="{1}">{2}</Item>
)",
                                field->name()->str(),
                                conditionExpression,
                                fieldValueExpression);

                            result << item;
                        }
                    }
                }
            }

            result << R"(        </Expand>
    </Type>
)";
        }
    }

    result << R"(</AutoVisualizer>
)";


    return result.str();
}

void WriteNatvis(
    std::string sourceBinarySchemaPath,
    std::string outputPath
)
{
    std::ifstream sourceBinarySchemaFile(sourceBinarySchemaPath, std::ios::binary);
    std::vector<char> sourceBinarySchema(
        std::istreambuf_iterator<char>(sourceBinarySchemaFile),
        {});

    flatbuffers::Verifier verifier(
        reinterpret_cast<const uint8_t*>(sourceBinarySchema.data()),
        sourceBinarySchema.size());
    if (!verifier.VerifyBuffer<reflection::Schema>())
    {
        throw std::exception("Invalid schema");
    }

    const reflection::Schema* schema = flatbuffers::GetRoot<reflection::Schema>(
        sourceBinarySchema.data());

    std::string natvis = GetNatvis(
        schema);

    std::ofstream outputFile(outputPath);
    outputFile << natvis;
    outputFile.flush();

    if (!outputFile)
    {
        throw std::exception("Error writing output file");
    }
}

int main(
    int argc,
    const char* argv[]
)
{
    namespace po = boost::program_options;

    bool help = false;
    boost::optional<std::string> sourceBinarySchemaPath;
    boost::optional<std::string> outputPath;

    po::options_description helpOptions("Help options");
    helpOptions.add_options()
        ("help", po::bool_switch(&help), "Show help message");

    po::options_description processingOptions("Processing options");
    processingOptions.add_options()
        ("binary-schema", po::value(&sourceBinarySchemaPath)->required(), "The source binary schema file")
        ("output", po::value(&outputPath)->required(), "The output .natvis file to generate");

    po::options_description allOptions;
    allOptions
        .add(helpOptions)
        .add(processingOptions);

    auto parsedOptions = po::parse_command_line(argc, argv, allOptions);
    po::variables_map optionsMap;
    store(parsedOptions, optionsMap);
    notify(optionsMap);

    if (help)
    {
        allOptions.print(std::cout);
        return 0;
    }

    if (!sourceBinarySchemaPath
        || !outputPath)
    {
        std::cerr << "Must provide --binary-schema and --output options. Try the --help option.\n";
        return 1;
    }

    try
    {
        WriteNatvis(
            *sourceBinarySchemaPath,
            *outputPath);
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception : " << e.what() << "\n";
        return 1;
    }
    catch (...)
    {
        std::cerr << "Unknown error.\n";
        return 1;
    }

    return 0;
}