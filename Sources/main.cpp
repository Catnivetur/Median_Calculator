#include "../Headers/csv_reader.hpp"
#include "../Headers/config_parser.hpp"
#include "../Headers/median_calculator.hpp"
#include <string>

namespace po = boost::program_options;


int main(int argc, char* argv[])
{
    // try {
        // Getting arguments from console
        spdlog::info("Program starting");
        po::options_description desc("options");
        desc.add_options()
            ("config,cfg", po::value<std::string>()->default_value("config.toml"), "toml file path")
            ("parallel,p", po::bool_switch()->default_value(false), "enable parallel processing");
        po::variables_map received_cmd_args;
        try {
            po::store(po::parse_command_line(argc, argv, desc, po::command_line_style::unix_style | 
                po::command_line_style::allow_long_disguise), received_cmd_args);
        }
        catch (const po::error& e) {
            spdlog::error("Command line parsing error: {}", e.what());
            std::exit(EXIT_FAILURE);
        }
        auto config_file_path = received_cmd_args["config"].as<std::string>();
        bool parallel_processing = received_cmd_args["parallel"].as<bool>();
        spdlog::info("Toml filepath is {}", config_file_path);
        spdlog::info("Parallel processing: {}", parallel_processing);
        
        std::string csv_input_dir, csv_output_dir;
        std::vector<std::string> csv_filename_mask_vec;
        
        config_parser::parses_config_file(csv_input_dir, csv_output_dir, csv_filename_mask_vec, config_file_path);
        auto metrics_arr = config_parser::get_metrics(config_file_path);
        {
            int temp = 0;
            for (const auto& metr: metrics_arr)
                temp += metr.second;
            if(!temp)
            {
                spdlog::error("No metrics selected in the config file");
                return 1;
            }
        }

        spdlog::info("Csv input directory: {}", csv_input_dir);

        // Check if input directory exists and can be read/execed
        if(!config_parser::directory_works(csv_input_dir))
        {
            std::exit(EXIT_FAILURE);
        }
        spdlog::info("Csv output directory: {}", csv_output_dir);
        
        // Checks if output directory exists, if not tries to create it where specified
        // if fails creates it in a default filepath "./examples/output"
        if (!config_parser::directory_works(csv_output_dir))
        {
            if (!config_parser::create_directory(csv_output_dir))
            {
                csv_output_dir = "./examples/output";
                if(!config_parser::create_directory(csv_output_dir))
                {
                    spdlog::error("Couldn't create output directory");
                    std::exit(EXIT_FAILURE);
                }
                else {
                    config_parser::directory_works(csv_output_dir);
                    spdlog::info("Created input directory at {}", csv_output_dir);
                }
            }
            else {
                config_parser::directory_works(csv_output_dir);
                spdlog::info("Created input directory at {}", csv_output_dir);
            }
        }
        spdlog::info("Filename masks: {}", csv_filename_mask_vec);

        // Get data from all suitable csv files
        std::vector<std::string> csv_files = config_parser::get_all_csv_filepaths(csv_input_dir, csv_filename_mask_vec);
        if (csv_files.empty())
        {
            spdlog::error("Couldn't find any csv files in the input folder");
            return 1;
        }

        spdlog::info("Metrics {}", metrics_arr);

        std::vector<std::pair<uint64_t, double>> csv_data;
        auto point1 = std::chrono::high_resolution_clock::now();
        if (!parallel_processing)
            csv_data = csv_reader::read_all_csv_files(csv_files, {"receive_ts", "price"});
        else
            csv_data = csv_reader::read_all_csv_files_parallel(csv_files, {"receive_ts", "price"});
        // std::vector<std::pair<uint64_t, double>> csv_data = csv_reader::read_all_csv_files(csv_files, {"receive_ts", "price"});
        // std::vector<std::pair<uint64_t, double>> csv_data = csv_reader::read_all_csv_files(csv_files, {"receive_ts", "price"});
        // auto point2 = std::chrono::high_resolution_clock::now();

        // std::vector<std::pair<uint64_t, double>> csv_data2 = csv_reader::read_all_csv_files_parallel(csv_files, {"receive_ts", "price"});

        // auto point3 = std::chrono::high_resolution_clock::now();

        // spdlog::info("Regular time: {}", (point2-point1).count());
        // spdlog::info("Parallel time: {}", (point3-point2).count());
        // spdlog::info("Size: {}, {}", csv_data.size(), csv_data2.size());
        std::sort(csv_data.begin(), csv_data.end());
        median_calculator::calculate_median_and_write_to_file(csv_output_dir, csv_data, metrics_arr, "result");
        
        spdlog::info("Program finished");

    // } catch (const std::exception& e) {
    //     spdlog::error(e.what());
    //     return 1;
    // }
    return 0;
}