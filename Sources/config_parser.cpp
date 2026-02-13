#include "../Headers/config_parser.hpp"

namespace config_parser{

    std::array<std::string, 4> matric_names_arr = {"price_median", "price_mean", "price_standart_deviation", "price_percentiles"};
    
    // Takes in strings to input and output directoris and a vector of masks by reference
    // and sets them to value from config. Also takes a config filepath
    void parses_config_file(std::string &csv_input_dir, std::string &csv_output_dir, std::vector<std::string> &csv_filename_mask_vec, std::string config_file_path)
    {
        toml::table config_table;
        try 
        {
            config_table = toml::parse_file(config_file_path);
        } catch (const std::exception& e) {
            spdlog::error("Config file could not be opened for reading");
            std::exit(EXIT_FAILURE);
        }
        // toml::table config_table = toml::parse_file(config_file_path);
        if (!config_table["main"] || !config_table["main"].is_table()){
            spdlog::error("[main] couldn't be identified");
            std::exit(EXIT_FAILURE);
        }
        {
            auto input = config_table["main"]["input"];
            if(!input || !input.is_string()){
                spdlog::error("input is missing from [main] or isn't a string");
                std::exit(EXIT_FAILURE);
            }
        }
        csv_input_dir = config_table["main"]["input"].value_or("./examples/input");
        csv_output_dir = config_table["main"]["output"].value_or("./examples/output");
        {
            auto filename_mask = config_table["main"]["filename_mask"];
            if (!filename_mask || !filename_mask.is_array()){
                spdlog::error("filename_mask is missing from [main] or isn't an array");
                std::exit(EXIT_FAILURE);
            }
            for (auto&& value : *filename_mask.as_array())
            {
                if (!value.is_string()){
                    spdlog::error("non string value detected in filename_mask");
                    std::exit(EXIT_FAILURE);
                }
                csv_filename_mask_vec.push_back(*value.value<std::string>());
            }
        }
    }

    // Takes in a config file path and returns an array of pairs of metric
    // files (string) and a bool of if they are set to be calculated in cfg file
    std::array<std::pair<std::string, bool>, 4> get_metrics(std::string config_file_path)
    {
        std::array<std::pair<std::string, bool>, 4> metrics_arr;
        toml::table config_table = toml::parse_file(config_file_path);

        // auto metrics = config_table["main"]["metrics"].as_table()
        if (!config_table["main"]["metrics"] || !config_table["main"]["metrics"].is_table()){
            spdlog::error("metrics table is missing from [main]");
            std::exit(EXIT_FAILURE);
        }
        // auto* metrics = config_table["main"]["metrics"].as_table();
        
        uint16_t count = 0;
        for (const auto& metric_name : matric_names_arr) {
            // auto toml_name = config_table["main"]["output"].value_or("./examples/output");
            auto toml_metric_name = config_table["main"]["metrics"][metric_name];

            if (!toml_metric_name || !toml_metric_name.is_boolean()){
                spdlog::error("{} is missing from [main.metrics] or isn't a boolean", metric_name);
                std::exit(EXIT_FAILURE);
            }
            auto boolean_setting = toml_metric_name.value_or(false);
            metrics_arr[count].first = metric_name;
            metrics_arr[count].second = boolean_setting;
            count++;
        }
        // uint16_t count = 0;
        // for (auto&& [key, value] : *metrics)
        // {
        //     if (count >= 4)
        //         break;
        //     auto boolean_setting = value.value<bool>();
        //     metrics_arr[count].first = std::string{ key.str() };
        //     metrics_arr[count].second = *boolean_setting;
        //     count++;
        // }
        return metrics_arr;
    }
    
    // Takes in an input directory and a mask for file names.
    // Returns a vector of all the csv files in input directory.
    std::vector<std::string> get_all_csv_filepaths(std::string input_dir_file_path, std::vector<std::string> &csv_filename_mask_vec)
    {
        std::vector<std::string> csv_file_paths;
        std::string file_extension = ".csv";
        for (const auto& file : std::filesystem::directory_iterator(input_dir_file_path)) {
            if (file.is_regular_file()) {
                std::string filename = file.path().filename().string();
                std::string ext = file.path().extension().string();
                // spdlog::info("filename: {}", filename);
                if (ext == file_extension)
                {
                    if (csv_filename_mask_vec.empty())
                    {
                        csv_file_paths.push_back(file.path().string());
                    }
                    else
                    {
                        for (const auto& mask : csv_filename_mask_vec)
                        {
                            if(filename.find(mask) != std::string::npos)
                            {
                                csv_file_paths.push_back(file.path().string());
                            }
                        }
                    }
                }
            }
        }
        spdlog::info("Located {} suitable csv files: {}", csv_file_paths.size(), csv_file_paths);
        return csv_file_paths;
        // input_dir_file_path
    }

    // Takes in a directory path string.
    // Checks if a directory exists, is a directory and if it has read/exec perms.
    bool directory_works(std::string &dir_path)
    {
        if (!std::filesystem::exists(dir_path))
        {
            spdlog::error("Directory does not exist: {}", dir_path);
            return false;
        }
        if (!std::filesystem::is_directory(dir_path))
        {
            spdlog::error("Directory not found: {}", dir_path);
            std::exit(EXIT_FAILURE);
            return false;
        }
        std::filesystem::perms p = std::filesystem::status(dir_path).permissions();

        if ((p & std::filesystem::perms::owner_read)  == std::filesystem::perms::none ||
            (p & std::filesystem::perms::owner_exec)  == std::filesystem::perms::none)
        {
            spdlog::error("No read/execute permission for directory: {}", dir_path);
            return false;
        }
        return true;
    }
    // Takes in a directory path string.
    // Creates a directory.
    bool create_directory(std::string &dir_path)
    {
        try {
            std::filesystem::create_directories(dir_path);
            return true;
        } catch (const std::exception& e) {
            spdlog::error("Couldn't create directory. {}", e.what());
            return false;
        }
        // if (std::filesystem::create_directories(dir_path)) {
        //     return true;
        // } else {
        //     return false;
        // }
    }
}