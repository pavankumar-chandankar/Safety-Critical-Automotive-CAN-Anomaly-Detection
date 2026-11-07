#pragma once
#include "cansec/engine.hpp"
#include <iosfwd>
#include <string>
namespace cansec {
std::string escape_json(const std::string& text);
void write_json(std::ostream& out,const Analysis& analysis);
void write_text(std::ostream& out,const Analysis& analysis);
} // namespace cansec
