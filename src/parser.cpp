#include "cansec/parser.hpp"
#include <algorithm>
#include <sstream>

namespace cansec {
namespace {
void fail(ParseError& error, std::size_t line, const char* code, const char* why) {
  error = {line,code,why};
}
bool parse_bool(const std::string& s, bool& value) {
  if(s=="0") {value=false;return true;}
  if(s=="1") {value=true;return true;}
  return false;
}
}
bool TraceParser::decode(const std::string& record, std::size_t line, Frame& frame, ParseError& error) {
  if (record.size() > max_record_bytes) {fail(error,line,"RECORD_TOO_LONG","record exceeds 4096 bytes");return false;}
  const auto fields=split_preserve_empty(record,',');
  if(fields.size()!=9U) {fail(error,line,"CSV_FIELDS","expected nine CSV fields");return false;}
  Frame result{};
  const auto ts=parse_unsigned(fields[0]);
  if(!ts) {fail(error,line,"TIMESTAMP","timestamp_us must be unsigned decimal");return false;}
  result.timestamp_us=*ts;
  const auto id=parse_unsigned(fields[1],0);
  if(!id || *id>0x1FFFFFFFULL) {fail(error,line,"IDENTIFIER","identifier must be unsigned within 29 bits");return false;}
  result.id=static_cast<std::uint32_t>(*id);
  if(fields[2]=="std") result.extended=false;
  else if(fields[2]=="ext") result.extended=true;
  else {fail(error,line,"FORMAT","format must be std or ext");return false;}
  if (!result.extended && result.id>0x7FFU) {fail(error,line,"IDENTIFIER","standard CAN identifier exceeds 11 bits");return false;}
  const auto bus=parse_unsigned(fields[3]);
  if(!bus || *bus>15U) {fail(error,line,"BUS","bus index must be 0..15");return false;}
  result.bus=static_cast<std::uint8_t>(*bus);
  if(!parse_bool(fields[4],result.fd) || !parse_bool(fields[5],result.brs) || !parse_bool(fields[6],result.rtr)) {
    fail(error,line,"FLAGS","fd, brs, rtr must be 0 or 1");return false;
  }
  if(result.brs && !result.fd) {fail(error,line,"BRS","BRS requires CAN FD");return false;}
  if(result.fd && result.rtr) {fail(error,line,"RTR","remote frames cannot use CAN FD");return false;}
  const auto dlc=parse_unsigned(fields[7]);
  if(!dlc || *dlc>15U) {fail(error,line,"DLC","DLC must be 0..15");return false;}
  result.dlc=static_cast<std::uint8_t>(*dlc);
  const auto len=dlc_length(result.dlc,result.fd);
  if(!len) {fail(error,line,"DLC","invalid DLC for classic CAN");return false;}
  result.length=result.rtr?0U:*len;
  std::istringstream bytes(fields[8]);
  std::string token;
  std::size_t count=0;
  while (bytes >> token) {
    if(count>=result.length) {fail(error,line,"PAYLOAD_LENGTH","payload longer than DLC");return false;}
    if(token.size()!=2U) {fail(error,line,"PAYLOAD_HEX","each byte requires exactly two hex digits");return false;}
    const auto value=parse_unsigned(token,16);
    if(!value || *value>255U) {fail(error,line,"PAYLOAD_HEX","invalid payload hex byte");return false;}
    result.payload[count++]=static_cast<std::uint8_t>(*value);
  }
  if(count != result.length) {fail(error,line,"PAYLOAD_LENGTH","payload does not match DLC");return false;}
  result.source_line=line;
  frame=result;
  return true;
}
ParseResult TraceParser::read(std::istream& input, std::size_t max_frames) const {
  ParseResult result;
  std::string line;
  std::size_t line_no=0;
  while(true) {
    bool overflow=false;
    if(!read_bounded_line(input,line,overflow,max_record_bytes)) break;
    ++line_no;
    if(overflow) {result.errors.push_back({line_no,"RECORD_TOO_LONG","input line exceeded bounded record limit"});continue;}
    auto cleaned=trim(line);
    if(cleaned.empty() || cleaned[0]=='#') continue;
    if(cleaned=="timestamp_us,id,format,bus,fd,brs,rtr,dlc,data") continue;
    if(result.frames.size() >= max_frames) {result.errors.push_back({line_no,"FRAME_LIMIT","frame limit reached"}); break;}
    Frame frame{}; ParseError error{};
    if(decode(cleaned,line_no,frame,error)) result.frames.push_back(frame);
    else result.errors.push_back(error);
  }
  return result;
}
} // namespace cansec
