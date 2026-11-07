#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <istream>
#include <string>
#include <vector>

namespace cansec {

struct Frame {
  std::uint64_t timestamp_us{};
  std::uint32_t id{};
  bool extended{false};
  std::uint8_t bus{};
  bool fd{false};
  bool brs{false};
  bool rtr{false};
  std::uint8_t dlc{};
  std::array<std::uint8_t,64> payload{};
  std::size_t length{};
  std::size_t source_line{};
};

std::optional<std::size_t> dlc_length(std::uint8_t dlc, bool fd) noexcept;
std::string frame_id(const Frame& frame);
std::string hex_byte(std::uint8_t value);
std::optional<std::uint64_t> parse_unsigned(const std::string& text, int base = 10) noexcept;
std::string trim(const std::string& text);
// Consume one logical line using constant bounded buffering; discard overflow until newline.
bool read_bounded_line(std::istream& input, std::string& line, bool& overflow, std::size_t limit);
std::vector<std::string> split_preserve_empty(const std::string& text, char separator);

} // namespace cansec
