#include "cansec/replay.hpp"
#include <algorithm>
#include <random>

namespace cansec {
std::vector<Frame> inject_fault(const std::vector<Frame>& original,const Injection& injection) {
  auto frames=original;
  if(frames.empty() || injection.index>=frames.size() || injection.fault==Fault::None)return frames;
  const auto index=injection.index;
  auto& frame=frames[index];
  std::mt19937_64 rng(injection.seed);
  switch(injection.fault) {
    case Fault::Replay: if(index>0U)frame=frames[index-1U];break;
    case Fault::CounterJump: if(frame.length>0U) frame.payload[0]^=0x0FU;break;
    case Fault::PayloadFlip: if(frame.length>0U)frame.payload[static_cast<std::size_t>(rng()%frame.length)]^=0x80U;break;
    case Fault::TimestampRegression: if(index>0U)frame.timestamp_us=frames[index-1U].timestamp_us>0U?frames[index-1U].timestamp_us-1U:0U;break;
    case Fault::PeriodBurst: if(index>0U)frame.timestamp_us=frames[index-1U].timestamp_us;break;
    case Fault::MissingHeartbeat: if(index>0U && frame.timestamp_us<=UINT64_MAX-2'000'000ULL)frame.timestamp_us+=2'000'000U;break;
    case Fault::ChecksumFlip: if(frame.length>1U)frame.payload[frame.length-1U]^=0xFFU;break;
    case Fault::UnknownId:frame.id=frame.extended?0x1FFFFFFFU:0x7FFU;break;
    case Fault::None:break;
  }
  return frames;
}
} // namespace cansec
