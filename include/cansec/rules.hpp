#pragma once
#include "cansec/frame.hpp"
#include "cansec/signals.hpp"
#include <cstdint>
#include <istream>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace cansec {
struct Rule {
  MessageKey key{};
  std::uint8_t bus{};
  std::uint64_t min_period_us{};
  std::uint64_t max_period_us{};
  std::size_t max_payload_bytes{};
  bool allow_fd{};
  std::optional<std::uint8_t> counter_byte{};
  std::uint8_t counter_mask{};
  std::optional<std::uint8_t> checksum_byte{};
};
struct RuleResult {std::map<MessageKey,Rule> rules; std::vector<std::string> errors;};
RuleResult load_rules(std::istream& in);
std::uint8_t sum8_excluding(const Frame& frame, std::size_t excluded_byte) noexcept;
} // namespace cansec
