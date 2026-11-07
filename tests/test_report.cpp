#include "harness.hpp"
#include "cansec/report.hpp"
#include <sstream>
using namespace cansec;
TEST_CASE("JSON escaping handles quotes, backslashes, newline") {CHECK(escape_json("a\"\\\nb")=="a\\\"\\\\\\nb");}
TEST_CASE("JSON emits schema, counters and findings") {
 Analysis a;a.summary.total_frames=2U;a.summary.total_findings=1U;
 a.summary.findings_by_code["UNKNOWN_ID"]=1U;
 a.findings.push_back({7,100,"std:0x700","UNKNOWN_ID",Severity::Warning,"unknown"});
 std::ostringstream stream;write_json(stream,a);
 CHECK(stream.str().find("\"schema_version\": 1")!=std::string::npos);
 CHECK(stream.str().find("\"UNKNOWN_ID\": 1")!=std::string::npos);
}
TEST_CASE("text report includes finding codes") {
 Analysis a;a.findings.push_back({3,100,"std:0x100","TEST",Severity::Info,"synthetic"});
 std::ostringstream stream;write_text(stream,a);CHECK(stream.str().find("TEST")!=std::string::npos);
}
TEST_CASE("severity names defined") {
 CHECK(severity_string(Severity::Info)=="info");CHECK(severity_string(Severity::Warning)=="warning");CHECK(severity_string(Severity::Error)=="error");
}
