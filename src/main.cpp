#include <optional>
#include <boost/optional.hpp>
#include <boost/program_options.hpp>

int main(
    int argc,
    const char* argv[]
)
{
    namespace po = boost::program_options;

    bool help;
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

    return 0;
}