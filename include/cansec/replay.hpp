#pragma once
#include "cansec/frame.hpp"
#include <cstddef>
#include <cstdint>
#include <vector>

namespace cansec {
enum class Fault {None,Replay,CounterJump,PayloadFlip,TimestampRegression,PeriodBurst,MissingHeartbeat,ChecksumFlip,UnknownId};
struct Injection {
  Fault fault{Fault::None};
  std::size_t index{0};
  std::uint64_t seed{42};
};
std::vector<Frame> inject_fault(const std::vector<Frame>& original,const Injection& injection);
} // namespace cansec
