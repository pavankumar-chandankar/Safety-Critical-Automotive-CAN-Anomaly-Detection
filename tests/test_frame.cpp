#include "harness.hpp"
#include "cansec/frame.hpp"
#include <limits>
using namespace cansec;
TEST_CASE("DLC classical exact mapping 0..8") {
  for(unsigned i=0;i<=8U;++i) {auto n=dlc_length(static_cast<std::uint8_t>(i),false);CHECK(n && *n==i);}
}
TEST_CASE("DLC rejects classical above eight") {for(unsigned i=9;i<=15U;++i)CHECK(!dlc_length(static_cast<std::uint8_t>(i),false));}
TEST_CASE("FD DLC lookup 0..15") {
  const std::size_t expected[]={0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64};
  for(unsigned i=0;i<16U;++i){auto n=dlc_length(static_cast<std::uint8_t>(i),true);CHECK(n && *n==expected[i]);}
}
TEST_CASE("DLC rejects out of range") {CHECK(!dlc_length(16U,true));CHECK(!dlc_length(255U,false));}
TEST_CASE("strict unsigned decimal parsing") {
  CHECK(parse_unsigned("123")==123U);CHECK(!parse_unsigned("-1"));CHECK(!parse_unsigned("12oops"));
  CHECK(!parse_unsigned(""));CHECK(!parse_unsigned("+22"));
}
TEST_CASE("hexadecimal identifiers parse") {CHECK(parse_unsigned("0x7FF",0)==2047U);CHECK(parse_unsigned("0x18DAF110",0)==0x18DAF110U);}
TEST_CASE("unsigned overflow rejected") {CHECK(!parse_unsigned("18446744073709551616"));}
TEST_CASE("split preserves trailing empty field") {
  const auto parts=split_preserve_empty("A,B,",',');CHECK(parts.size()==3U);CHECK(parts.back().empty());
}
TEST_CASE("trimming spaces and Windows CR") {CHECK(trim("  ABC\r\n")=="ABC");}
TEST_CASE("frame label includes standard or extended") {
  Frame frame;frame.id=0x120U;CHECK(frame_id(frame)=="std:0x120");frame.extended=true;CHECK(frame_id(frame)=="ext:0x120");
}
