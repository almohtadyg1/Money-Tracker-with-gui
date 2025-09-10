#pragma once
#include <string>
#include <map>

std::string loadFile(const std::string& filename, bool& err);
bool saveToFile(const std::string& dataString, const std::string& filename);

std::string dataToString(const std::map<std::string, std::string>& dataMap, const std::map<std::string, std::string>& borrowersMap, const std::vector<std::string>& orderVector);
void stringToData(const std::string_view data, std::map<std::string, std::string>& outDataMap, std::map<std::string, std::string>& outBorrowersMap, std::vector<std::string>& outOrderVector);
