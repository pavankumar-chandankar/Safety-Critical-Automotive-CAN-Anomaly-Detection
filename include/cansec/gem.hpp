#pragma once

#include "cansec/frame.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <istream>
#include <string>
#include <unordered_map>
#include <vector>

namespace cansec {

// Canonical GEM-CAN rows from Zenodo record 19161139, distinct from CANSEC trace CSV.
struct GemRow {
  Frame frame;
  int label{};
  std::string attack_type{"Normal"};
};

std::vector<GemRow> read_gem_csv(std::istream& input, std::size_t maximum_rows = 500'000);

struct Score {
  double value{};
  bool unfamiliar_id{};
  bool wrong_dlc{};
  std::size_t unfamiliar_bytes{};
  bool unexpected_rate{};
};

class NormalProfile {
public:
  void fit(const std::vector<GemRow>& ordered, std::size_t begin, std::size_t end);
  Score score(const Frame& frame);
  void reset_session();
  std::size_t known_identifiers() const noexcept { return profiles_.size(); }
private:
  struct IdProfile {
    std::size_t examples{};
    std::array<bool, 9> dlcs{};
    std::array<std::array<bool, 256>, 8> bytes{};
    double minimum_interval_seconds{};
  };
  std::unordered_map<std::uint32_t, IdProfile> profiles_;
  std::unordered_map<std::uint32_t, std::uint64_t> last_seen_;
};

struct Counts {
  std::uint64_t tp{}, fp{}, tn{}, fn{};
  void observe(bool predicted, bool actual);
};

struct GemEvaluation {
  Counts attack_session;
  Counts normal_holdout;
  double threshold{};
  std::size_t train_frames{};
  std::size_t calibration_frames{};
  std::size_t holdout_frames{};
  std::size_t attack_frames{};
  std::size_t known_ids{};
  std::unordered_map<std::string, Counts> by_attack_type;
};

GemEvaluation evaluate_gem(const std::vector<GemRow>& normal,
                           const std::vector<GemRow>& attack, double target_fpr,
                           std::ostream* predictions = nullptr);
void write_gem_report(std::ostream& out, const GemEvaluation& report);

} // namespace cansec
