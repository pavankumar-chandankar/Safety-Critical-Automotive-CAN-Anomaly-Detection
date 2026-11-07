#include "cansec/engine.hpp"
#include "cansec/candump.hpp"
#include "cansec/report.hpp"
#include <fstream>
#include <iostream>
#include <string>

namespace {
void usage() {
  std::cerr<<"CAN Sentinel (offline CAN/CAN FD engineering analysis)\n"
           <<"Usage: cansec analyze --trace FILE --rules FILE [--signals FILE] [--output FILE] [--format json|text] [--fail-on-finding] [--max-frames N]\n"
           <<"       cansec convert --input candump.log --output normalized.csv\n";
}
}
int main(int argc,char** argv) {
  if(argc>=2 && std::string(argv[1])=="convert") {
    std::string input_path, output_path;
    for(int i=2;i<argc;++i) {
      if(i+1>=argc) {usage();return 2;}
      std::string key=argv[i];std::string value=argv[++i];
      if(key=="--input")input_path=value;
      else if(key=="--output")output_path=value;
      else {usage();return 2;}
    }
    if(input_path.empty() || output_path.empty() || input_path==output_path) {usage();return 2;}
    std::ifstream input(input_path);
    if(!input) {std::cerr<<"Cannot read candump input\n";return 2;}
    const auto result=cansec::parse_candump(input);
    if(!result.errors.empty()) {
      for(const auto& e:result.errors)std::cerr<<"line "<<e.line<<" "<<e.code<<" "<<e.description<<'\n';
      return 2;
    }
    std::ofstream output(output_path);
    if(!output) {std::cerr<<"Cannot write normalized trace\n";return 2;}
    cansec::write_normalized_csv(output,result.frames);
    return 0;
  }
  if(argc<2 || std::string(argv[1])!="analyze") {usage();return 2;}
  std::string trace_path,rules_path,signals_path,output_path,format="json";
  bool fail_on_finding=false;
  std::size_t max_frames=1'000'000U;
  for(int i=2;i<argc;++i) {
    const std::string key=argv[i];
    if(key=="--fail-on-finding") {fail_on_finding=true;continue;}
    if(i+1>=argc) {usage();return 2;}
    const std::string value=argv[++i];
    if(key=="--trace") trace_path=value;
    else if(key=="--rules")rules_path=value;
    else if(key=="--signals")signals_path=value;
    else if(key=="--output")output_path=value;
    else if(key=="--format")format=value;
    else if(key=="--max-frames") {
      const auto parsed=cansec::parse_unsigned(value);
      if(!parsed || *parsed==0U || *parsed>10'000'000U) {usage();return 2;}
      max_frames=static_cast<std::size_t>(*parsed);
    } else {usage();return 2;}
  }
  if(trace_path.empty() || rules_path.empty() || (format!="json" && format!="text")) {usage();return 2;}
  std::ifstream traces(trace_path),rule_stream(rules_path);
  if(!traces || !rule_stream) {std::cerr<<"Cannot read trace or rule files\n";return 2;}
  const auto rules=cansec::load_rules(rule_stream);
  if(!rules.errors.empty() || rules.rules.empty()) {for(const auto& error:rules.errors)std::cerr<<error<<'\n';return 2;}
  cansec::SignalCatalog catalog;
  if(!signals_path.empty()) {
    std::ifstream signals_stream(signals_path);
    if(!signals_stream) {std::cerr<<"Cannot read signal catalog\n";return 2;}
    auto parsed=cansec::load_signals(signals_stream);
    if(!parsed.errors.empty()) {for(const auto& error:parsed.errors)std::cerr<<error<<'\n';return 2;}
    catalog=std::move(parsed.catalog);
  }
  const cansec::TraceParser parser;
  const auto parsed_trace=parser.read(traces,max_frames);
  cansec::Detector detector(rules.rules,std::move(catalog));
  const auto analysis=detector.inspect(parsed_trace);
  std::ofstream output_file;
  std::ostream* out=&std::cout;
  if(!output_path.empty()) {
    output_file.open(output_path);
    if(!output_file) {std::cerr<<"Cannot open output path\n";return 2;}
    out=&output_file;
  }
  if(format=="json") cansec::write_json(*out,analysis);
  else cansec::write_text(*out,analysis);
  if(!analysis.errors.empty()) return 2;
  if(fail_on_finding && analysis.summary.total_findings>0U) return 3;
  return 0;
}
