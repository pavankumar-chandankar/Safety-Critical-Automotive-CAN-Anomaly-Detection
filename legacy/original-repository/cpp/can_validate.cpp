// Minimal C++17 standard-ID CAN-frame validator for synthetic CSV fixtures.
// Input columns: timestamp_ms,can_id_hex,dlc,payload_bytes_hex_spaced
// This is an offline software-quality demonstration, not a vehicle safety monitor.
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

static std::string trim(const std::string& s) {
    const auto begin = s.find_first_not_of(" \t\r\n");
    if (begin == std::string::npos) return "";
    const auto end = s.find_last_not_of(" \t\r\n");
    return s.substr(begin, end - begin + 1);
}

static unsigned long parse_hex(const std::string& s, unsigned long limit) {
    const auto v = trim(s);
    if (v.empty()) throw std::invalid_argument("empty hex field");
    std::size_t used = 0;
    const unsigned long value = std::stoul(v, &used, 16);
    if (used != v.size() || value > limit)
        throw std::invalid_argument("out-of-range or malformed hex value");
    return value;
}

static unsigned long parse_decimal(const std::string& s, unsigned long limit) {
    const auto v = trim(s);
    if (v.empty()) throw std::invalid_argument("empty decimal field");
    if (!std::all_of(v.begin(), v.end(), [](unsigned char c){ return std::isdigit(c) != 0; }))
        throw std::invalid_argument("malformed decimal value");
    std::size_t used = 0;
    const auto value = std::stoul(v, &used, 10);
    if (used != v.size() || value > limit)
        throw std::invalid_argument("out-of-range decimal value");
    return value;
}

static void validate_row(const std::string& line) {
    std::stringstream input(line);
    std::vector<std::string> fields;
    std::string token;
    while (std::getline(input, token, ',')) fields.push_back(trim(token));
    if (!line.empty() && line.back() == ',') fields.emplace_back("");
    if (fields.size() != 4) throw std::invalid_argument("expected four CSV fields");
    parse_decimal(fields[0], 1000000000UL); // timestamp in milliseconds
    parse_hex(fields[1], 0x7FFUL);           // 11-bit standard CAN ID
    const auto dlc = parse_decimal(fields[2], 8UL); // classic CAN DLC
    std::stringstream bytes(fields[3]);
    std::size_t found = 0;
    while (bytes >> token) {
        parse_hex(token, 0xFFUL);
        ++found;
    }
    if (found != dlc) throw std::invalid_argument("DLC differs from payload byte count");
}

int main(int argc, char** argv) {
    if (argc != 2) {
        std::cerr << "Usage: can_validate frames.csv\n";
        return 2;
    }
    std::ifstream input(argv[1]);
    if (!input) {
        std::cerr << "Unable to open input file\n";
        return 2;
    }
    std::string line;
    std::size_t checked = 0;
    std::size_t rejected = 0;
    std::size_t line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        if (trim(line).empty()) continue;
        try {
            validate_row(line);
            ++checked;
        } catch (const std::exception& error) {
            ++rejected;
            std::cerr << "Line " << line_number << ": " << error.what() << '\n';
        }
    }
    std::cout << "valid=" << checked << " rejected=" << rejected << '\n';
    return rejected == 0 ? 0 : 1;
}
