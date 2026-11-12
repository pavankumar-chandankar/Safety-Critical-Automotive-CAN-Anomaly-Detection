#include "cansec/gem.hpp"
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
int usage() {
  std::cerr << "Usage: cansec_gem --normal GEM_Normal_Driving_Raw_Labeled.csv "
            << "--attack GEM_Attack_Scenario_Raw_Labeled.csv "
            << "[--report metrics.json] [--predictions predictions.csv] [--target-fpr 0.01]\n";
  return 2;
}
}
int main(int argc,char** argv) {
  std::string normal_path,attack_path,report_path,predictions_path;
  double target_fpr=0.01;
  try {
    for(int i=1;i<argc;i+=2) {
      if(i+1>=argc) return usage();
      const std::string k=argv[i],v=argv[i+1];
      if(k=="--normal")normal_path=v;
      else if(k=="--attack")attack_path=v;
      else if(k=="--report")report_path=v;
      else if(k=="--predictions")predictions_path=v;
      else if(k=="--target-fpr") {
        std::size_t used=0;
        target_fpr=std::stod(v,&used);
        if(used!=v.size())return usage();
      }else return usage();
    }
    if(normal_path.empty() || attack_path.empty() || normal_path==attack_path)return usage();
    std::ifstream normal_file(normal_path),attack_file(attack_path);
    if(!normal_file || !attack_file)throw std::runtime_error("cannot open GEM-CAN inputs; use scripts/download_gem_can.py first");
    const auto normal=cansec::read_gem_csv(normal_file);
    const auto attack=cansec::read_gem_csv(attack_file);
    std::ofstream predictions;
    if(!predictions_path.empty()) {
      predictions.open(predictions_path);
      if(!predictions)throw std::runtime_error("cannot open predictions output");
    }
    auto metrics=cansec::evaluate_gem(normal,attack,target_fpr,
                                      predictions_path.empty()?nullptr:&predictions);
    std::ofstream report;
    if(!report_path.empty()) {
      report.open(report_path);
      if(!report)throw std::runtime_error("cannot open report path");
    }
    cansec::write_gem_report(report_path.empty()?std::cout:report,metrics);
  } catch(const std::exception& error) {
    std::cerr<<"CAN GEM evaluation failed: "<<error.what()<<'\n';
    return 2;
  }
  return 0;
}
