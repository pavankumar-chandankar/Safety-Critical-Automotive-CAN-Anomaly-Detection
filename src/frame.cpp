#include "cansec/frame.hpp"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>

namespace cansec {
std::optional<std::size_t> dlc_length(std::uint8_t dlc, bool fd) noexcept {
  constexpr std::array<std::size_t,16> fd_lengths{{0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64}};
  if (fd && dlc <= 15U) return fd_lengths[dlc];
  if (!fd && dlc <= 8U) return static_cast<std::size_t>(dlc);
  return std::nullopt;
}
std::string frame_id(const Frame& frame) {
  std::ostringstream out;
  out << (frame.extended ? "ext:" : "std:") << "0x" << std::hex << std::uppercase << frame.id;
  return out.str();
}
std::string hex_byte(std::uint8_t value) {
  std::ostringstream out;
  out << std::uppercase << std::hex << std::setw(2) << std::setfill('0') << static_cast<unsigned>(value);
  return out.str();
}
std::optional<std::uint64_t> parse_unsigned(const std::string& text, int base) noexcept {
  const auto cleaned = trim(text);
  if (cleaned.empty() || cleaned[0] == '-' || cleaned[0] == '+') return std::nullopt;
  try {
    std::size_t pos = 0;
    const int actual_base = base == 0 ? (cleaned.size() > 2U && cleaned[0] == '0' && (cleaned[1] == 'x' || cleaned[1] == 'X') ? 16 : 10) : base;
    const auto result = std::stoull(cleaned, &pos, actual_base);
    if (pos != cleaned.size()) return std::nullopt;
    return static_cast<std::uint64_t>(result);
  } catch (const std::exception&) { return std::nullopt; }
}
bool read_bounded_line(std::istream& input, std::string& line, bool& overflow, std::size_t limit) {
  line.clear();overflow=false;char character=0;
  bool consumed=false;
  while(input.get(character)) {
    consumed=true;
    if(character=='\n') return true;
    if(line.size()<limit) line.push_back(character);
    else overflow=true;
  }
  return consumed;
}
std::string trim(const std::string& text) {
  const auto first = text.find_first_not_of(" \t\r\n");
  if (first == std::string::npos) return {};
  const auto last = text.find_last_not_of(" \t\r\n");
  return text.substr(first, last - first + 1U);
}
std::vector<std::string> split_preserve_empty(const std::string& text, char separator) {
  std::vector<std::string> parts;
  std::size_t first = 0;
  while (true) {
    const auto next = text.find(separator, first);
    if (next == std::string::npos) { parts.push_back(trim(text.substr(first))); break; }
    parts.push_back(trim(text.substr(first, next-first)));
    first = next + 1U;
  }
  return parts;
}
} // namespace cansec
