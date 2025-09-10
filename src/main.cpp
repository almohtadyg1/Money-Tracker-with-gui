// Money Tracker GUI - main.cpp (Enhanced with BigNumber support and improved UI)
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "misc/cpp/imgui_stdlib.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <map>
#include <vector>
#include <filesystem>
#include <cctype>
#include <string>
#include <algorithm>
#include <sstream>

// Your existing headers
#include "encrypter.h"
#include "savingFunctions.h"

const std::string DATA_FILE = "saves.data";

void SetGLFWWindowIcon(GLFWwindow* window) {
    GLFWimage icon;
    icon.pixels = stbi_load("resources/app_icon.png", &icon.width, &icon.height, 0, 4); // 4 = RGBA
    if (icon.pixels) {
        glfwSetWindowIcon(window, 1, &icon);
        stbi_image_free(icon.pixels);
    } else {
        fprintf(stderr, "Failed to load window icon\n");
    }
}

// Big Number class for handling arbitrarily large numbers
class BigNumber {
private:
    std::string number;
    bool negative;
    
    // Remove leading zeros
    void normalize() {
        size_t pos = number.find_first_not_of('0');
        if (pos == std::string::npos) {
            number = "0";
            negative = false;
        } else {
            number = number.substr(pos);
        }
        if (number.empty()) {
            number = "0";
            negative = false;
        }
    }
    
    // Compare absolute values (returns: -1 if this < other, 0 if equal, 1 if this > other)
    int compareAbs(const BigNumber& other) const {
        if (number.length() < other.number.length()) return -1;
        if (number.length() > other.number.length()) return 1;
        return number.compare(other.number);
    }
    
    std::string addPositive(const std::string& a, const std::string& b) const {
        // Find decimal points
        size_t decimalA = a.find('.');
        size_t decimalB = b.find('.');
        
        // If no decimal points, use original logic
        if (decimalA == std::string::npos && decimalB == std::string::npos) {
            // Your original integer addition code here
            std::string result;
            int carry = 0;
            int i = a.length() - 1;
            int j = b.length() - 1;
            
            while (i >= 0 || j >= 0 || carry > 0) {
                int sum = carry;
                if (i >= 0) sum += (a[i--] - '0');
                if (j >= 0) sum += (b[j--] - '0');
                
                result = char(sum % 10 + '0') + result;
                carry = sum / 10;
            }
            return result;
        }
        
        // Handle decimal numbers
        std::string intA = (decimalA == std::string::npos) ? a : a.substr(0, decimalA);
        std::string fracA = (decimalA == std::string::npos) ? "" : a.substr(decimalA + 1);
        std::string intB = (decimalB == std::string::npos) ? b : b.substr(0, decimalB);
        std::string fracB = (decimalB == std::string::npos) ? "" : b.substr(decimalB + 1);
        
        // Pad fractional parts to same length
        while (fracA.length() < fracB.length()) fracA += "0";
        while (fracB.length() < fracA.length()) fracB += "0";
        
        // Add fractional parts first
        std::string fracResult;
        int carry = 0;
        for (int i = fracA.length() - 1; i >= 0; i--) {
            int sum = carry + (fracA[i] - '0') + (fracB[i] - '0');
            fracResult = char(sum % 10 + '0') + fracResult;
            carry = sum / 10;
        }
        
        // Add integer parts
        std::string intResult;
        int i = intA.length() - 1;
        int j = intB.length() - 1;
        
        while (i >= 0 || j >= 0 || carry > 0) {
            int sum = carry;
            if (i >= 0) sum += (intA[i--] - '0');
            if (j >= 0) sum += (intB[j--] - '0');
            
            intResult = char(sum % 10 + '0') + intResult;
            carry = sum / 10;
        }
        
        // Combine results
        if (fracResult.empty()) return intResult;
        return intResult + "." + fracResult;
    }
    
    // Subtract two positive numbers (assumes a >= b)
    std::string subtractPositive(const std::string& a, const std::string& b) const {
        // Find decimal points
        size_t decimalA = a.find('.');
        size_t decimalB = b.find('.');
        
        // If no decimal points, use original logic
        if (decimalA == std::string::npos && decimalB == std::string::npos) {
            // Your original integer subtraction code here
            std::string result;
            int borrow = 0;
            int i = a.length() - 1;
            int j = b.length() - 1;
            
            while (i >= 0) {
                int sub = (a[i] - '0') - borrow;
                if (j >= 0) sub -= (b[j--] - '0');
                
                if (sub < 0) {
                    sub += 10;
                    borrow = 1;
                } else {
                    borrow = 0;
                }
                
                result = char(sub + '0') + result;
                i--;
            }
            return result;
        }
        
        // Handle decimal numbers
        std::string intA = (decimalA == std::string::npos) ? a : a.substr(0, decimalA);
        std::string fracA = (decimalA == std::string::npos) ? "" : a.substr(decimalA + 1);
        std::string intB = (decimalB == std::string::npos) ? b : b.substr(0, decimalB);
        std::string fracB = (decimalB == std::string::npos) ? "" : b.substr(decimalB + 1);
        
        // Pad fractional parts to same length
        while (fracA.length() < fracB.length()) fracA += "0";
        while (fracB.length() < fracA.length()) fracB += "0";
        
        // Subtract fractional parts first
        std::string fracResult;
        int borrow = 0;
        for (int i = fracA.length() - 1; i >= 0; i--) {
            int sub = (fracA[i] - '0') - borrow - (fracB[i] - '0');
            
            if (sub < 0) {
                sub += 10;
                borrow = 1;
            } else {
                borrow = 0;
            }
            
            fracResult = char(sub + '0') + fracResult;
        }
        
        // Subtract integer parts
        std::string intResult;
        int i = intA.length() - 1;
        int j = intB.length() - 1;
        
        while (i >= 0) {
            int sub = (intA[i] - '0') - borrow;
            if (j >= 0) sub -= (intB[j--] - '0');
            
            if (sub < 0) {
                sub += 10;
                borrow = 1;
            } else {
                borrow = 0;
            }
            
            intResult = char(sub + '0') + intResult;
            i--;
        }
        
        // Remove trailing zeros from fractional part
        while (!fracResult.empty() && fracResult.back() == '0') {
            fracResult.pop_back();
        }
        
        // Combine results
        if (fracResult.empty()) return intResult;
        return intResult + "." + fracResult;
    }
    
public:
    BigNumber() : number("0"), negative(false) {}
    
    BigNumber(const std::string& str) {
        if (str.empty() || str == "0") {
            number = "0";
            negative = false;
            return;
        }
        
        negative = (str[0] == '-');
        number = negative ? str.substr(1) : str;
        
        // Validate that it's all digits (allow decimal point)
        bool hasDecimal = false;
        std::string cleanNumber;
        for (char c : number) {
            if (c == '.') {
                if (hasDecimal) {
                    number = "0";
                    negative = false;
                    return;
                }
                hasDecimal = true;
                cleanNumber += c;  // Add this line to preserve the decimal point
            } else if (std::isdigit(c)) {
                cleanNumber += c;
            } else {
                number = "0";
                negative = false;
                return;
            }
        }

        number = cleanNumber.empty() ? "0" : cleanNumber;
        normalize();
    }
    
    BigNumber(long long val) {
        if (val < 0) {
            negative = true;
            val = -val;
        } else {
            negative = false;
        }
        number = std::to_string(val);
    }
    
    std::string toString() const {
        if (number == "0") return "0";
        return (negative ? "-" : "") + number;
    }
    
    bool isZero() const {
        return number == "0";
    }
    
    bool isNegative() const {
        return negative && !isZero();
    }
    
    BigNumber operator+(const BigNumber& other) const {
        BigNumber result;
        
        if (negative == other.negative) {
            // Same signs: add absolute values
            result.number = addPositive(number, other.number);
            result.negative = negative;
        } else {
            // Different signs: subtract absolute values
            int cmp = compareAbs(other);
            if (cmp == 0) {
                result.number = "0";
                result.negative = false;
            } else if (cmp > 0) {
                result.number = subtractPositive(number, other.number);
                result.negative = negative;
            } else {
                result.number = subtractPositive(other.number, number);
                result.negative = other.negative;
            }
        }
        
        result.normalize();
        return result;
    }
    
    BigNumber operator-(const BigNumber& other) const {
        BigNumber temp = other;
        temp.negative = !temp.negative;
        return *this + temp;
    }
    
    BigNumber& operator+=(const BigNumber& other) {
        *this = *this + other;
        return *this;
    }
    
    BigNumber& operator-=(const BigNumber& other) {
        *this = *this - other;
        return *this;
    }
};

// GUI State Management
enum class AppState {
    LOGIN,
    NEW_USER,
    MAIN_MENU,
    VIEW_DATA,
    TRANSACTION,
    NOTE,
    BORROWERS,
    RESET_CONFIRM
};

struct AppData {
    AppState currentState = AppState::LOGIN;
    std::map<std::string, std::string> dataMap;
    std::map<std::string, std::string> borrowersMap;
    std::vector<std::string> orderVector;
    std::string userKey;
    bool dataLoaded = false;
    bool fileExists = false;
    
    // GUI input strings (safer than char buffers)
    std::string passwordInput;
    std::string initialMoneyInput;
    std::string transactionValueInput;
    std::string noteInput;
    std::string borrowerNameInput;
    std::string borrowerValueInput;
    
    // GUI state
    bool showDemo = false;
    bool transactionIsPositive = true;
    bool borrowerIsYou = true;
    bool showResetDialog = false;
    bool showErrorAlert = false;
    std::string statusMessage;
    std::string alertMessage;
    ImVec4 statusColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    
    void setStatus(const std::string& msg, ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f)) {
        statusMessage = msg;
        statusColor = color;
    }
    
    void showAlert(const std::string& msg) {
        alertMessage = msg;
        showErrorAlert = true;
    }
};

// Enhanced utility functions with input validation
bool isValidNumber(const std::string& str, size_t maxLength = 50) {
    if (str.empty() || str.length() > maxLength) return false;

    size_t start = 0;
    bool hasDecimalPoint = false;

    // Allow negative numbers
    if (str[0] == '-') {
        if (str.length() == 1) return false; // Just a minus sign
        start = 1;
    }

    for (size_t i = start; i < str.length(); i++) {
        if (str[i] == '.') {
            // Only one decimal point allowed
            if (hasDecimalPoint) return false;
            hasDecimalPoint = true;
            // Decimal point cannot be first or last character
            if (i == start || i == str.length() - 1) return false;
        } else if (!std::isdigit(str[i])) {
            return false;
        }
    }

    return true;
}

bool isValidName(const std::string& str, size_t maxLength = 50) {
    if (str.empty() || str.length() > maxLength) return false;
    
    for (char c : str) {
        if (!std::isalnum(c) && c != ' ' && c != '-' && c != '_' && c != '.') {
            return false;
        }
    }
    return true;
}

bool isValidPassword(const std::string& str) {
    return !str.empty() && str.length() <= 32;
}

bool isValidNote(const std::string& str) {
    return str.length() <= 1000; // Reasonable limit for notes
}

std::string lowercase(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return str;
}

// GUI callback
static void glfw_error_callback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << std::endl;
}

// Enhanced input functions with validation
bool InputTextString(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, const char* filter = nullptr) {
    return ImGui::InputText(label, str, flags);
}

bool InputTextMultilineString(const char* label, std::string* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0) {
    return ImGui::InputTextMultiline(label, str, size, flags);
}

// Helper function to center a window
void CenterWindow(ImVec2 size) {
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImVec2 center = ImVec2(viewport->Pos.x + viewport->Size.x * 0.5f, viewport->Pos.y + viewport->Size.y * 0.5f);
    ImGui::SetNextWindowPos(ImVec2(center.x - size.x * 0.5f, center.y - size.y * 0.5f), ImGuiCond_Always);
    ImGui::SetNextWindowSize(size, ImGuiCond_Always);
}

// Helper function to center content horizontally
void CenterContent(float width) {
    float windowWidth = ImGui::GetWindowWidth();
    ImGui::SetCursorPosX((windowWidth - width) * 0.5f);
}

// Font loading function
void LoadCustomFont(ImGuiIO& io) {
    // Clear existing fonts first
    io.Fonts->Clear();
    
    // Try to load Segoe UI with larger size
    ImFont* font = io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf", 18.0f);
    
    // Fallback: if custom font fails, use default with larger size
    if (font == nullptr) {
        ImFontConfig config;
        config.SizePixels = 18.0f;
        io.Fonts->AddFontDefault(&config);
    }
    
    // Don't call Build() - let the backend handle it
}

void renderLoginScreen(AppData& app) {
    CenterWindow(ImVec2(500, 400));
    
    ImGui::Begin("Money Tracker - Login", nullptr, 
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 12));
    
    // Title
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    CenterContent(ImGui::CalcTextSize("MONEY TRACKER").x);
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "MONEY TRACKER");
    ImGui::PopFont();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    if (!app.fileExists) {
        CenterContent(ImGui::CalcTextSize("Welcome! Create your first account").x);
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Welcome! Create your first account");
        ImGui::Spacing();
        
        // Create account form
        ImGui::Text("Password (max 32 characters):");
        ImGui::SetNextItemWidth(-1);
        InputTextString("##password", &app.passwordInput, ImGuiInputTextFlags_Password);
        
        ImGui::Spacing();
        ImGui::Text("Initial money amount (max 50 digits, optional):");
        ImGui::SetNextItemWidth(-1);
        InputTextString("##initialmoney", &app.initialMoneyInput, ImGuiInputTextFlags_CharsDecimal);
        
        ImGui::Spacing();
        CenterContent(160);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
        if (ImGui::Button("Create Account", ImVec2(160, 40))) {
            if (!isValidPassword(app.passwordInput)) {
                app.showAlert("Invalid password (max 32 chars)!");
            } else if (!app.initialMoneyInput.empty() && !isValidNumber(app.initialMoneyInput)) {
                app.showAlert("Invalid money amount!");
            } else {
                // Setup new account
                app.userKey = app.passwordInput;
                if (app.userKey.length() <= 32) {
                    app.userKey.append(32 - app.userKey.length(), '*');
                }
                
                app.dataMap = {
                    {"Total Money", (app.initialMoneyInput.empty() ? "0" : app.initialMoneyInput)},
                    {"Last Transaction", "-"},
                    {"Short Note", "-"}
                };

                app.borrowersMap.clear();
                app.orderVector = {"Total Money", "Last Transaction", "Short Note"};
                
                app.currentState = AppState::MAIN_MENU;
                app.dataLoaded = true;
                app.setStatus("Account created successfully!", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
            }
        }
        ImGui::PopStyleColor(3);
    } else {
        CenterContent(ImGui::CalcTextSize("Welcome back!").x);
        ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "Welcome back!");
        ImGui::Spacing();
        
        ImGui::Text("Enter your password:");
        ImGui::SetNextItemWidth(-1);
        InputTextString("##password", &app.passwordInput, ImGuiInputTextFlags_Password);
        
        ImGui::Spacing();
        CenterContent(200);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.9f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.5f, 0.8f, 1.0f));
        if (ImGui::Button("Login", ImVec2(100, 40))) {
            if (!isValidPassword(app.passwordInput)) {
                app.showAlert("Invalid password!");
            } else {
                app.userKey = app.passwordInput;
                if (app.userKey.length() <= 32) {
                    app.userKey.append(32 - app.userKey.length(), '*');
                }
                
                // Try to load and decrypt data
                bool readErr = false;
                std::string saves = loadFile(DATA_FILE, readErr);
                if (!readErr) {
                    std::string decrypted = decryptAesCng(saves, app.userKey);
                    std::string suffix = "valid";
                    
                    if (decrypted.size() >= suffix.size() &&
                        decrypted.compare(decrypted.size() - suffix.size(), suffix.size(), suffix) == 0) {
                        
                        decrypted.erase(decrypted.size() - suffix.size());
                        while (decrypted.back() == '*') {
                            decrypted.pop_back();
                        }
                        
                        stringToData(decrypted, app.dataMap, app.borrowersMap, app.orderVector);
                        app.currentState = AppState::MAIN_MENU;
                        app.dataLoaded = true;
                        app.setStatus("Login successful!", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                    } else {
                        app.showAlert("Incorrect password!");
                    }
                } else {
                    app.showAlert("Could not read data file!");
                }
            }
        }
        ImGui::PopStyleColor(3);
        
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
        if (ImGui::Button("Reset Data", ImVec2(100, 40))) {
            app.showResetDialog = true;
        }
        ImGui::PopStyleColor(3);
    }
    
    // Status message
    if (!app.statusMessage.empty()) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        CenterContent(ImGui::CalcTextSize(app.statusMessage.c_str()).x);
        ImGui::TextColored(app.statusColor, "%s", app.statusMessage.c_str());
    }
    
    ImGui::PopStyleVar();
    ImGui::End();
}

void renderMainMenu(AppData& app) {
    CenterWindow(ImVec2(600, 700));
    
    ImGui::Begin("Money Tracker - Main Menu", nullptr, 
        ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 15));
    
    // Header
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    CenterContent(ImGui::CalcTextSize("MONEY TRACKER DASHBOARD").x);
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "MONEY TRACKER DASHBOARD");
    ImGui::PopFont();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Quick info panel
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.15f, 0.25f, 0.8f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    ImGui::BeginChild("QuickInfo", ImVec2(0, 120), true);
    
    BigNumber totalMoney(app.dataMap["Total Money"]);
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
    if (totalMoney.isNegative()) {
        CenterContent(ImGui::CalcTextSize(("Total Money: $" + totalMoney.toString()).c_str()).x);
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Total Money: $%s", totalMoney.toString().c_str());
    } else {
        CenterContent(ImGui::CalcTextSize(("Total Money: $" + totalMoney.toString()).c_str()).x);
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Total Money: $%s", totalMoney.toString().c_str());
    }
    
    ImGui::Spacing();
    CenterContent(ImGui::CalcTextSize(("Last Transaction: " + app.dataMap["Last Transaction"]).c_str()).x);
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.4f, 1.0f), "Last Transaction: %s", app.dataMap["Last Transaction"].c_str());
    
    if (app.dataMap["Short Note"] != "-") {
        ImGui::Spacing();
        CenterContent(ImGui::CalcTextSize(("Note: " + app.dataMap["Short Note"]).c_str()).x);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Note: %s", app.dataMap["Short Note"].c_str());
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    
    // Menu buttons
    ImVec2 buttonSize(250, 50);
    CenterContent(buttonSize.x);
    
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0.5f, 0.5f));
    
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.5f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.6f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.4f, 0.8f, 1.0f));
    if (ImGui::Button("VIEW DATA", buttonSize)) {
        app.currentState = AppState::VIEW_DATA;
    }
    ImGui::PopStyleColor(3);
    
    CenterContent(buttonSize.x);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.9f, 0.5f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.6f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.8f, 0.4f, 0.1f, 1.0f));
    if (ImGui::Button("TRANSACTION", buttonSize)) {
        app.currentState = AppState::TRANSACTION;
    }
    ImGui::PopStyleColor(3);
    
    CenterContent(buttonSize.x);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.2f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.3f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.1f, 0.8f, 1.0f));
    if (ImGui::Button("EDIT NOTE", buttonSize)) {
        app.noteInput = app.dataMap["Short Note"];
        if (app.noteInput == "-") app.noteInput.clear();
        app.currentState = AppState::NOTE;
    }
    ImGui::PopStyleColor(3);
    
    CenterContent(buttonSize.x);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.9f, 0.5f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 1.0f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.8f, 0.4f, 1.0f));
    if (ImGui::Button("BORROWERS", buttonSize)) {
        app.currentState = AppState::BORROWERS;
    }
    ImGui::PopStyleColor(3);
    
    ImGui::PopStyleVar();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Bottom buttons
    CenterContent(320);
    
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
    if (ImGui::Button("SAVE & EXIT", ImVec2(100, 35))) {
        try {
            // Save logic here
            std::string dataString = dataToString(app.dataMap, app.borrowersMap, app.orderVector);
            int remainder = dataString.length() % 16;
            if (remainder != 0) {
                int starCount = 16 - remainder;
                starCount = (starCount < 5 ? starCount + 11 : starCount - 5);
                dataString.append(starCount, '*');
            }
            dataString += "valid";
            std::string encrypted = encryptAesCng(dataString, app.userKey);
            
            if (!saveToFile(encrypted, DATA_FILE)) {
                app.setStatus("Data saved successfully!", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                exit(0);
            } else {
                app.showAlert("Failed to save data!");
            }
        } catch (const std::exception& e) {
            app.showAlert("Error saving data!");
        }
    }
    ImGui::PopStyleColor(3);
    
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.7f, 0.5f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.6f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.6f, 0.4f, 0.1f, 1.0f));
    if (ImGui::Button("EXIT ONLY", ImVec2(100, 35))) {
        exit(0);
    }
    ImGui::PopStyleColor(3);
    
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
    if (ImGui::Button("RESET ALL", ImVec2(100, 35))) {
        app.showResetDialog = true;
    }
    ImGui::PopStyleColor(3);
    
    // Status message
    if (!app.statusMessage.empty()) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        CenterContent(ImGui::CalcTextSize(app.statusMessage.c_str()).x);
        ImGui::TextColored(app.statusColor, "%s", app.statusMessage.c_str());
    }
    
    ImGui::PopStyleVar();
    ImGui::End();
}

void renderViewData(AppData& app) {
    CenterWindow(ImVec2(500, 500));
    
    ImGui::Begin("View Data", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 12));
    
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    CenterContent(ImGui::CalcTextSize("MONEY TRACKER DATA").x);
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "MONEY TRACKER DATA");
    ImGui::PopFont();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Main data
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.15f, 0.25f, 0.15f, 0.8f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    ImGui::BeginChild("MainData", ImVec2(0, 120), true);
    
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
    
    BigNumber totalMoney(app.dataMap["Total Money"]);
    if (totalMoney.isNegative()) {
        CenterContent(ImGui::CalcTextSize(("Total Money: $" + totalMoney.toString()).c_str()).x);
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Total Money: $%s", totalMoney.toString().c_str());
    } else {
        CenterContent(ImGui::CalcTextSize(("Total Money: $" + totalMoney.toString()).c_str()).x);
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Total Money: $%s", totalMoney.toString().c_str());
    }
    
    ImGui::Spacing();
    CenterContent(ImGui::CalcTextSize(("Last Transaction: " + app.dataMap["Last Transaction"]).c_str()).x);
    ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.4f, 1.0f), "Last Transaction: %s", app.dataMap["Last Transaction"].c_str());
    
    ImGui::Spacing();
    CenterContent(ImGui::CalcTextSize(("Short Note: " + app.dataMap["Short Note"]).c_str()).x);
    ImGui::TextColored(ImVec4(0.7f, 0.7f, 1.0f, 1.0f), "Short Note: %s", app.dataMap["Short Note"].c_str());
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    
    // Borrowers data
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.25f, 0.15f, 0.15f, 0.8f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    ImGui::BeginChild("BorrowersData", ImVec2(0, 250), true);
    
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
    CenterContent(ImGui::CalcTextSize("BORROWERS & LENDERS").x);
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "BORROWERS & LENDERS");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    if (app.orderVector.size() > 3) {
        for (size_t i = 3; i < app.orderVector.size(); i++) {
            std::string name = app.orderVector[i];
            std::string amount = app.borrowersMap[name];
            
            if (amount[0] == '-') {
                CenterContent(ImGui::CalcTextSize(("[OWES YOU] " + name + ": $" + amount.substr(1)).c_str()).x);
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.6f, 1.0f), "[OWES YOU] %s: $%s", 
                    name.c_str(), amount.substr(1).c_str());
            } else {
                CenterContent(ImGui::CalcTextSize(("[YOU OWE] " + name + ": $" + amount).c_str()).x);
                ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "[YOU OWE] %s: $%s", 
                    name.c_str(), amount.c_str());
            }
        }
    } else {
        CenterContent(ImGui::CalcTextSize("No borrowers/lenders").x);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No borrowers/lenders");
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    CenterContent(150);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.5f, 0.8f, 1.0f));
    if (ImGui::Button("BACK TO MENU", ImVec2(150, 40))) {
        app.currentState = AppState::MAIN_MENU;
    }
    ImGui::PopStyleColor(3);
    
    ImGui::PopStyleVar();
    ImGui::End();
}

void renderTransaction(AppData& app) {
    CenterWindow(ImVec2(450, 500));
    
    ImGui::Begin("Make Transaction", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 15));
    
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    CenterContent(ImGui::CalcTextSize("NEW TRANSACTION").x);
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "NEW TRANSACTION");
    ImGui::PopFont();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    BigNumber currentTotal(app.dataMap["Total Money"]);
    if (currentTotal.isNegative()) {
        CenterContent(ImGui::CalcTextSize(("Current Total: $" + currentTotal.toString()).c_str()).x);
        ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Current Total: $%s", currentTotal.toString().c_str());
    } else {
        CenterContent(ImGui::CalcTextSize(("Current Total: $" + currentTotal.toString()).c_str()).x);
        ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Current Total: $%s", currentTotal.toString().c_str());
    }
    
    ImGui::Spacing();
    ImGui::Text("Transaction Type:");
    
    CenterContent(250);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 0.4f, 1.0f));
    if (ImGui::RadioButton("INCOME (+)", app.transactionIsPositive)) {
        app.transactionIsPositive = true;
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f));
    if (ImGui::RadioButton("EXPENSE (-)", !app.transactionIsPositive)) {
        app.transactionIsPositive = false;
    }
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Text("Amount (max 50 digits, decimals allowed):");
    ImGui::SetNextItemWidth(-1);
    InputTextString("##amount", &app.transactionValueInput, ImGuiInputTextFlags_CharsDecimal);
    
    ImGui::Spacing();
    CenterContent(200);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
    if (ImGui::Button("APPLY TRANSACTION", ImVec2(180, 40))) {
        if (app.transactionValueInput.empty()) {
            app.showAlert("Please enter an amount!");
        } else if (!isValidNumber(app.transactionValueInput)) {
            app.showAlert("Invalid amount (max 50 digits)!");
        } else {
            try {
                BigNumber currentTotal(app.dataMap["Total Money"]);
                BigNumber transactionAmount(app.transactionValueInput);
                
                if (app.transactionIsPositive) {
                    app.dataMap["Last Transaction"] = app.transactionValueInput;
                    app.dataMap["Total Money"] = (currentTotal + transactionAmount).toString();
                } else {
                    app.dataMap["Last Transaction"] = "-" + app.transactionValueInput;
                    app.dataMap["Total Money"] = (currentTotal - transactionAmount).toString();
                }
                
                app.setStatus("Transaction completed!", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                app.transactionValueInput.clear();
            } catch (const std::exception& e) {
                app.showAlert("Error processing transaction!");
            }
        }
    }
    ImGui::PopStyleColor(3);
    
    ImGui::Spacing();
    CenterContent(100);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    if (ImGui::Button("BACK", ImVec2(100, 35))) {
        app.currentState = AppState::MAIN_MENU;
        app.statusMessage.clear(); // Clear status when going back
    }
    ImGui::PopStyleColor(3);
    
    // Status message
    if (!app.statusMessage.empty()) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        CenterContent(ImGui::CalcTextSize(app.statusMessage.c_str()).x);
        ImGui::TextColored(app.statusColor, "%s", app.statusMessage.c_str());
    }
    
    ImGui::PopStyleVar();
    ImGui::End();
}

void renderNote(AppData& app) {
    CenterWindow(ImVec2(500, 400));
    
    ImGui::Begin("Edit Note", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 12));
    
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    CenterContent(ImGui::CalcTextSize("EDIT SHORT NOTE").x);
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "EDIT SHORT NOTE");
    ImGui::PopFont();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::Text("Note (max 1000 characters):");
    InputTextMultilineString("##note", &app.noteInput, ImVec2(-1, 150));
    
    ImGui::Text("Characters: %zu/1000", app.noteInput.length());
    
    ImGui::Spacing();
    CenterContent(200);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
    if (ImGui::Button("SAVE NOTE", ImVec2(100, 35))) {
        if (!isValidNote(app.noteInput)) {
            app.showAlert("Note too long (max 1000 chars)!");
        } else {
            app.dataMap["Short Note"] = app.noteInput.empty() ? "-" : app.noteInput;
            app.setStatus("Note saved!", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        }
    }
    ImGui::PopStyleColor(3);
    
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    if (ImGui::Button("BACK", ImVec2(100, 35))) {
        app.currentState = AppState::MAIN_MENU;
        app.statusMessage.clear(); // Clear status when going back
    }
    ImGui::PopStyleColor(3);
    
    // Status message
    if (!app.statusMessage.empty()) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        CenterContent(ImGui::CalcTextSize(app.statusMessage.c_str()).x);
        ImGui::TextColored(app.statusColor, "%s", app.statusMessage.c_str());
    }
    
    ImGui::PopStyleVar();
    ImGui::End();
}

void renderBorrowers(AppData& app) {
    CenterWindow(ImVec2(600, 700));
    
    ImGui::Begin("Manage Borrowers/Lenders", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
    
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 12));
    
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);
    CenterContent(ImGui::CalcTextSize("BORROWERS & LENDERS").x);
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "BORROWERS & LENDERS");
    ImGui::PopFont();
    
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    // Current borrowers list
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.25f, 0.15f, 0.15f, 0.8f));
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 12.0f);
    ImGui::BeginChild("CurrentBorrowers", ImVec2(0, 180), true);
    
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
    CenterContent(ImGui::CalcTextSize("Current Records").x);
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.3f, 1.0f), "Current Records");
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();
    
    if (app.orderVector.size() > 3) {
        for (size_t i = 3; i < app.orderVector.size(); i++) {
            std::string name = app.orderVector[i];
            std::string amount = app.borrowersMap[name];
            
            if (amount[0] == '-') {
                CenterContent(ImGui::CalcTextSize(("[OWES YOU] " + name + ": $" + amount.substr(1)).c_str()).x);
                ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.6f, 1.0f), "[OWES YOU] %s: $%s", 
                    name.c_str(), amount.substr(1).c_str());
            } else {
                CenterContent(ImGui::CalcTextSize(("[YOU OWE] " + name + ": $" + amount).c_str()).x);
                ImGui::TextColored(ImVec4(0.6f, 1.0f, 0.6f, 1.0f), "[YOU OWE] %s: $%s", 
                    name.c_str(), amount.c_str());
            }
        }
    } else {
        CenterContent(ImGui::CalcTextSize("No records").x);
        ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No records");
    }
    ImGui::EndChild();
    ImGui::PopStyleVar();
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    
    // Add new borrower/lender
    ImGui::Text("Add New Record:");
    ImGui::Separator();
    ImGui::Spacing();
    
    ImGui::Text("Who borrowed money?");
    CenterContent(280);
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 1.0f, 0.6f, 1.0f));
    if (ImGui::RadioButton("YOU BORROWED", app.borrowerIsYou)) {
        app.borrowerIsYou = true;
    }
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.6f, 0.6f, 1.0f));
    if (ImGui::RadioButton("THEY BORROWED", !app.borrowerIsYou)) {
        app.borrowerIsYou = false;
    }
    ImGui::PopStyleColor();
    
    ImGui::Spacing();
    ImGui::Text("Name (max 50 characters):");
    ImGui::SetNextItemWidth(-1);
    InputTextString("##borrowername", &app.borrowerNameInput);
    
    ImGui::Spacing();
    ImGui::Text("Amount (max 50 digits, decimals allowed):");
    ImGui::SetNextItemWidth(-1);
    InputTextString("##borroweramount", &app.borrowerValueInput, ImGuiInputTextFlags_CharsDecimal);
    
    ImGui::Spacing();
    CenterContent(220);
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.7f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.8f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.6f, 0.1f, 1.0f));
    if (ImGui::Button("ADD RECORD", ImVec2(120, 35))) {
        if (app.borrowerNameInput.empty() || app.borrowerValueInput.empty()) {
            app.showAlert("Please fill all fields!");
        } else if (!isValidName(app.borrowerNameInput)) {
            app.showAlert("Invalid name (max 50 chars)!");
        } else if (!isValidNumber(app.borrowerValueInput)) {
            app.showAlert("Invalid amount (max 50 digits)!");
        } else {
            try {
                BigNumber currentTotal(app.dataMap["Total Money"]);
                BigNumber borrowAmount(app.borrowerValueInput);
                
                if (app.borrowerIsYou) {
                    // You borrowed money (positive for you, you owe them)
                    if (app.orderVector.size() > 3 && std::find(app.orderVector.begin() + 3, app.orderVector.end(), app.borrowerNameInput) != app.orderVector.end()) {
                        BigNumber existingAmount(app.borrowersMap[app.borrowerNameInput]);
                        borrowAmount = existingAmount + borrowAmount;
                    } else {
                        app.orderVector.push_back(app.borrowerNameInput);
                    }
                    app.borrowersMap[app.borrowerNameInput] = borrowAmount.toString();
                    app.dataMap["Total Money"] = (currentTotal + BigNumber(app.borrowerValueInput)).toString();
                } else {
                    // They borrowed money (negative for them, they owe you)
                    if (app.orderVector.size() > 3 && std::find(app.orderVector.begin() + 3, app.orderVector.end(), app.borrowerNameInput) != app.orderVector.end()) {
                        BigNumber existingAmount(app.borrowersMap[app.borrowerNameInput]);
                        borrowAmount = existingAmount - borrowAmount;
                        app.borrowersMap[app.borrowerNameInput] = borrowAmount.toString();
                    } else {
                        app.borrowersMap[app.borrowerNameInput] = "-" + app.borrowerValueInput;
                        app.orderVector.push_back(app.borrowerNameInput);
                    }
                    app.dataMap["Total Money"] = (currentTotal - BigNumber(app.borrowerValueInput)).toString();
                }
                
                app.setStatus("Record added!", ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
                app.borrowerNameInput.clear();
                app.borrowerValueInput.clear();
            } catch (const std::exception& e) {
                app.showAlert("Error processing record!");
            }
        }
    }
    ImGui::PopStyleColor(3);
    
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
    if (ImGui::Button("BACK", ImVec2(100, 35))) {
        app.currentState = AppState::MAIN_MENU;
        app.statusMessage.clear(); // Clear status when going back
    }
    ImGui::PopStyleColor(3);
    
    // Status message
    if (!app.statusMessage.empty()) {
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        CenterContent(ImGui::CalcTextSize(app.statusMessage.c_str()).x);
        ImGui::TextColored(app.statusColor, "%s", app.statusMessage.c_str());
    }
    
    ImGui::PopStyleVar();
    ImGui::End();
}

void renderResetDialog(AppData& app) {
    if (app.showResetDialog) {
        ImGui::OpenPopup("Reset Confirmation");
    }
    
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    if (ImGui::BeginPopupModal("Reset Confirmation", &app.showResetDialog, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 12));
        
        CenterContent(ImGui::CalcTextSize("WARNING!").x);
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "WARNING!");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        CenterContent(ImGui::CalcTextSize("This will delete ALL your data permanently!").x);
        ImGui::Text("This will delete ALL your data permanently!");
        CenterContent(ImGui::CalcTextSize("This action cannot be undone.").x);
        ImGui::Text("This action cannot be undone.");
        
        ImGui::Spacing();
        CenterContent(280);
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.3f, 0.3f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.7f, 0.1f, 0.1f, 1.0f));
        if (ImGui::Button("YES, DELETE EVERYTHING", ImVec2(200, 35))) {
            try {
                if (std::filesystem::remove(DATA_FILE)) {
                    app.setStatus("All data deleted. Restart the application.", ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
                    exit(0);
                } else {
                    app.showAlert("Could not delete data file.");
                }
            } catch (const std::exception& e) {
                app.showAlert("Error deleting file.");
            }
            app.showResetDialog = false;
        }
        ImGui::PopStyleColor(3);
        
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
        if (ImGui::Button("CANCEL", ImVec2(80, 35))) {
            app.showResetDialog = false;
        }
        ImGui::PopStyleColor(3);
        
        ImGui::PopStyleVar();
        ImGui::EndPopup();
    }
}

void renderErrorAlert(AppData& app) {
    if (app.showErrorAlert) {
        ImGui::OpenPopup("Error Alert");
    }
    
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
    if (ImGui::BeginPopupModal("Error Alert", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 12));
        
        CenterContent(ImGui::CalcTextSize("ERROR").x);
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "ERROR");
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();
        
        ImGui::TextWrapped("%s", app.alertMessage.c_str());
        
        ImGui::Spacing();
        CenterContent(80);
        
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.9f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 1.0f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.1f, 0.5f, 0.8f, 1.0f));
        if (ImGui::Button("OK", ImVec2(80, 35))) {
            app.showErrorAlert = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::PopStyleColor(3);
        
        ImGui::PopStyleVar();
        ImGui::EndPopup();
    }
}

int main() {
    // Initialize GLFW
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    const char* glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);

    // Create window with better size
    GLFWwindow* window = glfwCreateWindow(1000, 800, "Money Tracker - Professional Edition", nullptr, nullptr);
    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    SetGLFWWindowIcon(window);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Setup Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    // ADD THIS LINE TO DISABLE .ini FILE:
    io.IniFilename = nullptr;

    // Load custom font
    LoadCustomFont(io);

    // Setup modern style with enhanced colors and professional look
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    
    // Enhanced rounded corners
    style.WindowRounding = 15.0f;
    style.FrameRounding = 8.0f;
    style.ScrollbarRounding = 8.0f;
    style.GrabRounding = 8.0f;
    style.TabRounding = 8.0f;
    style.ChildRounding = 12.0f;
    style.PopupRounding = 10.0f;
    
    // Better padding and spacing
    style.WindowPadding = ImVec2(20, 20);
    style.FramePadding = ImVec2(12, 8);
    style.ItemSpacing = ImVec2(12, 8);
    style.ItemInnerSpacing = ImVec2(8, 6);
    style.IndentSpacing = 25.0f;
    style.ScrollbarSize = 16.0f;
    style.GrabMinSize = 12.0f;
    
    // Professional color scheme with gradients
    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.10f, 0.95f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.16f, 0.90f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.13f, 0.95f);
    colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.35f, 0.8f);
    colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.3f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.12f, 0.12f, 0.18f, 0.8f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.24f, 0.9f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.24f, 0.24f, 0.30f, 1.0f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.13f, 1.0f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.18f, 1.0f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.08f, 0.08f, 0.13f, 0.7f);
    colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.10f, 0.16f, 1.0f);
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.08f, 0.13f, 0.5f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.25f, 0.25f, 0.35f, 0.8f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.35f, 0.35f, 0.45f, 0.9f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.45f, 0.45f, 0.55f, 1.0f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.2f, 0.8f, 1.0f, 1.0f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.2f, 0.8f, 1.0f, 0.8f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.3f, 0.9f, 1.0f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.15f, 0.15f, 0.22f, 1.0f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.20f, 0.20f, 0.28f, 1.0f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.25f, 0.32f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.18f, 0.35f, 0.65f, 0.8f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.22f, 0.40f, 0.75f, 0.9f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.26f, 0.45f, 0.85f, 1.0f);
    colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.25f, 0.35f, 0.6f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.35f, 0.35f, 0.45f, 0.8f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(0.45f, 0.45f, 0.55f, 1.0f);
    colors[ImGuiCol_ResizeGrip] = ImVec4(0.2f, 0.8f, 1.0f, 0.3f);
    colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.3f, 0.9f, 1.0f, 0.6f);
    colors[ImGuiCol_ResizeGripActive] = ImVec4(0.4f, 1.0f, 1.0f, 0.9f);
    colors[ImGuiCol_Tab] = ImVec4(0.12f, 0.12f, 0.18f, 0.8f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.18f, 0.35f, 0.65f, 0.8f);
    colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.40f, 0.75f, 1.0f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.10f, 0.10f, 0.16f, 0.6f);
    colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.15f, 0.22f, 0.8f);
    colors[ImGuiCol_PlotLines] = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
    colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.0f, 0.8f, 0.35f, 1.0f);
    colors[ImGuiCol_PlotHistogram] = ImVec4(0.9f, 0.7f, 0.0f, 1.0f);
    colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.0f, 0.8f, 0.0f, 1.0f);
    colors[ImGuiCol_TableHeaderBg] = ImVec4(0.15f, 0.15f, 0.22f, 1.0f);
    colors[ImGuiCol_TableBorderStrong] = ImVec4(0.25f, 0.25f, 0.35f, 1.0f);
    colors[ImGuiCol_TableBorderLight] = ImVec4(0.20f, 0.20f, 0.28f, 1.0f);
    colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
    colors[ImGuiCol_TableRowBgAlt] = ImVec4(0.15f, 0.15f, 0.22f, 0.3f);
    colors[ImGuiCol_TextSelectedBg] = ImVec4(0.2f, 0.8f, 1.0f, 0.35f);
    colors[ImGuiCol_DragDropTarget] = ImVec4(0.2f, 0.8f, 1.0f, 0.9f);
    colors[ImGuiCol_NavHighlight] = ImVec4(0.2f, 0.8f, 1.0f, 1.0f);
    colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
    colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
    colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.6f);

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Application state
    AppData app;
    
    // Check if data file exists
    try {
        bool readErr = false;
        loadFile(DATA_FILE, readErr);
        app.fileExists = !readErr;
    } catch (const std::exception& e) {
        app.fileExists = false;
    }

    // Professional background gradient
    ImVec4 clear_color = ImVec4(0.04f, 0.04f, 0.08f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Clear status message after some time (enhanced timing)
        static int statusCounter = 0;
        if (!app.statusMessage.empty()) {
            statusCounter++;
            if (statusCounter > 240) { // ~4 seconds at 60fps
                app.statusMessage.clear();
                statusCounter = 0;
            }
        } else {
            statusCounter = 0;
        }

        // Render current screen with enhanced error handling
        try {
            switch (app.currentState) {
                case AppState::LOGIN:
                case AppState::NEW_USER:
                    renderLoginScreen(app);
                    break;
                case AppState::MAIN_MENU:
                    renderMainMenu(app);
                    break;
                case AppState::VIEW_DATA:
                    renderViewData(app);
                    break;
                case AppState::TRANSACTION:
                    renderTransaction(app);
                    break;
                case AppState::NOTE:
                    renderNote(app);
                    break;
                case AppState::BORROWERS:
                    renderBorrowers(app);
                    break;
            }

            // Handle dialogs
            renderResetDialog(app);
            renderErrorAlert(app);
        } catch (const std::exception& e) {
            app.showAlert("Unexpected error occurred!");
        }

        // Enhanced rendering with better performance
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        
        // Gradient background
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}