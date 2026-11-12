#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include <omp.h>

namespace fs = std::filesystem;

struct Metrics {
    long long count = 0;
    long double absolute_error = 0.0;
    long double squared_error = 0.0;
    long double percentage_error = 0.0;
};

struct ForecastResult {
    std::string method;
    int window = 0;
    Metrics metrics;
};

struct Arguments {
    fs::path quotes_path;
    fs::path output_directory;
    std::vector<int> windows{5, 10, 20, 30, 60, 120, 252};
    int thread_count = 1;
    int timing_passes = 1;
    double test_fraction = 0.2;
};

std::vector<std::string> split_csv_line(const std::string& line) {
    std::vector<std::string> fields;
    std::istringstream stream(line);
    std::string field;

    while (std::getline(stream, field, ',')) {
        fields.push_back(field);
    }

    return fields;
}

Arguments parse_arguments(int argc, char** argv) {
    Arguments arguments;

    for (int index = 1; index < argc; ++index) {
        const std::string option = argv[index];
        const auto next_value = [&]() -> std::string {
            if (++index >= argc) {
                throw std::runtime_error("Missing value for " + option);
            }
            return argv[index];
        };

        if (option == "--quotes") {
            arguments.quotes_path = next_value();
        } else if (option == "--output-dir") {
            arguments.output_directory = next_value();
        } else if (option == "--threads") {
            arguments.thread_count = std::stoi(next_value());
        } else if (option == "--timing-passes") {
            arguments.timing_passes = std::stoi(next_value());
        } else if (option == "--test-fraction") {
            arguments.test_fraction = std::stod(next_value());
        } else if (option == "--windows") {
            arguments.windows.clear();
            while (index + 1 < argc && std::string(argv[index + 1]).rfind("--", 0) != 0) {
                arguments.windows.push_back(std::stoi(argv[++index]));
            }
        } else {
            throw std::runtime_error("Unknown argument: " + option);
        }
    }

    const bool valid = !arguments.quotes_path.empty() && !arguments.output_directory.empty() &&
                       arguments.thread_count > 0 && arguments.timing_passes > 0 &&
                       arguments.test_fraction > 0.0 && arguments.test_fraction < 1.0 &&
                       !arguments.windows.empty();
    if (!valid) {
        throw std::runtime_error(
            "Usage: forecast_omp --quotes FILE --output-dir DIR --threads N "
            "[--timing-passes N] [--windows N ...]");
    }

    return arguments;
}

void load_quotes(const fs::path& file_path,
                 std::vector<std::string>& dates,
                 std::vector<double>& prices) {
    std::ifstream input(file_path);
    if (!input) {
        throw std::runtime_error("Cannot open quotes file: " + file_path.string());
    }

    std::string line;
    std::getline(input, line);  // CSV header: Date,Close

    while (std::getline(input, line)) {
        const auto fields = split_csv_line(line);
        if (fields.size() < 2) {
            continue;
        }

        try {
            const double close_price = std::stod(fields[1]);
            if (close_price > 0.0) {
                dates.push_back(fields[0]);
                prices.push_back(close_price);
            }
        } catch (const std::exception&) {
            // Skip a row with a missing or non-numeric close price.
        }
    }

    if (prices.size() < 20) {
        throw std::runtime_error("The quote file has too few valid prices");
    }
}

void build_prefix_sums(const std::vector<double>& prices,
                       std::vector<double>& prefix_sum,
                       std::vector<double>& weighted_prefix_sum) {
    prefix_sum.assign(prices.size() + 1, 0.0);
    weighted_prefix_sum.assign(prices.size() + 1, 0.0);

    for (size_t index = 0; index < prices.size(); ++index) {
        prefix_sum[index + 1] = prefix_sum[index] + prices[index];
        weighted_prefix_sum[index + 1] = weighted_prefix_sum[index] + (index + 1) * prices[index];
    }
}

double weighted_average(const std::vector<double>& prefix_sum,
                        const std::vector<double>& weighted_prefix_sum,
                        int left,
                        int right,
                        int window) {
    const double sum = prefix_sum[right] - prefix_sum[left];
    const double index_weighted_sum = weighted_prefix_sum[right] - weighted_prefix_sum[left];
    const double window_weighted_sum = index_weighted_sum - left * sum;
    const double total_weight = window * (window + 1) / 2.0;

    return window_weighted_sum / total_weight;
}

Metrics calculate_errors(const std::vector<double>& prices,
                         const std::vector<double>& prefix_sum,
                         const std::vector<double>& weighted_prefix_sum,
                         int window,
                         int test_start,
                         int timing_passes,
                         bool use_wma) {
    long long count = 0;
    long double absolute_error = 0.0;
    long double squared_error = 0.0;
    long double percentage_error = 0.0;
    const int test_samples = static_cast<int>(prices.size()) - test_start;

    // Every pair (pass, test date) is independent. collapse(2) gives OpenMP
    // enough iterations to distribute the small time series evenly.
    #pragma omp parallel for collapse(2) schedule(static) reduction(+:count,absolute_error,squared_error,percentage_error)
    for (int pass = 0; pass < timing_passes; ++pass) {
        for (int offset = 0; offset < test_samples; ++offset) {
            const int current = test_start + offset;
            const int left = current - window;
            const double sum = prefix_sum[current] - prefix_sum[left];
            const double forecast = use_wma
                ? weighted_average(prefix_sum, weighted_prefix_sum, left, current, window)
                : sum / window;
            const double error = forecast - prices[current];

            ++count;
            absolute_error += std::abs(error);
            squared_error += error * error;
            percentage_error += std::abs(error) / prices[current];
        }
    }

    return {count, absolute_error, squared_error, percentage_error};
}

long double mean_absolute_error(const Metrics& metrics) {
    return metrics.absolute_error / metrics.count;
}

long double root_mean_squared_error(const Metrics& metrics) {
    return std::sqrt(metrics.squared_error / metrics.count);
}

long double mean_absolute_percentage_error(const Metrics& metrics) {
    return 100.0L * metrics.percentage_error / metrics.count;
}

long long samples_per_pass(const Metrics& metrics, int timing_passes) {
    return metrics.count / timing_passes;
}

void write_metrics_csv(const fs::path& path,
                       const std::vector<ForecastResult>& results,
                       int timing_passes) {
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("Cannot write " + path.string());
    }

    output << std::setprecision(15);
    output << "method,window,samples,mae,rmse,mape_percent\n";

    for (const auto& result : results) {
        output << result.method << ',' << result.window << ','
               << samples_per_pass(result.metrics, timing_passes) << ','
               << mean_absolute_error(result.metrics) << ','
               << root_mean_squared_error(result.metrics) << ','
               << mean_absolute_percentage_error(result.metrics) << '\n';
    }
}

const ForecastResult& best_result(const std::vector<ForecastResult>& results,
                                  const std::string& method) {
    const ForecastResult* best = nullptr;

    for (const auto& result : results) {
        if (result.method != method) {
            continue;
        }
        if (best == nullptr || mean_absolute_error(result.metrics) < mean_absolute_error(best->metrics)) {
            best = &result;
        }
    }

    if (best == nullptr) {
        throw std::runtime_error("No metrics for method " + method);
    }
    return *best;
}

double next_prediction(const std::vector<double>& prices,
                       const std::vector<double>& prefix_sum,
                       const std::vector<double>& weighted_prefix_sum,
                       const ForecastResult& result) {
    const int window = result.window;
    const int right = static_cast<int>(prices.size());
    const int left = right - window;
    const double sum = prefix_sum[right] - prefix_sum[left];

    if (result.method == "SMA") {
        return sum / window;
    }
    return weighted_average(prefix_sum, weighted_prefix_sum, left, right, window);
}

void write_summary_json(const fs::path& path,
                        const std::vector<std::string>& dates,
                        const std::vector<double>& prices,
                        int test_start,
                        const Arguments& arguments,
                        double elapsed_seconds,
                        const ForecastResult& best_sma,
                        const ForecastResult& best_wma,
                        const std::vector<double>& prefix_sum,
                        const std::vector<double>& weighted_prefix_sum) {
    std::ofstream output(path);
    if (!output) {
        throw std::runtime_error("Cannot write " + path.string());
    }

    const auto write_result = [&](const ForecastResult& result) {
        output << "{\"window\": " << result.window
               << ", \"samples\": " << samples_per_pass(result.metrics, arguments.timing_passes)
               << ", \"mae\": " << mean_absolute_error(result.metrics)
               << ", \"rmse\": " << root_mean_squared_error(result.metrics)
               << ", \"mape_percent\": " << mean_absolute_percentage_error(result.metrics) << '}';
    };

    output << std::setprecision(12);
    output << "{\n"
           << "  \"symbol_observations\": " << prices.size() << ",\n"
           << "  \"first_date\": \"" << dates.front() << "\",\n"
           << "  \"last_date\": \"" << dates.back() << "\",\n"
           << "  \"test_start_date\": \"" << dates[test_start] << "\",\n"
           << "  \"test_samples\": " << prices.size() - test_start << ",\n"
           << "  \"openmp_threads\": " << arguments.thread_count << ",\n"
           << "  \"timing_passes\": " << arguments.timing_passes << ",\n"
           << "  \"elapsed_seconds\": " << elapsed_seconds << ",\n"
           << "  \"best_by_mae\": {\n"
           << "    \"SMA\": ";
    write_result(best_sma);
    output << ",\n    \"WMA\": ";
    write_result(best_wma);
    output << "\n  },\n"
           << "  \"next_step_predictions\": {\n"
           << "    \"SMA\": " << next_prediction(prices, prefix_sum, weighted_prefix_sum, best_sma) << ",\n"
           << "    \"WMA\": " << next_prediction(prices, prefix_sum, weighted_prefix_sum, best_wma) << "\n"
           << "  }\n"
           << "}\n";
}

int main(int argc, char** argv) {
    try {
        Arguments arguments = parse_arguments(argc, argv);
        omp_set_dynamic(0);
        omp_set_num_threads(arguments.thread_count);

        std::vector<std::string> dates;
        std::vector<double> prices;
        load_quotes(arguments.quotes_path, dates, prices);

        std::vector<double> prefix_sum;
        std::vector<double> weighted_prefix_sum;
        build_prefix_sums(prices, prefix_sum, weighted_prefix_sum);

        std::sort(arguments.windows.begin(), arguments.windows.end());
        const int largest_window = arguments.windows.back();
        const int test_start = std::max(largest_window,
            static_cast<int>(prices.size() * (1.0 - arguments.test_fraction)));
        if (test_start >= static_cast<int>(prices.size())) {
            throw std::runtime_error("Not enough quotes for the selected windows");
        }

        const auto started = std::chrono::steady_clock::now();
        std::vector<ForecastResult> results;

        for (const int window : arguments.windows) {
            results.push_back({"SMA", window, calculate_errors(prices, prefix_sum, weighted_prefix_sum,
                window, test_start, arguments.timing_passes, false)});
            results.push_back({"WMA", window, calculate_errors(prices, prefix_sum, weighted_prefix_sum,
                window, test_start, arguments.timing_passes, true)});
        }

        const double elapsed_seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started).count();

        fs::create_directories(arguments.output_directory);
        write_metrics_csv(arguments.output_directory / "forecast_metrics.csv", results, arguments.timing_passes);

        const auto& best_sma = best_result(results, "SMA");
        const auto& best_wma = best_result(results, "WMA");
        write_summary_json(arguments.output_directory / "summary.json", dates, prices, test_start,
            arguments, elapsed_seconds, best_sma, best_wma, prefix_sum, weighted_prefix_sum);

        std::cout << "threads=" << arguments.thread_count
                  << " elapsed_seconds=" << elapsed_seconds << '\n';
    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}
