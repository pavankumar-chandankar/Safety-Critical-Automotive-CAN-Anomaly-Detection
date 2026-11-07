#pragma once
#include "cansec/parser.hpp"
#include <istream>
#include <ostream>
#include <string>
namespace cansec {
// Strict subset of Linux can-utils candump ASCII format.
// Input example: (1700000000.123456) can0 123#1122334455667788
// FD example:    (1700000000.123457) can1 18DAF110##1AABBCCDDEEFF001122334455
bool decode_candump_line(const std::string& text,std::size_t line,Frame& frame,ParseError& error);
ParseResult parse_candump(std::istream& input,std::size_t max_frames=1000000U);
void write_normalized_csv(std::ostream& out,const std::vector<Frame>& frames);
} // namespace cansec
