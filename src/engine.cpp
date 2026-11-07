#include "cansec/engine.hpp"
#include <sstream>
#include <utility>

namespace cansec {
Detector::Detector(std::map<MessageKey,Rule> rules,SignalCatalog catalog,EngineOptions options):
  rules_(std::move(rules)),catalog_(std::move(catalog)),options_(options) {}

void Detector::record(Analysis& out,const Frame& frame,std::string code,Severity severity,std::string description) const {
  ++out.summary.total_findings;
  ++out.summary.findings_by_code[code];
  if(out.findings.size()<options_.max_findings) out.findings.push_back({frame.source_line,frame.timestamp_us,frame_id(frame),std::move(code),severity,std::move(description)});
}
Analysis Detector::inspect(const ParseResult& trace) {
  previous_.clear();global_previous_.reset();
  Analysis out;
  out.errors=trace.errors;
  out.summary.parse_errors=out.errors.size();
  for(const auto& frame:trace.frames) {
    ++out.summary.total_frames;
    ++out.summary.frames_by_id[frame_id(frame)];
    if(global_previous_ && frame.timestamp_us<*global_previous_ && options_.check_global_timestamp_order)
      record(out,frame,"TIMESTAMP_REGRESSION",Severity::Error,"capture timestamps went backwards");
    global_previous_=frame.timestamp_us;
    const MessageKey key{frame.extended,frame.id};
    const auto rule_it=rules_.find(key);
    if(rule_it==rules_.end()) {
      if(options_.reject_unknown_ids) record(out,frame,"UNKNOWN_ID",Severity::Warning,"message identifier not present in rule pack");
      continue;
    }
    const auto& rule=rule_it->second;
    if(frame.bus!=rule.bus) record(out,frame,"WRONG_BUS",Severity::Error,"message observed on an unexpected bus");
    if(frame.fd && !rule.allow_fd) record(out,frame,"UNEXPECTED_FD",Severity::Warning,"CAN FD not enabled in rule pack");
    if(frame.length>rule.max_payload_bytes) record(out,frame,"PAYLOAD_BOUND",Severity::Error,"payload exceeds rule's maximum length");
    const auto state_key=std::make_pair(frame.bus,key);
    auto state_it=previous_.find(state_key);
    if(state_it!=previous_.end()) {
      const auto previous=state_it->second.timestamp_us;
      if(frame.timestamp_us>=previous) {
        const auto elapsed=frame.timestamp_us-previous;
        if(rule.min_period_us>0U && elapsed<rule.min_period_us)
          record(out,frame,"PERIOD_TOO_SHORT",Severity::Warning,"interarrival time below configured lower bound");
        if(rule.max_period_us>0U && elapsed>rule.max_period_us)
          record(out,frame,"PERIOD_TOO_LONG",Severity::Warning,"interarrival time exceeds configured upper bound");
      }
    }
    std::optional<std::uint8_t> new_counter;
    if(rule.counter_byte && *rule.counter_byte<frame.length) {
      new_counter=static_cast<std::uint8_t>(frame.payload[*rule.counter_byte] & rule.counter_mask);
      if(state_it!=previous_.end() && state_it->second.counter) {
        const auto expected=static_cast<std::uint8_t>((*state_it->second.counter+1U)&rule.counter_mask);
        if(*new_counter!=expected) record(out,frame,"COUNTER_JUMP",Severity::Warning,"rolling counter did not advance by one");
      }
    } else if(rule.counter_byte && !frame.rtr) {
      record(out,frame,"COUNTER_MISSING",Severity::Error,"counter byte is absent from payload");
    }
    if(rule.checksum_byte) {
      if(*rule.checksum_byte>=frame.length) {
        if(!frame.rtr) record(out,frame,"CHECKSUM_MISSING",Severity::Error,"checksum byte is absent from payload");
      } else if(frame.payload[*rule.checksum_byte]!=sum8_excluding(frame,*rule.checksum_byte)) {
        record(out,frame,"CHECKSUM_MISMATCH",Severity::Warning,"toy SUM8 checksum mismatch");
      }
    }
    const auto sig_it=catalog_.find(key);
    if(sig_it!=catalog_.end()) {
      for(const auto& spec:sig_it->second) {
        const auto decoded=decode_signal(frame,spec);
        if(!decoded) record(out,frame,"SIGNAL_UNDECODABLE",Severity::Warning,"signal "+spec.name+" does not fit actual payload");
        else if(!decoded->inside_range) record(out,frame,"SIGNAL_RANGE",Severity::Warning,"signal "+spec.name+" violates configured engineering limits");
      }
    }
    // Per-message timestamps are refreshed even for out-of-order events; anomaly is reported above.
    previous_[state_key]=Previous{frame.timestamp_us,new_counter};
  }
  return out;
}
std::string severity_string(Severity level) {
  switch(level) {
    case Severity::Info:return "info";
    case Severity::Warning:return "warning";
    case Severity::Error:return "error";
  }
  return "unknown";
}
} // namespace cansec
