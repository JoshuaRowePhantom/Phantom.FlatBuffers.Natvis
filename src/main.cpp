#include <optional>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <flatbuffers/reflection.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <boost/algorithm/string.hpp>

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

            auto name = object->name()->str();
            boost::replace_all(
                name,
                ".",
                "::"
            );
            
            result << R"(    <Type Name=")" << name << R"(">
)";

            result << R"(    </Type>
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