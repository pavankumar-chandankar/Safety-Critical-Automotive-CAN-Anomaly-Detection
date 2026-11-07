#pragma once
#include <functional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace testkit {
struct Entry {std::string name; std::function<void()> function;};
inline std::vector<Entry>& registry() {static std::vector<Entry> cases;return cases;}
struct Register {
  Register(const std::string& name,std::function<void()> f) {registry().push_back({name,std::move(f)});}
};
inline void require(bool condition,const char* expression,const char* file,int line) {
  if(!condition)throw std::runtime_error(std::string(file)+":"+std::to_string(line)+" failed: "+expression);
}
}
#define CANSEC_JOIN2(a,b) a##b
#define CANSEC_JOIN(a,b) CANSEC_JOIN2(a,b)
#define TEST_CASE(name) static void CANSEC_JOIN(test_,__LINE__)(); \
  static testkit::Register CANSEC_JOIN(registration_,__LINE__){name,CANSEC_JOIN(test_,__LINE__)}; \
  static void CANSEC_JOIN(test_,__LINE__)()
#define CHECK(expression) testkit::require(static_cast<bool>(expression),#expression,__FILE__,__LINE__)
