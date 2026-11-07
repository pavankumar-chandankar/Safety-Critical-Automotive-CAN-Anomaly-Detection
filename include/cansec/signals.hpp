#pragma once
#include "cansec/frame.hpp"
#include <cstdint>
#include <istream>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace cansec {
enum class Endian {Intel, Motorola};
struct SignalSpec {
  std::string name;
  std::uint16_t start_bit{};
  std::uint8_t bit_length{};
  Endian endian{Endian::Intel};
  bool signed_value{};
  double factor{1.0};
  double offset{};
  double minimum{};
  double maximum{};
};
struct DecodedSignal {std::string name; std::int64_t raw{}; double physical{}; bool inside_range{};};
using MessageKey=std::pair<bool,std::uint32_t>;
using SignalCatalog=std::map<MessageKey,std::vector<SignalSpec>>;
struct CatalogResult {SignalCatalog catalog; std::vector<std::string> errors;};
std::optional<DecodedSignal> decode_signal(const Frame& frame, const SignalSpec& spec) noexcept;
std::vector<std::size_t> occupied_bits(const SignalSpec& spec, std::size_t payload_bytes);
CatalogResult load_signals(std::istream& input);
} // namespace cansec
