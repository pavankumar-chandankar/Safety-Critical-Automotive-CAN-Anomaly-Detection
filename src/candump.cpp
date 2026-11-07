#include "cansec/candump.hpp"
#include <algorithm>
#include <cctype>
#include <iomanip>
#include <limits>
#include <sstream>

namespace cansec {
namespace {
void fail(ParseError& error,std::size_t line,const char* code,const char* message){error={line,code,message};}
}
bool decode_candump_line(const std::string& text,std::size_t line,Frame& frame,ParseError& error) {
  if(text.size()>TraceParser::max_record_bytes) {fail(error,line,"RECORD_TOO_LONG","candump line too long");return false;}
  std::istringstream stream(text);
  std::string time,bus,encoded,extra;
  if(!(stream>>time>>bus>>encoded) || (stream>>extra)) {fail(error,line,"CANDUMP_FIELDS","expected timestamp, bus, frame token");return false;}
  if(time.size()<4U || time.front()!='(' || time.back()!=')') {fail(error,line,"CANDUMP_TIME","timestamp must be parenthesized");return false;}
  time=time.substr(1U,time.size()-2U);
  const auto decimal=time.find('.');
  if(decimal==std::string::npos || decimal==0U || time.size()-decimal-1U!=6U) {fail(error,line,"CANDUMP_TIME","seconds.microseconds requires six digits after decimal");return false;}
  const auto seconds=parse_unsigned(time.substr(0,decimal));
  const auto micros=parse_unsigned(time.substr(decimal+1U));
  if(!seconds || !micros || *micros>=1000000U || *seconds>(std::numeric_limits<std::uint64_t>::max()-*micros)/1000000U) {
    fail(error,line,"CANDUMP_TIME","invalid or overflowing timestamp");return false;
  }
  if(bus.size()<4U || bus.substr(0,3U)!="can") {fail(error,line,"CANDUMP_BUS","only can0..can15 interfaces are supported");return false;}
  const auto bus_number=parse_unsigned(bus.substr(3U));
  if(!bus_number || *bus_number>15U) {fail(error,line,"CANDUMP_BUS","invalid bus index");return false;}
  const auto hash=encoded.find('#');
  if(hash==std::string::npos || hash==0U) {fail(error,line,"CANDUMP_FRAME","frame must contain #");return false;}
  const auto id_token=encoded.substr(0,hash);
  if(id_token.size()!=3U && id_token.size()!=8U) {fail(error,line,"CANDUMP_ID","CAN ID must use 3 or 8 hex digits");return false;}
  for(char ch:id_token)if(std::isxdigit(static_cast<unsigned char>(ch))==0) {fail(error,line,"CANDUMP_ID","identifier must be hexadecimal");return false;}
  const auto id=parse_unsigned(id_token,16);
  const bool extended=id_token.size()==8U;
  if(!id || *id>(extended?0x1FFFFFFFULL:0x7FFULL)) {fail(error,line,"CANDUMP_ID","identifier out of range");return false;}
  Frame f;f.id=static_cast<std::uint32_t>(*id);f.extended=extended;
  f.timestamp_us=*seconds*1000000U+*micros;f.bus=static_cast<std::uint8_t>(*bus_number);f.source_line=line;
  std::string payload=encoded.substr(hash+1U);
  if(!payload.empty() && payload.front()=='#') {
    f.fd=true;payload.erase(payload.begin());
    if(payload.empty()) {fail(error,line,"CANDUMP_FD","missing FD flags nibble");return false;}
    if(payload[0]=='0')f.brs=false;
    else if(payload[0]=='1')f.brs=true;
    else {fail(error,line,"CANDUMP_FD","only FD flags 0/1 supported (ESI not supported)");return false;}
    payload.erase(payload.begin());
  } else if(!payload.empty() && payload[0]=='R') {
    f.rtr=true;
    payload.erase(payload.begin());
    if(payload.size()!=1U || payload[0]<'0' || payload[0]>'8') {fail(error,line,"CANDUMP_RTR","remote frames require R0..R8");return false;}
    f.dlc=static_cast<std::uint8_t>(payload[0]-'0');
    f.length=0U;frame=f;return true;
  }
  if(payload.size()%2U!=0U || payload.size()>128U) {fail(error,line,"CANDUMP_PAYLOAD","payload needs an even count of <=128 hex characters");return false;}
  f.length=payload.size()/2U;
  for(std::size_t i=0;i<f.length;++i) {
    const auto token=payload.substr(i*2U,2U);
    for(char ch:token)if(std::isxdigit(static_cast<unsigned char>(ch))==0) {fail(error,line,"CANDUMP_PAYLOAD","invalid payload byte");return false;}
    const auto value=parse_unsigned(token,16);
    if(!value) {fail(error,line,"CANDUMP_PAYLOAD","invalid byte");return false;}
    f.payload[i]=static_cast<std::uint8_t>(*value);
  }
  if(!f.fd) {if(f.length>8U) {fail(error,line,"CANDUMP_LENGTH","classic CAN exceeds eight bytes");return false;}
    f.dlc=static_cast<std::uint8_t>(f.length);
  } else {
    bool matched=false;
    for(std::uint8_t code=0;code<16U;++code) {
      const auto len=dlc_length(code,true);
      if(len && *len==f.length) {f.dlc=code;matched=true;break;}
    }
    if(!matched) {fail(error,line,"CANDUMP_LENGTH","CAN FD length is not DLC representable");return false;}
  }
  frame=f;return true;
}
ParseResult parse_candump(std::istream& input,std::size_t max_frames) {
  ParseResult result;std::string line;std::size_t line_number=0;
  while(true) {
    bool overflow=false;
    if(!read_bounded_line(input,line,overflow,TraceParser::max_record_bytes)) break;
    ++line_number;
    if(overflow) {result.errors.push_back({line_number,"RECORD_TOO_LONG","input line exceeded bounded record limit"});continue;}
    if(trim(line).empty() || trim(line)[0]=='#')continue;
    if(result.frames.size()>=max_frames) {result.errors.push_back({line_number,"FRAME_LIMIT","frame cap reached"});break;}
    Frame frame;ParseError error;
    if(decode_candump_line(trim(line),line_number,frame,error))result.frames.push_back(frame);
    else result.errors.push_back(error);
  }
  return result;
}
void write_normalized_csv(std::ostream& out,const std::vector<Frame>& frames) {
  out<<"timestamp_us,id,format,bus,fd,brs,rtr,dlc,data\n";
  for(const auto& frame:frames) {
    out<<frame.timestamp_us<<",0x"<<std::hex<<std::uppercase<<frame.id<<std::dec
       <<','<<(frame.extended?"ext":"std")<<','<<static_cast<unsigned>(frame.bus)<<','
       <<(frame.fd?1:0)<<','<<(frame.brs?1:0)<<','<<(frame.rtr?1:0)<<','<<static_cast<unsigned>(frame.dlc)<<',';
    for(std::size_t i=0;i<frame.length;++i) {if(i)out<<' ';out<<hex_byte(frame.payload[i]);}
    out<<'\n';
  }
}
} // namespace cansec
