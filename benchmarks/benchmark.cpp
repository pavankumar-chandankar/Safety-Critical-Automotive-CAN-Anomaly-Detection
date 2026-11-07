#include "cansec/engine.hpp"
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <map>
#include <vector>
int main(int argc,char** argv) {
  std::size_t count=200000U;
  if(argc>1) {
    const auto parsed=cansec::parse_unsigned(argv[1]);
    if(!parsed || *parsed==0U || *parsed>2000000U)return 2;
    count=static_cast<std::size_t>(*parsed);
  }
  cansec::Rule r;r.key={false,0x120U};r.bus=0;r.max_payload_bytes=8U;r.min_period_us=10000U;r.max_period_us=10000U;
  std::map<cansec::MessageKey,cansec::Rule> rules{{r.key,r}};
  cansec::ParseResult input;
  input.frames.reserve(count);
  for(std::size_t i=0;i<count;++i) {
    cansec::Frame f;f.id=0x120U;f.dlc=8U;f.length=8U;f.timestamp_us=static_cast<std::uint64_t>(i)*10000U;
    input.frames.push_back(f);
  }
  const auto start=std::chrono::steady_clock::now();
  const auto result=cansec::Detector(rules).inspect(input);
  const auto stop=std::chrono::steady_clock::now();
  const double seconds=std::chrono::duration<double>(stop-start).count();
  std::cout<<"frames="<<result.summary.total_frames<<" findings="<<result.summary.total_findings
           <<" elapsed_s="<<std::fixed<<std::setprecision(6)<<seconds<<" frames_per_second="
           <<(seconds>0?static_cast<double>(count)/seconds:0)<<'\n';
}
