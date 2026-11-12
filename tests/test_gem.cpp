#include "harness.hpp"
#include "cansec/gem.hpp"
#include <sstream>
#include <string>
#include <vector>

namespace {
const char* header = "timestamp,arbitration_id,dlc,data0,data1,data2,data3,data4,data5,data6,data7,label,Attack_Type\n";
std::string row(const std::string& id, int label, const std::string& type) {
  return "8.209,"+id+",8,5,0,64,5B,0A,0,FF,0,"+std::to_string(label)+","+type+"\n";
}
}
TEST_CASE("GEM-CAN parser recognizes published 29-bit brake example") {
  std::istringstream input(std::string(header)+row("18FF00F9",1,"Brake Tampering"));
  const auto parsed=cansec::read_gem_csv(input);
  CHECK(parsed.size()==1);
  CHECK(parsed[0].frame.id==0x18FF00F9U);
  CHECK(parsed[0].frame.extended);
  CHECK(parsed[0].frame.payload[2]==0x64U);
  CHECK(parsed[0].frame.timestamp_us==8'209'000U);
  CHECK(parsed[0].attack_type=="Brake Tampering");
}
TEST_CASE("GEM parser reads normal session even when Attack_Type absent") {
  std::string s="Timestamp,Arbitration_ID,DLC,data0,data1,data2,data3,data4,data5,data6,data7,Label\n";
  s+="\"127,876.661\",18FF00F9,8,5,0,4C,C3,FD,FF,FF,0,0\n";
  std::istringstream input(s);
  const auto rows=cansec::read_gem_csv(input);
  CHECK(rows.size()==1);
  CHECK(rows[0].label==0);
  CHECK(rows[0].frame.payload[2]==0x4CU);
  CHECK(rows[0].frame.timestamp_us==127'876'661'000ULL);
}
TEST_CASE("GEM parser rejects injected payload corruption") {
  std::istringstream input(std::string(header)+row("18FF00F9",1,"Brake Tampering")+"8.21,18FF00F9,8,GG,0,64,5B,0A,0,FF,0,1,Brake Tampering\n");
  bool failed=false;
  try{ (void)cansec::read_gem_csv(input); }
  catch(const std::runtime_error& e){failed=std::string(e.what()).find("line 3")!=std::string::npos;}
  CHECK(failed);
}
TEST_CASE("GEM parser rejects missing labels instead of treating them as normal") {
  std::istringstream input("timestamp,arbitration_id,dlc,data0,data1,data2,data3,data4,data5,data6,data7\n");
  bool failed=false;
  try{(void)cansec::read_gem_csv(input);}catch(const std::runtime_error&){failed=true;}
  CHECK(failed);
}
TEST_CASE("GEM parser rejects CAN FD DLC in Classical CAN recording") {
  std::istringstream input(std::string(header)+"8.209,18FF00F9,9,5,0,64,5B,0A,0,FF,0,1,DoS\n");
  bool failed=false;
  try{(void)cansec::read_gem_csv(input);}catch(const std::runtime_error&){failed=true;}
  CHECK(failed);
}
TEST_CASE("GEM normal profile flags an ID never observed in calibration data") {
  std::vector<cansec::GemRow> rows(120);
  for(std::size_t i=0;i<rows.size();++i){rows[i].frame.id=0x18FF00F9U;rows[i].frame.timestamp_us=i*10'000U;rows[i].frame.length=8;rows[i].frame.dlc=8;rows[i].frame.payload[2]=0x64;}
  cansec::NormalProfile model;
  model.fit(rows,0,80);
  cansec::Frame unexpected=rows[80].frame;
  unexpected.id=0;
  CHECK(model.score(unexpected).unfamiliar_id);
  CHECK(model.known_identifiers()==1);
}
TEST_CASE("GEM evaluation uses independent capture for attack assessment") {
  std::vector<cansec::GemRow> normal(120);
  for(std::size_t i=0;i<normal.size();++i){
    normal[i].frame.id=0x18FF00F9U;normal[i].frame.timestamp_us=i*10'000U;
    normal[i].frame.dlc=8;normal[i].frame.length=8;
  }
  auto attack=normal;
  for(std::size_t i=0;i<attack.size();++i){
    attack[i].frame.id=0;
    attack[i].label=1;
    attack[i].attack_type="DoS";
  }
  const auto results=cansec::evaluate_gem(normal,attack,0.01);
  CHECK(results.train_frames==72);
  CHECK(results.calibration_frames==24);
  CHECK(results.holdout_frames==24);
  CHECK(results.attack_session.tp==120);
  CHECK(results.by_attack_type.at("DoS").tp==120);
}
TEST_CASE("GEM evaluation rejects polluted normal baseline") {
  std::vector<cansec::GemRow> normal(120),attack(1);
  normal[40].label=1;
  bool failed=false;
  try{(void)cansec::evaluate_gem(normal,attack,0.01);}catch(const std::invalid_argument&){failed=true;}
  CHECK(failed);
}
