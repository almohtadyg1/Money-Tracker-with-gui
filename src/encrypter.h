#pragma once
#include <string>

std::string encryptAesCng(const std::string& plaintext, const std::string& keyStr);

std::string decryptAesCng(const std::string& ciphertext, const std::string& keyStr);