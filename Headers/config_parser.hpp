#pragma once
#include <boost/program_options.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <spdlog/spdlog.h>
#include <toml++/toml.h>
#include <string>
#include <vector>
#include "spdlog/fmt/bundled/ranges.h"
#include <filesystem>

namespace config_parser{

    void parses_config_file(std::string &csv_input_dir, std::string &csv_output_dir, std::vector<std::string> &csv_filename_mask_vec, std::string config_file_path);

    std::vector<std::string> get_all_csv_filepaths(std::string input_dir_file_path, std::vector<std::string> &csv_filename_mask_vec);

    std::array<std::pair<std::string, bool>, 4> get_metrics(std::string config_file_path);

    bool directory_works(std::string &dir_path);

    bool create_directory(std::string &dir_path);
    
}
