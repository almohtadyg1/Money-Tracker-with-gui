#include <string>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>

std::string loadFile(const std::string& filename, bool& err) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        err = true;
        return "";
    }

    // Move to the end to get size, then back to start
    file.seekg(0, std::ios::end);
    std::streampos pos = file.tellg();
    if (pos < 0) {
        err = true;
        return "";
    }

    size_t size = static_cast<size_t>(pos);
    file.seekg(0);

    std::string buffer(size, '\0');
    file.read(&buffer[0], size);
    file.close();

    return buffer;
}


bool saveToFile(const std::string& dataString, const std::string& filename) {
    std::ofstream file(filename, std::ios::binary);
    if (file.is_open()) {
        file.write(dataString.data(), dataString.size());
        file.close();
        return false;
    }
    else {
        return true;
    }
}



std::string dataToString(const std::map<std::string, std::string>& dataMap, const std::map<std::string, std::string>& borrowersMap, const std::vector<std::string>& orderVector) {

    std::string result;
    for (const auto& pair : dataMap) {
        result += pair.first + ":" + pair.second + ",";
    }
    if (!result.empty()) result.pop_back();

    result.push_back('|');
    for (const auto& pair : borrowersMap) {
        result += pair.first + ":" + pair.second + ",";
    }
    if (result.back() != '|') result.pop_back();

    result.push_back('|');
    size_t totalSize = result.size();
    for (const auto& item : orderVector) {
        totalSize += item.size() + 1;
    }

    result.reserve(totalSize);

    for (size_t i = 0; i < orderVector.size(); ++i) {
        result += orderVector[i];
        result.push_back(',');
    }
    if (!result.empty()) result.pop_back();

    return result;
}

void stringToData(const std::string_view data, std::map<std::string, std::string>& outDataMap, std::map<std::string, std::string>& outBorrowersMap, std::vector<std::string>& outOrderVector) {

    size_t first = data.find('|');
    size_t second = data.find('|', first + 1);

    std::string_view dataMapString = data.substr(0, first);
    std::string_view borrowersMapString = data.substr(first + 1, second - first - 1);
    std::string_view orderVectorString = data.substr(second + 1);

    size_t start = 0;
    {
        while (start < dataMapString.size()) {
            size_t end = dataMapString.find(',', start);
            if (end == std::string_view::npos) end = dataMapString.size();

            std::string_view pair = dataMapString.substr(start, end - start);
            size_t colon = pair.find(':');
            if (colon != std::string_view::npos) {
                outDataMap[std::string(pair.substr(0, colon))] = std::string(pair.substr(colon + 1));
            }

            start = end + 1;
        }
    }
    start = 0;
    {
        while (start < borrowersMapString.size()) {
            size_t end = borrowersMapString.find(',', start);
            if (end == std::string_view::npos) end = borrowersMapString.size();

            std::string_view pair = borrowersMapString.substr(start, end - start);
            size_t colon = pair.find(':');
            if (colon != std::string_view::npos) {
                outBorrowersMap[std::string(pair.substr(0, colon))] = std::string(pair.substr(colon + 1));
            }

            start = end + 1;
        }
    }
    start = 0;
    {
        while (start < orderVectorString.size()) {
            size_t end = orderVectorString.find(',', start);
            if (end == std::string_view::npos) end = orderVectorString.size();
            outOrderVector.push_back(std::string(orderVectorString.substr(start, end - start)));

            start = end + 1;
        }
    }
}