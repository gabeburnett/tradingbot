#ifndef UTILS_H
#define UTILS_H

#include <vector>
#include <string>
#include <sstream>
#include <regex>

bool isDouble(std::string str);

bool split(std::vector<std::string> *values, std::string str, char delimiter, bool isNumber);

/**
 * Appends a line to a file.
 * 
 * @param filePath The file path to append to.
 * @param line The line being appended.
 */ 
void appendLineToFile(std::string filePath, std::string line);

#endif