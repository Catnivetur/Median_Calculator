#include "../Headers/csv_reader.hpp"

namespace csv_reader {
    // Recieves a list of csv file paths and returns parsed data of
    // fields "receive_ts" and "price" from all of the suitable files
    // as a vector of pairs of uint64_t and double
    std::vector<std::pair<uint64_t, double>> read_all_csv_files(std::vector<std::string>& csv_files_paths, std::array<std::string, 2> fields_to_parse)
    {
        spdlog::info("Opening csv files...");
        const unsigned int fields_to_parse_size = fields_to_parse.size();
        std::vector<std::pair<uint64_t, double>> data;
        unsigned int count_of_files = 0;

        // Iterating over every file
        for (const auto& file_path : csv_files_paths)
        {
            spdlog::info("Reading file: {}", file_path);
            std::ifstream file(file_path);
            if (!file.is_open())
            {
                spdlog::error("Couldn't open a file {}", file_path);
                continue;
            }
            if (file.peek() == std::ifstream::traits_type::eof())
            {
                spdlog::error("File is empty: {}", file_path);
                continue;
            }
            std::string line;
            // Array of indexes to preserve ordering 
            unsigned int fields_indexes[fields_to_parse_size];
            unsigned int num_of_wanted_fields = 0;
            unsigned int num_of_expected_fields = 0;
            // Read the fields of a csv file and find suitable ones
            {
                std::getline(file, line);
                std::stringstream ss(line);
                std::string field;
                unsigned int count_of_fields = 0;
                while (std::getline(ss, field, ';'))
                {
                    for(unsigned int t = 0; t < fields_to_parse_size; t++)
                    {
                        if (fields_to_parse[t] == field)
                            {
                                fields_indexes[t] = count_of_fields;
                                num_of_wanted_fields++;
                            }
                        }
                    count_of_fields++;
                    num_of_expected_fields++;
                }
            }
            if (num_of_wanted_fields < 2)
            {
                spdlog::error("The csv file doesn't contain requiered fields {}", file_path);
                continue;
            }
            unsigned int count_of_lines = 0;

            // Reading file line by line
            while (std::getline(file, line))
            {
                uint32_t count_of_fields = 0;
                std::stringstream ss(line);
                // Skip empty lines.
                if (line.empty())
                {
                    spdlog::error("Empty line in row {} of file {}", count_of_lines+2, file_path);
                    continue;
                }
                std::string field;
                std::vector<std::string> row(fields_to_parse_size);
                std::pair<uint64_t, double> row_converted;

                // Reading fields of a row and writing them into a vector of same name
                while (std::getline(ss, field, ';')) 
                {
                    for(unsigned int t = 0; t < fields_to_parse_size; t++)
                    {
                        if (fields_indexes[t] == count_of_fields) {
                            // Files with empty fields are skipped 
                            if (field.empty())
                            {
                                // If any rows were added to data remove them
                                if (count_of_lines)
                                    data.resize(data.size() - count_of_lines);
                                spdlog::error("One of the filds is empty in row {} of file {}", count_of_lines + 2, file_path);
                                goto skip_iteration;
                            }
                            row[t] = field;
                            bool conversion_valid = true;
                            // Converting values: timestamps to uint64, price to double
                            convert_to_numeric(row_converted, t, field, conversion_valid);
                            // If conversion fails - skip the file
                            if (!conversion_valid)
                            {
                                // If any rows were added to data remove them
                                if (count_of_lines)
                                    data.resize(data.size() - count_of_lines);
                                spdlog::error("Failed conversion in row {} of file {}", count_of_lines + 2, file_path);
                                goto skip_iteration;
                            }
                        }
                    }
                    count_of_fields++;
                }
                if (count_of_fields != num_of_expected_fields)
                    {
                        spdlog::error("Invalid CSV format: wrong field count in row {} of file {}", count_of_lines + 2, file_path);
                        goto skip_iteration;
                    }
                data.push_back(row_converted);
                count_of_lines++;
            }
            if (count_of_lines == 0)
            {
                spdlog::error("File is empty: {}", file_path);
                continue;
            }
            count_of_files++;
        skip_iteration:;
        }
        if (count_of_files == 0){
            spdlog::error("No suitable files in specified directory");
            std::exit(EXIT_FAILURE);
        }
        else
            spdlog::info("Files parsed: {}", count_of_files);
        spdlog::info("Data size: {} rows", data.size());
        return data;
    }
    
    // Parallel version of read_all_csv_files().
    // Recieves a list of csv file paths and returns parsed data of
    // fields "receive_ts" and "price" from all of the suitable files
    // as a vector of pairs of uint64_t and double
    std::vector<std::pair<uint64_t, double>> read_all_csv_files_parallel(std::vector<std::string>& csv_files_paths, std::array<std::string, 2> fields_to_parse)
    {
        spdlog::info("Opening csv files...");
        uint64_t expected_total_rows = 0;
        // Estimating total amount of rows in all files.
        for (const auto& path : csv_files_paths)
        {
            try 
            {
                auto size = std::filesystem::file_size(path);
                expected_total_rows += size/90;
            } 
            catch (std::filesystem::filesystem_error& e)
            {
                spdlog::info("{}", e.what());
            }
        }
        std::vector<std::pair<uint64_t, double>> data;
        data.reserve(expected_total_rows);
        std::mutex mutex;

        auto worker = [&](const std::string& file_path)
        {
            // Reads a csv file
            auto temp = read_csv_file(file_path, fields_to_parse);
            if(temp.empty())
                return;
            std::lock_guard<std::mutex> lock(mutex);
            // Merge of data
            data.insert(data.end(), temp.begin(), temp.end());
        };
        std::vector<std::future<void>> futures;
        // Thread for each csv file
        for (const auto& path : csv_files_paths)
            futures.emplace_back(std::async(std::launch::async, worker, path));

        for (auto& f : futures)
            f.get();
        return data;
    }

    // Recieves a string that is a path to csv file and fields that should be parsed.
    // Returns a vector of pairs (uint64, double) with data from a single file
    std::vector<std::pair<uint64_t, double>> read_csv_file (const std::string file_path, std::array<std::string, 2> fields_to_parse)
    {
        std::vector<std::pair<uint64_t, double>> file_data;
        const uint32_t fields_to_parse_size = fields_to_parse.size();
        std::ifstream file(file_path);
            if (!file.is_open())
            {
                spdlog::error("Couldn't open a file {}", file_path);
                return file_data;
            }
            if (file.peek() == std::ifstream::traits_type::eof())
            {
                spdlog::error("File is empty: {}", file_path);
                return file_data;
            }
            std::string line;
            // Array of indexex to preserve ordering 
            unsigned int fields_indexes[fields_to_parse_size];
            unsigned int num_of_wanted_fields = 0;
            unsigned int num_of_expected_fields = 0;
            // Read the fields of a csv file and find suitable ones
            {
                std::getline(file, line);
                std::stringstream ss(line);
                std::string field;
                unsigned int count_of_fields = 0;
                while (std::getline(ss, field, ';'))
                {
                    for(unsigned int t = 0; t < fields_to_parse_size; t++)
                    {
                        if (fields_to_parse[t] == field)
                            {
                                fields_indexes[t] = count_of_fields;
                                num_of_wanted_fields++;
                            }
                        }
                    count_of_fields++;
                    num_of_expected_fields++;
                }
            }
            if (num_of_wanted_fields < 2)
            {
                spdlog::error("The csv file doesn't contain requiered fields {}", file_path);
                return file_data;
            }
            unsigned int count_of_lines = 0;

            // Reading file line by line
            while (std::getline(file, line))
            {
                uint32_t count_of_fields = 0;
                std::stringstream ss(line);
                if (line.empty())
                {
                    spdlog::error("Empty line in row {} of file {}", count_of_lines+2, file_path);
                    continue;
                }
                std::string field;
                std::vector<std::string> row(fields_to_parse_size);
                std::pair<uint64_t, double> row_converted;

                // Reading fields of a row and writing them into a vector of same name
                while (std::getline(ss, field, ';')) 
                {
                    for(unsigned int t = 0; t < fields_to_parse_size; t++)
                    {
                        if (fields_indexes[t] == count_of_fields) {
                            // Files with empty fields are skipped 
                            if (field.empty())
                            {
                                // If any rows were added to data remove them
                                if (count_of_lines)
                                    file_data.resize(file_data.size() - count_of_lines);
                                spdlog::error("One of the filds is empty in row {} of file {}", count_of_lines + 2, file_path);
                                return file_data;
                            }
                            row[t] = field;
                            bool conversion_valid = true;
                            // Converting values: timestamps to uint64, price to double
                            convert_to_numeric(row_converted, t, field, conversion_valid);
                            // If conversion fails - skip the file
                            if (!conversion_valid)
                            {
                                // If any rows were added to data remove them
                                if (count_of_lines)
                                    file_data.resize(file_data.size() - count_of_lines);
                                spdlog::error("Failed conversion in row {} of file {}", count_of_lines + 2, file_path);
                                return file_data;
                            }
                        }
                    }
                    count_of_fields++;
                }
                if (count_of_fields != num_of_expected_fields)
                    {
                        spdlog::error("Invalid CSV format: wrong field count in row {} of file {}", count_of_lines + 2, file_path);
                        return file_data;
                    }
                file_data.push_back(row_converted);
                count_of_lines++;
            }
            if (count_of_lines == 0)
            {
                spdlog::error("File is empty: {}", file_path);
                return file_data;
            }
            return file_data;
    }
    
    // Takes in a pair of uint64 and double by reference, index to check 
    // the value at, a string to convert and a bool of if conversion valid.
    // Sets one of the elements of a pair to converted value. Depending on the success
    // of the conversion sets conversion_valid boolean.
    void convert_to_numeric (std::pair<uint64_t, double> &row, uint32_t index, std::string &field, bool &conversion_valid)
    {
        try {
            size_t pos = 0;
            auto temp = std::stod(field, &pos);
            if (pos != field.length())
            {
                conversion_valid = false;
                return;
            }
            if (temp < 0)
            {
                conversion_valid = false;
                return;
            }
            if (index)
            {
                row.second = temp;
            }
            else {
                row.first = static_cast<uint64_t>(temp);
            }
        }
        catch (const std::invalid_argument& e) {
            spdlog::error("Invalid argument: {}", e.what());
            conversion_valid = false;
        }
        catch (const std::out_of_range& e) {
            spdlog::error("Out of range: {}", e.what());
            conversion_valid = false;
        }
    }
}