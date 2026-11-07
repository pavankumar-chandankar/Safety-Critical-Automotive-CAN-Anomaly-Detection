#include "cansec/candump.hpp"
#include <cstddef>
#include <cstdint>
#include <string>
extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data,std::size_t size) {
  if(size>4096U)return 0;
  const std::string candidate(reinterpret_cast<const char*>(data),size);
  cansec::Frame frame;cansec::ParseError error;
  (void)cansec::decode_candump_line(candidate,1U,frame,error);
  return 0;
}
