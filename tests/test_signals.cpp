#include "harness.hpp"
#include "cansec/signals.hpp"
#include <sstream>
#include <limits>
using namespace cansec;
namespace {
SignalSpec signal(std::string name,unsigned start,unsigned bits,Endian endian,bool signed_value=false,double factor=1.0,double offset=0.0,double min=-10000,double max=10000){
 return SignalSpec{std::move(name),static_cast<std::uint16_t>(start),static_cast<std::uint8_t>(bits),endian,signed_value,factor,offset,min,max};
}
}
TEST_CASE("Intel 16 bit little endian") {
  Frame f;f.length=2U;f.payload[0]=0x34U;f.payload[1]=0x12U;
  const auto r=decode_signal(f,signal("value",0,16,Endian::Intel));CHECK(r && r->raw==0x1234);
}
TEST_CASE("Intel bit offsets across bytes") {
  Frame f;f.length=2U;f.payload[0]=0xF0U;f.payload[1]=0x0FU;
  const auto r=decode_signal(f,signal("value",4,8,Endian::Intel));CHECK(r && r->raw==0xFF);
}
TEST_CASE("Motorola whole byte start-bit seven") {
  Frame f;f.length=1U;f.payload[0]=0xA5U;
  const auto r=decode_signal(f,signal("value",7,8,Endian::Motorola));CHECK(r && r->raw==0xA5);
}
TEST_CASE("Motorola sawtooth across bytes") {
  Frame f;f.length=2U;f.payload[0]=0x12U;f.payload[1]=0x34U;
  const auto r=decode_signal(f,signal("value",7,16,Endian::Motorola));CHECK(r && r->raw==0x1234);
}
TEST_CASE("signed Intel 8-bit negative") {
  Frame f;f.length=1U;f.payload[0]=0xFEU;
  const auto r=decode_signal(f,signal("value",0,8,Endian::Intel,true));CHECK(r && r->raw==-2);
}
TEST_CASE("signed Motorola 8-bit negative") {
  Frame f;f.length=1U;f.payload[0]=0x80U;
  const auto r=decode_signal(f,signal("value",7,8,Endian::Motorola,true));CHECK(r && r->raw==-128);
}
TEST_CASE("signed 64-bit minimum is decoded") {
  Frame f;f.length=8U;f.payload[7]=0x80U;
  const auto r=decode_signal(f,signal("value",0,64,Endian::Intel,true));CHECK(r && r->raw==std::numeric_limits<std::int64_t>::min());
}
TEST_CASE("unsigned 64-bit overflow is explicitly rejected") {
  Frame f;f.length=8U;f.payload[7]=0x80U;
  CHECK(!decode_signal(f,signal("value",0,64,Endian::Intel)));
}
TEST_CASE("scaled physical engineering units") {
  Frame f;f.length=1U;f.payload[0]=40U;
  const auto r=decode_signal(f,signal("temp",0,8,Endian::Intel,false,0.5,-30.0));CHECK(r && r->physical==-10.0);
}
TEST_CASE("physical range violation reported") {
  Frame f;f.length=1U;f.payload[0]=200U;
  const auto r=decode_signal(f,signal("temp",0,8,Endian::Intel,false,1.0,0.0,0.0,100.0));CHECK(r && !r->inside_range);
}
TEST_CASE("signal outside payload rejected") {
  Frame f;f.length=1U;CHECK(!decode_signal(f,signal("invalid",7,16,Endian::Motorola)));
}
TEST_CASE("catalog rejects signal overlaps") {
  std::istringstream stream("0x100,std,x,0,8,intel,0,1,0,0,255\n0x100,std,y,4,4,intel,0,1,0,0,15\n");
  const auto res=load_signals(stream);CHECK(res.catalog.size()==1U);CHECK(res.catalog.begin()->second.size()==1U);CHECK(res.errors.size()==1U);
}
TEST_CASE("catalog rejects duplicate names") {
  std::istringstream stream("0x100,std,x,0,8,intel,0,1,0,0,255\n0x100,std,x,8,8,intel,0,1,0,0,255\n");
  const auto res=load_signals(stream);CHECK(res.errors.size()==1U);
}
TEST_CASE("catalog rejects invalid factor") {
  std::istringstream stream("0x100,std,x,0,8,intel,0,0,0,0,255\n");
  CHECK(load_signals(stream).errors.size()==1U);
}
TEST_CASE("catalog rejects signal footprint beyond 64 bytes") {
  std::istringstream stream("0x100,std,x,504,8,motorola,0,1,0,0,255\n");
  CHECK(load_signals(stream).errors.size()==1U);
}
TEST_CASE("signal footprint contains unique bits") {
  const auto bits=occupied_bits(signal("x",11,12,Endian::Motorola),64U);
  CHECK(bits.size()==12U);
  for(std::size_t i=0;i<bits.size();++i)for(std::size_t j=i+1U;j<bits.size();++j) CHECK(bits[i]!=bits[j]);
}
