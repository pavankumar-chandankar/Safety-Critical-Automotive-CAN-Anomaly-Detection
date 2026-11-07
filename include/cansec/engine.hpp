#pragma once
#include "cansec/parser.hpp"
#include "cansec/rules.hpp"
#include "cansec/signals.hpp"
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace cansec {
enum class Severity {Info, Warning, Error};
struct Finding {
  std::size_t line{};
  std::uint64_t timestamp_us{};
  std::string frame_key;
  std::string code;
  Severity severity{Severity::Warning};
  std::string description;
};
struct Summary {
  std::size_t total_frames{};
  std::size_t total_findings{};
  std::size_t parse_errors{};
  std::map<std::string,std::size_t> findings_by_code;
  std::map<std::string,std::size_t> frames_by_id;
};
struct Analysis {Summary summary; std::vector<Finding> findings; std::vector<ParseError> errors;};
struct EngineOptions {
  bool reject_unknown_ids{true};
  bool check_global_timestamp_order{true};
  std::size_t max_findings{100000U};
};
class Detector {
 public:
  Detector(std::map<MessageKey,Rule> rules,SignalCatalog catalog={},EngineOptions options={});
  Analysis inspect(const ParseResult& trace);
 private:
  struct Previous {std::uint64_t timestamp_us{}; std::optional<std::uint8_t> counter{};};
  void record(Analysis& out, const Frame& frame,std::string code,Severity severity,std::string description) const;
  std::map<MessageKey,Rule> rules_;
  SignalCatalog catalog_;
  EngineOptions options_;
  std::map<std::pair<std::uint8_t,MessageKey>,Previous> previous_;
  std::optional<std::uint64_t> global_previous_;
};
std::string severity_string(Severity level);
} // namespace cansec
