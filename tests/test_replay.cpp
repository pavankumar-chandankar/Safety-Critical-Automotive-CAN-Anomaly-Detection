#include "harness.hpp"
#include "cansec/replay.hpp"
using namespace cansec;
namespace {
std::vector<Frame> frames(){Frame f;f.id=0x120;f.timestamp_us=100;f.length=8;Frame g=f;g.timestamp_us=200;return {f,g};}
}
TEST_CASE("fault injector replay duplicates prior frame") {
 auto data=inject_fault(frames(),{Fault::Replay,1,42});CHECK(data[1].timestamp_us==100U);
}
TEST_CASE("fault injector counter jump changes first byte") {
 auto data=inject_fault(frames(),{Fault::CounterJump,1,42});CHECK(data[1].payload[0]==0x0FU);
}
TEST_CASE("fault injector payload flip reproducible") {
 auto a=inject_fault(frames(),{Fault::PayloadFlip,1,43});auto b=inject_fault(frames(),{Fault::PayloadFlip,1,43});CHECK(a[1].payload==b[1].payload);
}
TEST_CASE("fault injector timestamp regression") {
 auto data=inject_fault(frames(),{Fault::TimestampRegression,1,42});CHECK(data[1].timestamp_us==99U);
}
TEST_CASE("fault injector period burst") {
 auto data=inject_fault(frames(),{Fault::PeriodBurst,1,42});CHECK(data[1].timestamp_us==100U);
}
TEST_CASE("fault injector missing heartbeat") {
 auto data=inject_fault(frames(),{Fault::MissingHeartbeat,1,42});CHECK(data[1].timestamp_us==2000200U);
}
TEST_CASE("fault injector checksum flip") {
 auto data=inject_fault(frames(),{Fault::ChecksumFlip,1,42});CHECK(data[1].payload[7]==0xFFU);
}
TEST_CASE("fault injector unknown ID") {
 auto data=inject_fault(frames(),{Fault::UnknownId,1,42});CHECK(data[1].id==0x7FFU);
}
TEST_CASE("fault injector refuses invalid index") {
 auto original=frames();auto data=inject_fault(original,{Fault::PayloadFlip,999,42});CHECK(data[0].payload==original[0].payload);
}
