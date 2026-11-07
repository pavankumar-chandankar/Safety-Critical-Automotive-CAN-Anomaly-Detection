#pragma once
#include "cansec/frame.hpp"
#include <istream>
#include <string>
#include <vector>

namespace cansec {
struct ParseError {std::size_t line{}; std::string code; std::string description;};
struct ParseResult {std::vector<Frame> frames; std::vector<ParseError> errors;};
class TraceParser {
 public:
  static constexpr std::size_t max_record_bytes = 4096U;
  ParseResult read(std::istream& input, std::size_t max_frames = 1'000'000U) const;
  static bool decode(const std::string& record, std::size_t line, Frame& frame, ParseError& error);
};
} // namespace cansec
