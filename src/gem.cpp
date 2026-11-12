#include "cansec/gem.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace cansec {
namespace {
std::string lower(std::string s) {
  std::transform(s.begin(),s.end(),s.begin(),[](unsigned char c){return static_cast<char>(std::tolower(c));});
  return s;
}
std::string key(std::string s) {
  s=lower(trim(s));
  s.erase(std::remove_if(s.begin(),s.end(),[](char c){return c=='_'||c=='-'||c==' ';}),s.end());
  if(s.size()>=3 && s.compare(0,3,"\xEF\xBB\xBF")==0)s.erase(0,3);
  return s;
}
std::vector<std::string> columns(const std::string& s) {
  std::vector<std::string> out;std::string cell;bool quote=false;
  for(std::size_t i=0;i<s.size();++i) {
    char c=s[i];
    if(c=='"') {
      if(quote && i+1<s.size() && s[i+1]=='"'){cell+='"';++i;}
      else quote=!quote;
    }else if(c==','&&!quote){out.push_back(trim(cell));cell.clear();}
    else cell+=c;
  }
  if(quote)throw std::runtime_error("unclosed CSV quote");
  out.push_back(trim(cell));
  return out;
}
std::uint64_t integer(std::string s,int base,std::uint64_t upper) {
  s=trim(s);
  if(s.empty()||s[0]=='-')throw std::runtime_error("missing/negative numeric field");
  if(base==16 && s.size()>2 && s[0]=='0' && (s[1]=='x'||s[1]=='X'))s.erase(0,2);
  std::size_t pos=0;
  const auto n=std::stoull(s,&pos,base);
  if(pos!=s.size() || n>upper)throw std::runtime_error("invalid numeric field");
  return static_cast<std::uint64_t>(n);
}
std::uint64_t timestamp(std::string s) {
  s.erase(std::remove(s.begin(),s.end(),','),s.end());
  std::size_t pos=0;
  const double seconds=std::stod(s,&pos);
  if(pos!=s.size() || !std::isfinite(seconds) || seconds<0 || seconds>1e10)
    throw std::runtime_error("invalid timestamp");
  return static_cast<std::uint64_t>(std::llround(seconds*1e6));
}
std::string json_escape(const std::string& s) {
  std::string out;
  for(char c:s) {
    if(c=='"')out+="\\\"";
    else if(c=='\\')out+="\\\\";
    else if(c=='\n')out+="\\n";
    else if(static_cast<unsigned char>(c)<32)out+='?';
    else out+=c;
  }
  return out;
}
void metrics(std::ostream& o,const Counts& c) {
  const double tp=static_cast<double>(c.tp),fp=static_cast<double>(c.fp);
  const double tn=static_cast<double>(c.tn),fn=static_cast<double>(c.fn);
  auto div=[](double a,double b){return b>0.0 ? a/b : 0.0;};
  o<<"{\"tp\":"<<c.tp<<",\"fp\":"<<c.fp<<",\"tn\":"<<c.tn<<",\"fn\":"<<c.fn
   <<",\"precision\":"<<div(tp,tp+fp)<<",\"recall\":"<<div(tp,tp+fn)
   <<",\"f1\":"<<div(2*tp,2*tp+fp+fn)<<",\"fpr\":"<<div(fp,fp+tn)<<"}";
}
} // namespace

std::vector<GemRow> read_gem_csv(std::istream& input,std::size_t maximum_rows) {
  std::string first;
  if(!std::getline(input,first))throw std::runtime_error("empty GEM-CAN CSV");
  const auto header=columns(first);
  std::unordered_map<std::string,std::size_t> loc;
  for(std::size_t i=0;i<header.size();++i)loc.emplace(key(header[i]),i);
  auto required=[&](const std::string& name){
    auto it=loc.find(name);
    if(it==loc.end())throw std::runtime_error("missing GEM-CAN field: "+name);
    return it->second;
  };
  const auto ts=required("timestamp"),id=required("arbitrationid");
  const auto dlc=required("dlc"),label=required("label");
  std::array<std::size_t,8> payload{};
  for(std::size_t i=0;i<8;++i)payload[i]=required("data"+std::to_string(i));
  const auto attack=loc.find("attacktype");
  std::vector<GemRow> records;
  std::string line;std::size_t line_no=1;
  while(std::getline(input,line)) {
    ++line_no;
    if(trim(line).empty())continue;
    if(line.size()>4096 || records.size()>=maximum_rows)
      throw std::runtime_error("GEM-CAN input size limit exceeded");
    try {
      auto cells=columns(line);
      if(cells.size()!=header.size())throw std::runtime_error("wrong CSV column count");
      auto field=[&](std::size_t idx)->const std::string&{return cells.at(idx);};
      GemRow row;
      row.frame.timestamp_us=timestamp(field(ts));
      row.frame.id=static_cast<std::uint32_t>(integer(field(id),16,0x1FFFFFFFU));
      row.frame.extended=row.frame.id>0x7FFU;
      row.frame.dlc=static_cast<std::uint8_t>(integer(field(dlc),10,8));
      row.frame.length=row.frame.dlc;row.frame.source_line=line_no;
      for(std::size_t i=0;i<8;++i) {
        if(field(payload[i]).empty()) {
          if(i<row.frame.length)throw std::runtime_error("required payload byte missing");
          row.frame.payload[i]=0;
        }else row.frame.payload[i]=static_cast<std::uint8_t>(integer(field(payload[i]),16,255));
      }
      row.label=static_cast<int>(integer(field(label),10,1));
      if(row.label==0)row.attack_type="Normal";
      else if(attack!=loc.end() && !field(attack->second).empty())
        row.attack_type=field(attack->second);
      else row.attack_type="Attack";
      records.push_back(std::move(row));
    }catch(const std::exception& error){
      throw std::runtime_error("line "+std::to_string(line_no)+": "+error.what());
    }
  }
  if(records.empty())throw std::runtime_error("GEM-CAN CSV contains no frames");
  return records;
}

void NormalProfile::fit(const std::vector<GemRow>& ordered,std::size_t begin,std::size_t end) {
  if(begin>=end || end>ordered.size())throw std::invalid_argument("bad normal-training range");
  profiles_.clear();last_seen_.clear();
  std::unordered_map<std::uint32_t,std::vector<double>> gaps;
  for(std::size_t i=begin;i<end;++i) {
    const auto& row=ordered[i];
    if(row.label!=0)throw std::invalid_argument("attack labels prohibited in normal training");
    const Frame& f=row.frame;
    auto& p=profiles_[f.id];
    ++p.examples;
    p.dlcs.at(f.dlc)=true;
    for(std::size_t j=0;j<f.length;++j)p.bytes[j][f.payload[j]]=true;
    auto last=last_seen_.find(f.id);
    if(last!=last_seen_.end() && f.timestamp_us>last->second)
      gaps[f.id].push_back(static_cast<double>(f.timestamp_us-last->second)/1e6);
    last_seen_[f.id]=f.timestamp_us;
  }
  for(auto& item:gaps) {
    auto& v=item.second;
    if(v.size()<25)continue;
    std::sort(v.begin(),v.end());
    profiles_.at(item.first).minimum_interval_seconds=v[(v.size()-1)/100];
  }
  last_seen_.clear();
}
Score NormalProfile::score(const Frame& f) {
  Score s;
  auto it=profiles_.find(f.id);
  if(it==profiles_.end()){s.value=4.0;s.unfamiliar_id=true;}
  else {
    const auto& p=it->second;
    if(!p.dlcs.at(f.dlc)){s.value+=3.0;s.wrong_dlc=true;}
    if(p.examples>=25) {
      for(std::size_t i=0;i<f.length && i<8;++i)
        if(!p.bytes[i][f.payload[i]]){++s.unfamiliar_bytes;s.value+=0.75;}
    }
    auto last=last_seen_.find(f.id);
    if(p.minimum_interval_seconds>0 && last!=last_seen_.end() && f.timestamp_us>last->second) {
      double dt=static_cast<double>(f.timestamp_us-last->second)/1e6;
      if(dt<0.30*p.minimum_interval_seconds){s.value+=1.25;s.unexpected_rate=true;}
    }
  }
  last_seen_[f.id]=f.timestamp_us;
  return s;
}
void NormalProfile::reset_session(){last_seen_.clear();}
void Counts::observe(bool predicted,bool actual) {
  if(predicted&&actual)++tp;
  else if(predicted)++fp;
  else if(actual)++fn;
  else ++tn;
}
GemEvaluation evaluate_gem(const std::vector<GemRow>& normal,const std::vector<GemRow>& attack,
                            double target_fpr,std::ostream* predictions) {
  if(!(target_fpr>0 && target_fpr<1))throw std::invalid_argument("target FPR outside range");
  if(normal.size()<100 || attack.empty())throw std::invalid_argument("insufficient recordings");
  auto sorted=[](const std::vector<GemRow>& input){
    auto output=input;
    std::stable_sort(output.begin(),output.end(),[](const GemRow& a,const GemRow& b){
      return a.frame.timestamp_us<b.frame.timestamp_us;
    });
    return output;
  };
  auto baseline=sorted(normal),evaluation=sorted(attack);
  for(const auto& row:baseline)
    if(row.label!=0)throw std::invalid_argument("normal baseline includes attacks");
  const std::size_t total=baseline.size(),ntrain=(total*60)/100,ncal=(total*80)/100;
  NormalProfile p;p.fit(baseline,0,ntrain);
  std::vector<double> scores;scores.reserve(ncal-ntrain);
  p.reset_session();
  for(std::size_t i=ntrain;i<ncal;++i)scores.push_back(p.score(baseline[i].frame).value);
  std::sort(scores.begin(),scores.end());
  const std::size_t idx=std::min(scores.size()-1,
    static_cast<std::size_t>(std::floor((1.0-target_fpr)*static_cast<double>(scores.size()))));
  const double threshold=scores[idx];
  GemEvaluation e;
  e.threshold=threshold;e.train_frames=ntrain;e.calibration_frames=ncal-ntrain;
  e.holdout_frames=total-ncal;e.attack_frames=evaluation.size();e.known_ids=p.known_identifiers();
  p.reset_session();
  for(std::size_t i=ncal;i<total;++i)
    e.normal_holdout.observe(p.score(baseline[i].frame).value>threshold,false);
  p.reset_session();
  if(predictions)
    *predictions<<"source_line,timestamp_us,arbitration_id,attack_type,label,prediction,score,threshold,reason\n";
  for(const auto& row:evaluation) {
    auto score=p.score(row.frame);
    bool alert=score.value>threshold;
    e.attack_session.observe(alert,row.label==1);
    e.by_attack_type[row.attack_type].observe(alert,row.label==1);
    if(predictions) {
      const char* why=score.unfamiliar_id?"unseen_id":
        score.wrong_dlc?"unseen_dlc":score.unfamiliar_bytes?"unseen_payload_byte":
        score.unexpected_rate?"unusual_rate":"none";
      *predictions<<row.frame.source_line<<','<<row.frame.timestamp_us<<','
                  <<std::hex<<std::uppercase<<row.frame.id<<std::dec<<",\""
                  <<json_escape(row.attack_type)<<"\","<<row.label<<','
                  <<static_cast<int>(alert)<<','<<score.value<<','<<threshold<<','<<why<<'\n';
    }
  }
  return e;
}
void write_gem_report(std::ostream& out,const GemEvaluation& r) {
  out<<std::fixed<<std::setprecision(6);
  out<<"{\n  \"dataset\": \"GEM-CAN v3 (Zenodo 19161139)\",\n"
     <<"  \"evaluation_design\": \"normal-session chronological train/calibrate/holdout; independent attack session\",\n"
     <<"  \"model\": \"C++17 unsupervised CAN ID/DLC/payload/timing baseline\",\n"
     <<"  \"train_frames\": "<<r.train_frames<<",\n  \"calibration_frames\": "<<r.calibration_frames
     <<",\n  \"normal_holdout_frames\": "<<r.holdout_frames
     <<",\n  \"attack_session_frames\": "<<r.attack_frames
     <<",\n  \"known_ids\": "<<r.known_ids<<",\n  \"threshold\": "<<r.threshold
     <<",\n  \"normal_holdout\": ";
  metrics(out,r.normal_holdout);
  out<<",\n  \"attack_session\": ";metrics(out,r.attack_session);
  out<<",\n  \"by_attack_type\": {";
  std::vector<std::string> names;
  for(const auto& x:r.by_attack_type)names.push_back(x.first);
  std::sort(names.begin(),names.end());
  for(std::size_t i=0;i<names.size();++i) {
    if(i)out<<',';
    out<<"\n    \""<<json_escape(names[i])<<"\": ";
    metrics(out,r.by_attack_type.at(names[i]));
  }
  out<<"\n  }\n}\n";
}
} // namespace cansec
