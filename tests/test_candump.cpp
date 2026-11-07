#include "harness.hpp"
#include "cansec/candump.hpp"
#include <sstream>
using namespace cansec;
TEST_CASE("candump standard frame can0") {
 Frame f;ParseError e;CHECK(decode_candump_line("(1700000000.123456) can0 123#AABB",1,f,e));
 CHECK(f.timestamp_us==1700000000123456ULL);CHECK(f.id==0x123U);CHECK(f.length==2U);
}
TEST_CASE("candump extended CAN FD with BRS") {
 Frame f;ParseError e;CHECK(decode_candump_line("(1700000000.123456) can1 18DAF110##1AABBCCDDEEFF001122334455",1,f,e));
 CHECK(f.extended && f.fd && f.brs && f.dlc==9U && f.length==12U);
}
TEST_CASE("candump 8-digit low ID remains extended") {
 Frame f;ParseError e;CHECK(decode_candump_line("(1700000000.123456) can0 00000123#AA",1,f,e));CHECK(f.extended);
}
TEST_CASE("candump remote request supported") {
 Frame f;ParseError e;CHECK(decode_candump_line("(1700000000.123456) can0 123#R8",1,f,e));CHECK(f.rtr && f.length==0U && f.dlc==8U);
}
TEST_CASE("candump rejects CAN FD nonrepresentable length") {
 Frame f;ParseError e;CHECK(!decode_candump_line("(1700000000.123456) can0 123##1AABBCCDDEEFF001122",1,f,e));CHECK(e.code=="CANDUMP_LENGTH");
}
TEST_CASE("candump rejects unsafe unsupported FD flags") {
 Frame f;ParseError e;CHECK(!decode_candump_line("(1700000000.123456) can0 123##3AABB",1,f,e));CHECK(e.code=="CANDUMP_FD");
}
TEST_CASE("candump rejects bad timestamp") {
 Frame f;ParseError e;CHECK(!decode_candump_line("(1700000000.123) can0 123#AABB",1,f,e));
}
TEST_CASE("candump rejects incorrect identifier width") {
 Frame f;ParseError e;CHECK(!decode_candump_line("(1700000000.123456) can0 12#AA",1,f,e));
}
TEST_CASE("candump rejects invalid bus") {
 Frame f;ParseError e;CHECK(!decode_candump_line("(1700000000.123456) can17 123#AA",1,f,e));
}
TEST_CASE("candump CSV conversion round trip") {
 std::istringstream raw("(1700000000.123456) can0 123#AABB\n(1700000000.123457) can1 18DAF110##1AABBCCDDEEFF001122334455\n");
 auto parsed=parse_candump(raw);CHECK(parsed.errors.empty());CHECK(parsed.frames.size()==2U);
 std::ostringstream csv;write_normalized_csv(csv,parsed.frames);std::istringstream generated(csv.str());
 auto decoded=TraceParser{}.read(generated);CHECK(decoded.errors.empty());CHECK(decoded.frames.size()==2U);
 CHECK(decoded.frames[0].payload[1]==parsed.frames[0].payload[1]);
}
TEST_CASE("candump parser reports wrong line and continues") {
 std::istringstream input("oops\n(1700000000.123456) can0 123#AA\n");
 auto parsed=parse_candump(input);CHECK(parsed.errors.size()==1U);CHECK(parsed.frames.size()==1U);CHECK(parsed.errors[0].line==1U);
}
TEST_CASE("oversized candump line discards rest and recovers") {
 std::istringstream input(std::string(100000U,'Z')+"\n(1700000000.123456) can0 123#AA\n");
 const auto result=parse_candump(input);CHECK(result.errors.size()==1U);CHECK(result.frames.size()==1U);
}
