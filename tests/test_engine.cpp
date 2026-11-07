#include "harness.hpp"
#include "cansec/engine.hpp"
#include <sstream>
using namespace cansec;
namespace {
std::map<MessageKey,Rule> rules(){
 std::istringstream ss("0x120,std,0,8000,12000,8,0,0,0x0F,7\n");return load_rules(ss).rules;
}
Frame frame(std::uint64_t time,std::uint8_t counter) {
 Frame f;f.timestamp_us=time;f.id=0x120;f.bus=0;f.length=8;f.dlc=8;f.payload[0]=counter;
 f.payload[7]=sum8_excluding(f,7U);return f;
}
Analysis analyze(std::vector<Frame> frames){ParseResult trace;trace.frames=std::move(frames);return Detector(rules()).inspect(trace);}
bool contains(const Analysis& result,const std::string& code){return result.summary.findings_by_code.count(code)>0U;}
}
TEST_CASE("normal rolling count with 10 millisecond intervals") {
 const auto result=analyze({frame(10000,0),frame(20000,1),frame(30000,2)});
 CHECK(result.summary.total_frames==3U);CHECK(result.summary.total_findings==0U);
}
TEST_CASE("detect unknown ID") {
 auto f=frame(10000,0);f.id=0x333U;
 CHECK(contains(analyze({f}),"UNKNOWN_ID"));
}
TEST_CASE("detect wrong bus") {
 auto f=frame(10000,0);f.bus=2U;
 CHECK(contains(analyze({f}),"WRONG_BUS"));
}
TEST_CASE("detect unexpected CAN FD") {
 auto f=frame(10000,0);f.fd=true;
 CHECK(contains(analyze({f}),"UNEXPECTED_FD"));
}
TEST_CASE("detect payload maximum exceeded") {
 auto f=frame(10000,0);f.length=9U;
 CHECK(contains(analyze({f}),"PAYLOAD_BOUND"));
}
TEST_CASE("detect short interarrival time") {
 CHECK(contains(analyze({frame(10000,0),frame(10001,1)}),"PERIOD_TOO_SHORT"));
}
TEST_CASE("detect missing heartbeat from long gap") {
 CHECK(contains(analyze({frame(10000,0),frame(80000,1)}),"PERIOD_TOO_LONG"));
}
TEST_CASE("detect rolling counter jump") {
 CHECK(contains(analyze({frame(10000,0),frame(20000,3)}),"COUNTER_JUMP"));
}
TEST_CASE("rolling counter wraps modulo sixteen") {
 CHECK(analyze({frame(10000,15),frame(20000,0)}).summary.total_findings==0U);
}
TEST_CASE("checksum mismatch") {
 auto f=frame(10000,0);f.payload[7]^=1U;
 CHECK(contains(analyze({f}),"CHECKSUM_MISMATCH"));
}
TEST_CASE("missing counter byte") {
 auto f=frame(10000,0);f.length=0U;
 CHECK(contains(analyze({f}),"COUNTER_MISSING"));
}
TEST_CASE("missing checksum byte") {
 auto f=frame(10000,0);f.length=2U;
 CHECK(contains(analyze({f}),"CHECKSUM_MISSING"));
}
TEST_CASE("global timestamp regression") {
 CHECK(contains(analyze({frame(10000,0),frame(9999,1)}),"TIMESTAMP_REGRESSION"));
}
TEST_CASE("reported signal range violation") {
 auto rule_set=rules();SignalCatalog catalog;
 SignalSpec signal{"speed",8,8,Endian::Intel,false,1.0,0.0,0.0,100.0};
 catalog[{false,0x120U}]={signal};
 auto f=frame(10000,0);f.payload[1]=200;f.payload[7]=sum8_excluding(f,7U);
 ParseResult t;t.frames={f};const auto result=Detector(rule_set,catalog).inspect(t);
 CHECK(contains(result,"SIGNAL_RANGE"));
}
TEST_CASE("short payload cannot decode signal") {
 auto rule_set=rules();SignalCatalog catalog;
 catalog[{false,0x120U}]={{"speed",32,8,Endian::Intel,false,1.0,0.0,0.0,100.0}};
 auto f=frame(10000,0);f.length=2U;
 ParseResult t;t.frames={f};const auto result=Detector(rule_set,catalog).inspect(t);
 CHECK(contains(result,"SIGNAL_UNDECODABLE"));
}
TEST_CASE("finding retention bound still counts all findings") {
 auto a=frame(10000,0);a.id=0x700U;auto b=frame(20000,1);b.id=0x701U;
 ParseResult t;t.frames={a,b};EngineOptions opts;opts.max_findings=1U;
 const auto result=Detector(rules(),{},opts).inspect(t);
 CHECK(result.summary.total_findings==2U);CHECK(result.findings.size()==1U);
}
TEST_CASE("detector resets state between runs") {
 Detector d(rules());ParseResult t;t.frames={frame(10000,0),frame(20000,1)};
 CHECK(d.inspect(t).summary.total_findings==0U);CHECK(d.inspect(t).summary.total_findings==0U);
}
TEST_CASE("parse errors are carried through analysis") {
 ParseResult t;t.errors={{7,"CSV_FIELDS","bad"}};
 const auto result=Detector(rules()).inspect(t);
 CHECK(result.summary.parse_errors==1U);CHECK(result.errors[0].line==7U);
}
TEST_CASE("unknown ID reporting can be disabled") {
 auto f=frame(10000,0);f.id=0x700U;
 ParseResult t;t.frames={f};EngineOptions opts;opts.reject_unknown_ids=false;
 CHECK(Detector(rules(),{},opts).inspect(t).summary.total_findings==0U);
}
