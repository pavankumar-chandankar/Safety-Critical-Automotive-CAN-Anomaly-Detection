#include "cansec/report.hpp"
#include <iomanip>
#include <ostream>
#include <sstream>

namespace cansec {
std::string escape_json(const std::string& text) {
  std::ostringstream out;
  for(char character:text) {
    const auto ch=static_cast<unsigned char>(character);
    switch(ch) {
      case '"':out<<"\\\"";break;
      case '\\':out<<"\\\\";break;
      case '\n':out<<"\\n";break;
      case '\r':out<<"\\r";break;
      case '\t':out<<"\\t";break;
      default:
        if(ch<0x20U) {out<<"\\u"<<std::uppercase<<std::hex<<std::setfill('0')<<std::setw(4)<<static_cast<unsigned>(ch)<<std::dec;}
        else out<<static_cast<char>(ch);
    }
  }
  return out.str();
}
void write_json(std::ostream& out,const Analysis& analysis) {
  const auto& summary=analysis.summary;
  out<<"{\n  \"schema_version\": 1,\n  \"frames\": "<<summary.total_frames
     <<",\n  \"findings_total\": "<<summary.total_findings
     <<",\n  \"parse_errors_total\": "<<summary.parse_errors<<",\n";
  auto write_counts=[&out](const std::string& name,const std::map<std::string,std::size_t>& counts){
    out<<"  \""<<name<<"\": {";
    bool first=true;
    for(const auto& p:counts) {if(!first)out<<",";out<<"\n    \""<<escape_json(p.first)<<"\": "<<p.second;first=false;}
    if(!counts.empty())out<<'\n';
    out<<"  },\n";
  };
  write_counts("by_code",summary.findings_by_code);
  write_counts("by_message",summary.frames_by_id);
  out<<"  \"findings\": [";
  for(std::size_t i=0;i<analysis.findings.size();++i) {
    if(i) out<<',';
    const auto& f=analysis.findings[i];
    out<<"\n    {\"line\": "<<f.line<<", \"timestamp_us\": "<<f.timestamp_us
       <<", \"frame\": \""<<escape_json(f.frame_key)<<"\", \"code\": \""<<escape_json(f.code)
       <<"\", \"severity\": \""<<severity_string(f.severity)<<"\", \"description\": \""<<escape_json(f.description)<<"\"}";
  }
  if(!analysis.findings.empty())out<<'\n';
  out<<"  ],\n  \"parse_errors\": [";
  for(std::size_t i=0;i<analysis.errors.size();++i) {
    if(i)out<<',';
    const auto& e=analysis.errors[i];
    out<<"\n    {\"line\": "<<e.line<<", \"code\": \""<<escape_json(e.code)<<"\", \"description\": \""<<escape_json(e.description)<<"\"}";
  }
  if(!analysis.errors.empty())out<<'\n';
  out<<"  ]\n}\n";
}
void write_text(std::ostream& out,const Analysis& a) {
  out<<"CAN Sentinel | frames="<<a.summary.total_frames<<" findings="<<a.summary.total_findings<<" parse_errors="<<a.summary.parse_errors<<"\n";
  for(const auto& e:a.errors)out<<"parse-error line="<<e.line<<" "<<e.code<<" "<<e.description<<'\n';
  for(const auto& f:a.findings)out<<"finding line="<<f.line<<" "<<f.frame_key<<" "<<f.code<<" "<<f.description<<'\n';
}
} // namespace cansec
