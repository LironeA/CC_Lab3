#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
#include <omp.h>

namespace fs = std::filesystem;

struct Metrics { long long count{}; long double absolute{}, squared{}, percentage{}; };
struct Row { std::string method; int window{}; Metrics metrics; };

struct Args {
    fs::path quotes, output;
    std::vector<int> windows{5, 10, 20, 30, 60, 120, 252};
    int threads = 1, timing_passes = 1;
    double test_fraction = .2;
};

std::vector<std::string> split(const std::string& text, char delimiter) {
    std::vector<std::string> fields; std::stringstream stream(text); std::string field;
    while (std::getline(stream, field, delimiter)) fields.push_back(field);
    return fields;
}

Args parse_args(int argc, char** argv) {
    Args args;
    for (int i = 1; i < argc; ++i) {
        const std::string key = argv[i];
        auto value = [&]() -> std::string { if (++i >= argc) throw std::runtime_error("Missing value for " + key); return argv[i]; };
        if (key == "--quotes") args.quotes = value();
        else if (key == "--output-dir") args.output = value();
        else if (key == "--threads") args.threads = std::stoi(value());
        else if (key == "--timing-passes") args.timing_passes = std::stoi(value());
        else if (key == "--test-fraction") args.test_fraction = std::stod(value());
        else if (key == "--windows") { args.windows.clear(); while (i + 1 < argc && std::string(argv[i + 1]).rfind("--", 0) != 0) args.windows.push_back(std::stoi(argv[++i])); }
        else throw std::runtime_error("Unknown argument: " + key);
    }
    if (args.quotes.empty() || args.output.empty() || args.threads < 1 || args.timing_passes < 1 || !(args.test_fraction > 0 && args.test_fraction < 1))
        throw std::runtime_error("Usage: forecast_omp --quotes FILE --output-dir DIR --threads N [--timing-passes N] [--windows ...]");
    return args;
}

void read_quotes(const fs::path& path, std::vector<std::string>& dates, std::vector<double>& prices) {
    std::ifstream file(path); if (!file) throw std::runtime_error("Cannot open quotes file: " + path.string());
    std::string line; std::getline(file, line);
    while (std::getline(file, line)) {
        auto fields = split(line, ',');
        if (fields.size() < 2) continue;
        try { const double price = std::stod(fields[1]); if (price > 0) { dates.push_back(fields[0]); prices.push_back(price); } }
        catch (...) {}
    }
    if (prices.size() < 20) throw std::runtime_error("The quote file has too few valid prices");
}

Metrics calculate_method(const std::vector<double>& prices, const std::vector<double>& prefix, const std::vector<double>& weighted,
                         int window, int test_start, int timing_passes, bool use_wma) {
    long long count = 0; long double absolute = 0, squared = 0, percentage = 0;
    const int samples = static_cast<int>(prices.size()) - test_start;
    #pragma omp parallel for collapse(2) schedule(static) reduction(+:count,absolute,squared,percentage)
    for (int pass = 0; pass < timing_passes; ++pass) {
        for (int position = 0; position < samples; ++position) {
            const int t = test_start + position, left = t - window;
            const double sum = prefix[t] - prefix[left];
            const double forecast = use_wma ? ((weighted[t] - weighted[left]) - left * sum) / (window * (window + 1) / 2.0) : sum / window;
            const double error = forecast - prices[t];
            count += 1; absolute += std::abs(error); squared += error * error; percentage += std::abs(error) / prices[t];
        }
    }
    return {count, absolute, squared, percentage};
}

double mae(const Metrics& value) { return static_cast<double>(value.absolute / value.count); }
void write_json_string(std::ostream& out, const std::string& value) { out << '"' << value << '"'; }

int main(int argc, char** argv) {
    try {
        Args args = parse_args(argc, argv);
        omp_set_dynamic(0); omp_set_num_threads(args.threads);
        std::vector<std::string> dates; std::vector<double> prices; read_quotes(args.quotes, dates, prices);
        std::vector<double> prefix(prices.size() + 1), weighted(prices.size() + 1);
        for (size_t i = 0; i < prices.size(); ++i) { prefix[i + 1] = prefix[i] + prices[i]; weighted[i + 1] = weighted[i] + (i + 1) * prices[i]; }
        const int maximum_window = *std::max_element(args.windows.begin(), args.windows.end());
        const int test_start = std::max(maximum_window, static_cast<int>(prices.size() * (1.0 - args.test_fraction)));
        if (test_start >= static_cast<int>(prices.size())) throw std::runtime_error("Not enough quotes for windows");
        std::sort(args.windows.begin(), args.windows.end());
        const auto started = std::chrono::steady_clock::now(); std::vector<Row> rows;
        for (const int window : args.windows) {
            for (const bool wma : {false, true}) rows.push_back({wma ? "WMA" : "SMA", window, calculate_method(prices, prefix, weighted, window, test_start, args.timing_passes, wma)});
        }
        const double seconds = std::chrono::duration<double>(std::chrono::steady_clock::now() - started).count();
        fs::create_directories(args.output);
        std::ofstream metrics(args.output / "forecast_metrics.csv"); metrics << "method,window,samples,mae,rmse,mape_percent\n" << std::setprecision(15);
        for (const auto& row : rows) {
            // Each timing pass sees identical data; normalize accumulated sums.
            const long double count = row.metrics.count / args.timing_passes;
            metrics << row.method << ',' << row.window << ',' << static_cast<long long>(count) << ',' << row.metrics.absolute / row.metrics.count << ','
                    << std::sqrt(row.metrics.squared / row.metrics.count) << ',' << 100.0L * row.metrics.percentage / row.metrics.count << '\n';
        }
        std::map<std::string, const Row*> best;
        for (const auto& row : rows) if (!best[row.method] || mae(row.metrics) < mae(best[row.method]->metrics)) best[row.method] = &row;
        std::ofstream summary(args.output / "summary.json"); summary << std::setprecision(12) << "{\n";
        summary << "  \"symbol_observations\": " << prices.size() << ",\n  \"first_date\": "; write_json_string(summary, dates.front());
        summary << ",\n  \"last_date\": "; write_json_string(summary, dates.back()); summary << ",\n  \"test_start_date\": "; write_json_string(summary, dates[test_start]);
        summary << ",\n  \"test_samples\": " << prices.size() - test_start << ",\n  \"openmp_threads\": " << args.threads << ",\n  \"timing_passes\": " << args.timing_passes << ",\n  \"elapsed_seconds\": " << seconds << ",\n  \"best_by_mae\": {\n";
        for (auto iterator = best.begin(); iterator != best.end(); ++iterator) { const Row& row = *iterator->second; const long double count = row.metrics.count / args.timing_passes;
            summary << "    \"" << row.method << "\": {\"window\": " << row.window << ", \"samples\": " << static_cast<long long>(count) << ", \"mae\": " << row.metrics.absolute / row.metrics.count << ", \"rmse\": " << std::sqrt(row.metrics.squared / row.metrics.count) << ", \"mape_percent\": " << 100.0L * row.metrics.percentage / row.metrics.count << "}" << (std::next(iterator) == best.end() ? "\n" : ",\n"); }
        summary << "  },\n  \"next_step_predictions\": {\n";
        for (auto iterator = best.begin(); iterator != best.end(); ++iterator) { const Row& row = *iterator->second; const int w = row.window; double prediction = 0;
            if (row.method == "SMA") prediction = (prefix[prices.size()] - prefix[prices.size() - w]) / w;
            else { const int left = static_cast<int>(prices.size()) - w; const double sum = prefix[prices.size()] - prefix[left]; prediction = ((weighted[prices.size()] - weighted[left]) - left * sum) / (w * (w + 1) / 2.0); }
            summary << "    \"" << row.method << "\": " << prediction << (std::next(iterator) == best.end() ? "\n" : ",\n"); }
        summary << "  }\n}\n";
        std::cout << "threads=" << args.threads << " elapsed_seconds=" << seconds << "\n";
    } catch (const std::exception& error) { std::cerr << "Error: " << error.what() << '\n'; return 1; }
}
