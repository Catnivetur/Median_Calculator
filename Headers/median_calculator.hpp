
#include <spdlog/spdlog.h>
#include "spdlog/fmt/bundled/ranges.h"
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/median.hpp>
#include <boost/accumulators/statistics/mean.hpp>
#include <boost/accumulators/statistics/variance.hpp> 
#include <boost/accumulators/statistics/p_square_quantile.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include "boost/accumulators/statistics_fwd.hpp"
#include <fstream>
#include <filesystem>

namespace median_calculator
{
    
    void calculate_median_and_write_to_file(std::string &output_dir_path, std::vector<std::pair<uint64_t, double>> &csv_data, std::array<std::pair<std::string, bool>, 4> &metrics_vec, std::string filename);

}