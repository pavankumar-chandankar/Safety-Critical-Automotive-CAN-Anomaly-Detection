#include "harness.hpp"
#include <exception>
#include <iostream>
int main() {
  std::size_t passed=0,failed=0;
  for(const auto& test:testkit::registry()) {
    try {test.function();std::cout<<"[PASS] "<<test.name<<'\n';++passed;}
    catch(const std::exception& error) {std::cerr<<"[FAIL] "<<test.name<<": "<<error.what()<<'\n';++failed;}
    catch(...) {std::cerr<<"[FAIL] "<<test.name<<": unknown error\n";++failed;}
  }
  std::cout<<"RESULT: "<<passed<<" passed / "<<failed<<" failed\n";
  return failed==0U?0:1;
}
