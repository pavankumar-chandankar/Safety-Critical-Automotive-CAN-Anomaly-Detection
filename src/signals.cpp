#include "cansec/signals.hpp"
#include <cmath>
#include <limits>
#include <set>
#include <sstream>

namespace cansec {
namespace {
std::optional<std::size_t> signal_bit(std::size_t start, std::size_t position, Endian endian, std::size_t total_bits) {
  std::size_t bit=start;
  if(endian==Endian::Intel) {
    if(position>total_bits || start>total_bits-position) return std::nullopt;
    bit=start+position;
  } else {
    for(std::size_t i=0;i<position;++i) {
      if(bit%8U==0U) {if(bit>total_bits || total_bits-bit<=15U) return std::nullopt; bit+=15U;}
      else --bit;
    }
  }
  if(bit>=total_bits) return std::nullopt;
  return bit;
}
std::optional<double> parse_double(const std::string& s) {
  try{std::size_t used=0;double x=std::stod(s,&used);if(used==s.size() && std::isfinite(x)) return x;}catch(...){}
  return std::nullopt;
}
}
std::vector<std::size_t> occupied_bits(const SignalSpec& spec, std::size_t payload_bytes) {
  std::vector<std::size_t> bits;
  if(spec.bit_length==0U || spec.bit_length>64U || payload_bytes>64U) return bits;
  for(std::size_t pos=0;pos<spec.bit_length;++pos) {
    const auto bit=signal_bit(spec.start_bit,pos,spec.endian,payload_bytes*8U);
    if(!bit) return {};
    bits.push_back(*bit);
  }
  return bits;
}
std::optional<DecodedSignal> decode_signal(const Frame& frame, const SignalSpec& spec) noexcept {
  if(spec.bit_length==0U || spec.bit_length>64U) return std::nullopt;
  std::uint64_t raw=0U;
  for(std::size_t pos=0;pos<spec.bit_length;++pos) {
    const auto bit=signal_bit(spec.start_bit,pos,spec.endian,frame.length*8U);
    if(!bit) return std::nullopt;
    const auto bit_value=static_cast<std::uint64_t>((frame.payload[*bit/8U] >> (*bit%8U)) & 1U);
    if(spec.endian==Endian::Intel) raw |= bit_value<<pos;
    else raw=(raw<<1U)|bit_value;
  }
  std::int64_t signed_raw=0;
  if(spec.signed_value && spec.bit_length<64U && (raw & (std::uint64_t{1} << (spec.bit_length-1U)))) {
    const auto extend=raw | (~std::uint64_t{0} << spec.bit_length);
    const auto magnitude=(~extend)+1U;
    signed_raw=-static_cast<std::int64_t>(magnitude);
  } else if(spec.signed_value && spec.bit_length==64U && (raw >> 63U)) {
    const auto magnitude=(~raw)+1U;
    if(magnitude==std::uint64_t{1}<<63U) signed_raw=std::numeric_limits<std::int64_t>::min();
    else signed_raw=-static_cast<std::int64_t>(magnitude);
  } else {
    if(raw>static_cast<std::uint64_t>(std::numeric_limits<std::int64_t>::max())) return std::nullopt;
    signed_raw=static_cast<std::int64_t>(raw);
  }
  const double value=static_cast<double>(signed_raw)*spec.factor+spec.offset;
  return DecodedSignal{spec.name,signed_raw,value,std::isfinite(value) && value>=spec.minimum && value<=spec.maximum};
}
CatalogResult load_signals(std::istream& input) {
  CatalogResult result;
  std::map<MessageKey,std::set<std::size_t>> used_bits;
  std::map<MessageKey,std::set<std::string>> used_names;
  std::string line;
  std::size_t number=0;
  while(std::getline(input,line)) {
    ++number;const auto cleaned=trim(line);
    if(cleaned.empty() || cleaned[0]=='#' || cleaned=="id,format,name,start_bit,length,endian,signed,factor,offset,min,max")continue;
    auto cells=split_preserve_empty(cleaned,',');
    const auto prefix="line "+std::to_string(number)+": ";
    if(cells.size()!=11U) {result.errors.push_back(prefix+"expected 11 columns");continue;}
    const auto id=parse_unsigned(cells[0],0);
    const bool ext=cells[1]=="ext";
    if((cells[1]!="std" && !ext) || !id || *id>(ext?0x1FFFFFFFULL:0x7FFULL)) {result.errors.push_back(prefix+"invalid identifier or frame format");continue;}
    const auto start=parse_unsigned(cells[3]);const auto len=parse_unsigned(cells[4]);
    const bool intel=cells[5]=="intel",motorola=cells[5]=="motorola";
    const auto factor=parse_double(cells[7]);const auto offset=parse_double(cells[8]);
    const auto minimum=parse_double(cells[9]);const auto maximum=parse_double(cells[10]);
    if(cells[2].empty() || !start || *start>=512U || !len || *len<1U || *len>64U ||
       (!intel && !motorola) || (cells[6]!="0" && cells[6]!="1") ||
       !factor || *factor==0 || !offset || !minimum || !maximum || *minimum>*maximum) {
       result.errors.push_back(prefix+"invalid signal definition");continue;
    }
    SignalSpec s{cells[2],static_cast<std::uint16_t>(*start),static_cast<std::uint8_t>(*len),
      intel?Endian::Intel:Endian::Motorola,cells[6]=="1",*factor,*offset,*minimum,*maximum};
    const auto key=MessageKey{ext,static_cast<std::uint32_t>(*id)};
    const auto bits=occupied_bits(s,64U);
    if(bits.size()!=s.bit_length) {result.errors.push_back(prefix+"signal exceeds maximum payload");continue;}
    bool overlap=false;
    for(auto bit:bits) if(used_bits[key].count(bit)>0U) {overlap=true;break;}
    if(overlap || used_names[key].count(s.name)>0U) {result.errors.push_back(prefix+"overlapping bits or duplicate signal name");continue;}
    for(auto bit:bits) used_bits[key].insert(bit);
    used_names[key].insert(s.name);
    result.catalog[key].push_back(s);
  }
  return result;
}
} // namespace cansec
