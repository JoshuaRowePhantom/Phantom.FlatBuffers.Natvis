#include <optional>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <flatbuffers/reflection.h>
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
    const reflection::Field* field)
{
    switch (field->type()->base_type())
    {
    case reflection::BaseType::UType:
        return GetCppTypeName(
            schema->enums()->Get(field->type()->index())->name()->str());
        break;

    case reflection::BaseType::Obj:
        return GetCppTypeName(
            schema->objects()->Get(field->type()->index()));

    case reflection::BaseType::Vector:
        if (field->type()->element() == reflection::BaseType::Obj)
        {
            return std::format(
                "flatbuffers::Vector&lt;flatbuffers::Offset&lt;{0}&gt; &gt;",
                GetCppTypeName(
                    schema->objects()->Get(field->type()->index()))
            );
        }

        return std::format(
            "flatbuffers::Vector&lt;{0}&gt;",
            GetFieldType(field->type()->element())
        );
    }
    return "UnknownType";
}

bool IsIndirect(
    const reflection::Field* field)
{
    switch (field->type()->base_type())
    {
    case reflection::BaseType::Obj:
    case reflection::BaseType::String:
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
        return "signed long long";
    case reflection::BaseType::UByte:
        return "unsigned char";
        break;
    case reflection::BaseType::UInt:
        return "unsigned int";
        break;
    case reflection::BaseType::Union:
        return "Union";
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

            auto name = GetCppTypeName(
                object);
            
            result << R"(    <Type Name=")" << name << R"(">
        <Expand>
)";

            std::string vtableExpression = "reinterpret_cast&lt;uint16_t*&gt;(data_ - *reinterpret_cast&lt;int32_t*&gt;(&amp;data_[0]))";
            std::string vtableLengthExpression = vtableExpression + "[0]";
            
            if (object->fields())
            {
                for (auto field : *object->fields())
                {
                    std::string fieldName = field->name()->str();
                    std::string fieldOffsetExpression = std::format("{0}[{1}]", vtableExpression, field->id() + 2);
                    std::string fieldPresentExpression = std::format(
                        "({0} &gt; {1} &amp;&amp; {2} != 0)",
                        vtableLengthExpression,
                        field->id(),
                        fieldOffsetExpression);

                    // Write out an Item that displays if the field is not present.
                    result << R"(            <!-- If )" << field->name()->str() << R"( is not present -->
)";
                    result << R"(            <Item Optional="true" Name=")" << field->name()->str() << R"(" Condition=")";
                    result << "!" << fieldPresentExpression;
                    result << R"(">0</Item>
)";

                    // Write out an Item that displays if the field is present.
                    result << R"(            <!-- If )" << field->name()->str() << R"( is present -->
)";
                    result << R"(            <Item Optional="true" Name=")" << field->name()->str() << R"(" Condition=")";
                    result << fieldPresentExpression;
                    result << R"(">)";

                    // Now read the value correctly.
                    std::string fieldType = GetFieldType(field->type()->base_type());
                    std::string fieldValueExpression = std::format(
                        "*reinterpret_cast&lt;{0}*&gt;(data_ + {1})",
                        fieldType,
                        fieldOffsetExpression
                    );

                    if (field->type()->base_type() == reflection::BaseType::UType)
                    {
                        auto unionEnumerationType = GetIndirectFieldType(
                            schema,
                            field);

                        fieldValueExpression = std::format(
                            "static_cast&lt;{0}&gt;({1})",
                            unionEnumerationType,
                            fieldValueExpression);
                    }

                    if (IsIndirect(field))
                    {
                        auto indirectFieldType = GetIndirectFieldType(schema, field);

                        fieldValueExpression = std::format(
                            "reinterpret_cast&lt;{0}*&gt;(data_ + {1} + {2})",
                            indirectFieldType,
                            fieldOffsetExpression,
                            fieldValueExpression);
                    }

                    result << fieldValueExpression;

                    result << R"(</Item>
)";
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