#pragma once
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <spdlog/spdlog.h>
#include "spdlog/fmt/bundled/ranges.h"
#include <cstddef>
#include <variant>
#include <future>
#include <filesystem>

namespace csv_reader {
    
    std::vector<std::pair<uint64_t, double>> read_all_csv_files(std::vector<std::string>& csv_files_paths, std::array<std::string, 2> fields_to_parse);

    std::vector<std::pair<uint64_t, double>> read_csv_file (const std::string file_path, std::array<std::string, 2> fields_to_parse);

    std::vector<std::pair<uint64_t, double>> read_all_csv_files_parallel(std::vector<std::string>& csv_files_paths, std::array<std::string, 2> fields_to_parse);

    void convert_to_numeric (std::pair<uint64_t, double> &row, uint32_t index, std::string &field, bool &conversion_valid);

}