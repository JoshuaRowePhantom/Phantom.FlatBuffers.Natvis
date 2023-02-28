#include <optional>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>
#include <flatbuffers/reflection.h>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>

std::string GetNatvis(
    std::span<const uint8_t> schema
)
{
    return "";
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

    std::string natvis = GetNatvis(
        std::span<const uint8_t>
        {
            reinterpret_cast<const uint8_t*>(sourceBinarySchema.data()),
            sourceBinarySchema.size()
        });

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

    po::parse_command_line(argc, argv, allOptions);

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