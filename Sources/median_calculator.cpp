#include "../Headers/median_calculator.hpp"

namespace accum = boost::accumulators;

namespace median_calculator
{
    void calculate_median_and_write_to_file(std::string &output_dir_path, std::vector<std::pair<uint64_t, double>> &csv_data, 
        std::array<std::pair<std::string, bool>, 4> &metrics_vec, std::string filename)
    {
        spdlog::info("Calculating metrics...");
        namespace fs = std::filesystem;
        accum::accumulator_set<double, accum::stats<
            accum::tag::median,
            accum::tag::mean,
            accum::tag::variance,
            accum::tag::p_square_quantile
                >> accumilator;
        accum::accumulator_set<
            double,
            accum::stats<accum::tag::p_square_quantile>
        > acc_p90(accum::quantile_probability = 0.9);

        accum::accumulator_set<
            double,
            accum::stats<accum::tag::p_square_quantile>
        > acc_p95(accum::quantile_probability = 0.95);

        accum::accumulator_set<
            double,
            accum::stats<accum::tag::p_square_quantile>
        > acc_p99(accum::quantile_probability = 0.99);
        
        // Array of ofstream pointers for each file
        std::array<std::unique_ptr<std::ofstream>, 4> ofstream_output_files;
        std::array<fs::path, 4> output_file_names;
        // Used to get rid of a loose endl in the end of the file.
        std::array<uint64_t, 4> count_for_metrics = {0,0,0,0};
        const uint32_t precision_for_output = 6;
        // Going over every metric that is set to true and writing a header into the file.
        for (uint64_t t = 0; t < metrics_vec.size(); t++)
        {
            if(!metrics_vec[t].second)
                continue;
            fs::path output_dir(output_dir_path);
            fs::path output_file = output_dir / (filename);
            output_file.replace_filename(filename + "_" + metrics_vec[t].first);
            output_file.replace_extension(".csv");
            output_file_names[t] = output_file;
            ofstream_output_files[t] = std::make_unique<std::ofstream>(output_file);
            ofstream_output_files[t]->setf(std::ios::fixed);
            ofstream_output_files[t]->precision(precision_for_output);
            *ofstream_output_files[t] << "receive_ts" << ";" << metrics_vec[t].first << "\n";
        }
        std::optional<double> last_median = 0, last_mean, last_variance;
        std::array<double, 4> last_percentiles {0,0,0,0};

        // Going over each selected metric and writing it into a separate file 
        for(uint64_t value = 0; value < csv_data.size(); value++)
        {
            // Order of metrics: 
            // {"price_median", "price_mean", "price_standart_deviation", "price_percentiles"}
            //
            // -- price_median --
            accumilator(csv_data[value].second);
            acc_p90(csv_data[value].second);
            acc_p95(csv_data[value].second);
            acc_p99(csv_data[value].second);
            uint32_t metric_num = 0;
            if(metrics_vec[metric_num].second)
            {
                double current_median = accum::median(accumilator);
                if(accum::count(accumilator)>=4){
                    if (*last_median != current_median)
                    {
                        
                        last_median = current_median;
                        *ofstream_output_files[metric_num] << csv_data[value].first << ";" << current_median;
                        *ofstream_output_files[metric_num] << "\n";
                        count_for_metrics[metric_num]++;
                    }
                }
            } metric_num++;

            // -- price_mean --
            if(metrics_vec[metric_num].second)
            {
                double current_mean = accum::mean(accumilator);
                if (!last_mean || *last_mean != current_mean)
                {
                    last_mean = current_mean;
                    *ofstream_output_files[metric_num] << csv_data[value].first << ";" << current_mean;
                    *ofstream_output_files[metric_num] << "\n";
                    count_for_metrics[metric_num]++;
                }
            } metric_num++;

            // -- price_standart_deviation --
            if(metrics_vec[metric_num].second)
            {
                double current_variance = accum::variance(accumilator);
                if (!last_variance || *last_variance != current_variance)
                {
                    last_variance = current_variance;
                    *ofstream_output_files[metric_num] << csv_data[value].first << ";" << std::sqrt(current_variance);
                    *ofstream_output_files[metric_num] << "\n";
                    count_for_metrics[metric_num]++;
                }
            } metric_num++;

            // -- price_percentiles --
            if(metrics_vec[metric_num].second)
            {
                if(accum::count(accumilator) >= 4)
                {
                    if (accum::p_square_quantile(accumilator) != last_percentiles[0] ||
                        accum::p_square_quantile(acc_p90) != last_percentiles[1] ||
                        accum::p_square_quantile(acc_p95) != last_percentiles[2] ||
                        accum::p_square_quantile(acc_p99) != last_percentiles[3])
                        {
                            last_percentiles[0] = accum::p_square_quantile(accumilator);
                            last_percentiles[1] = accum::p_square_quantile(acc_p90);
                            last_percentiles[2] = accum::p_square_quantile(acc_p95);
                            last_percentiles[3] = accum::p_square_quantile(acc_p99);
                            *ofstream_output_files[metric_num] << csv_data[value].first << ";" << accum::p_square_quantile(accumilator) << ";"
                                << accum::p_square_quantile(acc_p90) << ";" << accum::p_square_quantile(acc_p95) << ";" << accum::p_square_quantile(acc_p99);

                            *ofstream_output_files[metric_num] << "\n";
                            count_for_metrics[metric_num]++;
                        }
                }
                

            } metric_num++;
        }

        for (uint64_t t = 0; t < ofstream_output_files.size(); t++)
        {
            if (ofstream_output_files[t] == nullptr)
                continue;
            ofstream_output_files[t]->close();
            // Remove loose endlines.
            auto output_file_size = fs::file_size(output_file_names[t]);
            fs::resize_file(output_file_names[t], output_file_size - 2);
        }
        spdlog::info("Calculated all the metrics and wrote them into result files");
    }
}