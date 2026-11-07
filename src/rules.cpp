#include "cansec/rules.hpp"
#include <algorithm>

namespace cansec {
RuleResult load_rules(std::istream& in) {
  RuleResult result;
  std::string line;
  std::size_t line_number=0;
  while(std::getline(in,line)) {
    ++line_number;
    const auto cleaned=trim(line);
    if(cleaned.empty() || cleaned[0]=='#' || cleaned=="id,format,bus,min_period_us,max_period_us,max_bytes,allow_fd,counter_byte,counter_mask,checksum_byte") continue;
    const auto cells=split_preserve_empty(cleaned,',');
    const auto prefix="line "+std::to_string(line_number)+": ";
    if(cells.size()!=10U) {result.errors.push_back(prefix+"expected 10 columns");continue;}
    const auto id=parse_unsigned(cells[0],0);
    const bool ext=cells[1]=="ext";
    const auto bus=parse_unsigned(cells[2]);
    const auto minp=parse_unsigned(cells[3]);
    const auto maxp=parse_unsigned(cells[4]);
    const auto bytes=parse_unsigned(cells[5]);
    if((cells[1]!="std" && !ext) || !id || *id>(ext?0x1FFFFFFFULL:0x7FFULL) || !bus || *bus>15U ||
       !minp || !maxp || *maxp<*minp || !bytes || *bytes>(cells[6]=="1"?64U:8U) ||
       (cells[6]!="0" && cells[6]!="1")) {
      result.errors.push_back(prefix+"invalid rule identifiers, timing, payload bounds or flags");continue;
    }
    Rule rule;
    rule.key={ext,static_cast<std::uint32_t>(*id)};
    rule.bus=static_cast<std::uint8_t>(*bus);
    rule.min_period_us=*minp;rule.max_period_us=*maxp;
    rule.max_payload_bytes=static_cast<std::size_t>(*bytes);
    rule.allow_fd=cells[6]=="1";
    bool valid=true;
    if(cells[7]!="-") {
      const auto index=parse_unsigned(cells[7]);
      const auto mask=parse_unsigned(cells[8],0);
      if(!index || *index>=*bytes || !mask || *mask==0U || *mask>255U ||
         ((*mask & (*mask+1U))!=0U)) valid=false;
      else {rule.counter_byte=static_cast<std::uint8_t>(*index);rule.counter_mask=static_cast<std::uint8_t>(*mask);}
    } else if(cells[8]!="-") valid=false;
    if(cells[9]!="-") {
      const auto index=parse_unsigned(cells[9]);
      if(!index || *index>=*bytes) valid=false;
      else rule.checksum_byte=static_cast<std::uint8_t>(*index);
    }
    if(!valid) {result.errors.push_back(prefix+"invalid counter/checksum specification");continue;}
    if(rule.counter_byte && rule.checksum_byte && *rule.counter_byte==*rule.checksum_byte) {
      result.errors.push_back(prefix+"counter and checksum bytes overlap");continue;
    }
    if(result.rules.count(rule.key)) {result.errors.push_back(prefix+"duplicate message rule");continue;}
    result.rules.emplace(rule.key,rule);
  }
  return result;
}
std::uint8_t sum8_excluding(const Frame& frame, std::size_t excluded_byte) noexcept {
  unsigned sum=0U;
  for(std::size_t i=0;i<frame.length;++i) if(i!=excluded_byte) sum=(sum+frame.payload[i])&0xFFU;
  return static_cast<std::uint8_t>(sum);
}
} // namespace cansec
