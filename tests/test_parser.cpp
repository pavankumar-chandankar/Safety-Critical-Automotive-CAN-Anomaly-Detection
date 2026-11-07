#include "harness.hpp"
#include "cansec/parser.hpp"
#include <sstream>
using namespace cansec;
namespace {
bool read_one(const std::string& record,Frame& frame,ParseError& err){return TraceParser::decode(record,7U,frame,err);}
}
TEST_CASE("valid standard CAN frame") {
  Frame f;ParseError e;CHECK(read_one("100,0x123,std,0,0,0,0,2,AA BB",f,e));CHECK(f.id==0x123U);CHECK(f.length==2U);CHECK(f.payload[1]==0xBBU);CHECK(f.source_line==7U);
}
TEST_CASE("valid CAN FD 64-byte payload DLC 15") {
  Frame f;ParseError e;std::string bytes;
  for(unsigned i=0;i<64U;++i){if(i)bytes+=' ';bytes+="AA";}
  CHECK(read_one("0,0x18DAF110,ext,2,1,1,0,15,"+bytes,f,e));CHECK(f.length==64U && f.fd && f.extended);
}
TEST_CASE("valid zero-byte classical CAN") {Frame f;ParseError e;CHECK(read_one("100,0x123,std,0,0,0,0,0,",f,e));CHECK(f.length==0U);}
TEST_CASE("remote frame permits empty payload") {Frame f;ParseError e;CHECK(read_one("100,0x123,std,0,0,0,1,8,",f,e));CHECK(f.rtr && f.length==0U);}
TEST_CASE("remote frame disallows nonempty payload") {Frame f;ParseError e;CHECK(!read_one("100,0x123,std,0,0,0,1,8,AA",f,e));}
TEST_CASE("BRS cannot be used with classic CAN") {Frame f;ParseError e;CHECK(!read_one("100,0x123,std,0,0,1,0,0,",f,e));CHECK(e.code=="BRS");}
TEST_CASE("CAN FD disallows remote frames") {Frame f;ParseError e;CHECK(!read_one("100,0x123,std,0,1,0,1,0,",f,e));CHECK(e.code=="RTR");}
TEST_CASE("standard identifier bound") {Frame f;ParseError e;CHECK(!read_one("100,0x800,std,0,0,0,0,0,",f,e));CHECK(e.code=="IDENTIFIER");}
TEST_CASE("extended identifier bound") {Frame f;ParseError e;CHECK(!read_one("100,0x20000000,ext,0,0,0,0,0,",f,e));}
TEST_CASE("invalid bus rejected") {Frame f;ParseError e;CHECK(!read_one("100,0x123,std,16,0,0,0,0,",f,e));CHECK(e.code=="BUS");}
TEST_CASE("negative timestamp rejected") {Frame f;ParseError e;CHECK(!read_one("-1,0x123,std,0,0,0,0,0,",f,e));}
TEST_CASE("payload rejects length mismatch") {Frame f;ParseError e;CHECK(!read_one("1,0x100,std,0,0,0,0,2,AA",f,e));CHECK(e.code=="PAYLOAD_LENGTH");}
TEST_CASE("payload rejects extra byte") {Frame f;ParseError e;CHECK(!read_one("1,0x100,std,0,0,0,0,1,AA BB",f,e));}
TEST_CASE("payload rejects nonhex values") {Frame f;ParseError e;CHECK(!read_one("1,0x100,std,0,0,0,0,1,GG",f,e));CHECK(e.code=="PAYLOAD_HEX");}
TEST_CASE("payload rejects hex with incorrect width") {Frame f;ParseError e;CHECK(!read_one("1,0x100,std,0,0,0,0,1,F",f,e));}
TEST_CASE("bad CSV field count") {Frame f;ParseError e;CHECK(!read_one("1,0x100,std,0,0,0,0,1",f,e));CHECK(e.code=="CSV_FIELDS");}
TEST_CASE("invalid format rejected") {Frame f;ParseError e;CHECK(!read_one("1,0x100,standard,0,0,0,0,1,AA",f,e));}
TEST_CASE("invalid flags rejected") {Frame f;ParseError e;CHECK(!read_one("1,0x100,std,0,true,0,0,1,AA",f,e));}
TEST_CASE("oversized record limited") {Frame f;ParseError e;CHECK(!read_one(std::string(4097U,'X'),f,e));CHECK(e.code=="RECORD_TOO_LONG");}
TEST_CASE("trace parser preserves good rows and parse errors") {
  std::istringstream ss("# heading\ntimestamp_us,id,format,bus,fd,brs,rtr,dlc,data\n100,0x100,std,0,0,0,0,0,\n100,0x800,std,0,0,0,0,0,\n");
  const auto result=TraceParser{}.read(ss);CHECK(result.frames.size()==1U);CHECK(result.errors.size()==1U);CHECK(result.frames[0].source_line==3U);
}
TEST_CASE("trace parser enforces frame cap") {
  std::istringstream ss("1,0x100,std,0,0,0,0,0,\n2,0x100,std,0,0,0,0,0,\n");
  auto result=TraceParser{}.read(ss,1U);CHECK(result.frames.size()==1U);CHECK(result.errors[0].code=="FRAME_LIMIT");
}

TEST_CASE("oversized input line does not prevent recovery") {
 std::istringstream input(std::string(100000U,'A')+"\n1,0x100,std,0,0,0,0,0,\n");
 const auto result=TraceParser{}.read(input);CHECK(result.errors.size()==1U);CHECK(result.errors[0].code=="RECORD_TOO_LONG");CHECK(result.frames.size()==1U);
}
