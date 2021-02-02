#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <sstream>
#include <regex>

bool split(std::vector<std::string> *values, std::string str, char delimiter, bool isNumber);

#endif