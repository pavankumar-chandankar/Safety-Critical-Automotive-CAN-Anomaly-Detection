#include "harness.hpp"
#include "cansec/rules.hpp"
#include <sstream>
using namespace cansec;
TEST_CASE("rule parser accepts counter and checksum") {
 std::istringstream stream("0x120,std,0,8000,12000,8,0,0,0x0F,7\n");
 auto res=load_rules(stream);CHECK(res.errors.empty());CHECK(res.rules.size()==1U);
 const auto& rule=res.rules.begin()->second;CHECK(rule.counter_byte==0U);CHECK(rule.checksum_byte==7U);
}
TEST_CASE("rule parser rejects inverted periods") {
 std::istringstream stream("0x120,std,0,12000,8000,8,0,-,-,-\n");CHECK(load_rules(stream).errors.size()==1U);
}
TEST_CASE("rule parser rejects non-contiguous counter mask") {
 std::istringstream stream("0x120,std,0,0,0,8,0,0,0x0A,-\n");CHECK(load_rules(stream).errors.size()==1U);
}
TEST_CASE("rule parser rejects checksum/counter collision") {
 std::istringstream stream("0x120,std,0,0,0,8,0,7,0x0F,7\n");CHECK(load_rules(stream).errors.size()==1U);
}
TEST_CASE("rule parser rejects duplicate rules") {
 std::istringstream stream("0x120,std,0,0,0,8,0,-,-,-\n0x120,std,0,0,0,8,0,-,-,-\n");CHECK(load_rules(stream).errors.size()==1U);
}
TEST_CASE("rule parser accepts FD up to 64 bytes") {
 std::istringstream stream("0x18DAF110,ext,1,0,0,64,1,-,-,-\n");CHECK(load_rules(stream).errors.empty());
}
TEST_CASE("rule parser rejects classic more than eight bytes") {
 std::istringstream stream("0x120,std,0,0,0,12,0,-,-,-\n");CHECK(load_rules(stream).errors.size()==1U);
}
TEST_CASE("SUM8 excludes checksum byte") {
 Frame f;f.length=4;f.payload[0]=3;f.payload[1]=10;f.payload[2]=5;f.payload[3]=18;
 CHECK(sum8_excluding(f,3)==18U);
}
