#define NOMINMAX
#include <windows.h>
#include <bcrypt.h>
#include <string>

#pragma comment(lib, "bcrypt.lib")


std::string encryptAesCng(const std::string& plaintext, const std::string& keyStr) {
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_KEY_HANDLE hKey = nullptr;
    NTSTATUS status;
    DWORD cbKeyObject = 0, cbData = 0, cbCipherText = 0;
    PBYTE pbKeyObject = nullptr, pbCipherText = nullptr;

    // Convert key to 256-bit (32 bytes)
    BYTE key[32] = { 0 };
    memcpy(key, keyStr.data(), std::min(keyStr.size(), sizeof(key)));

    BYTE iv[16] = { 0 }; // For CBC, IV must be 16 bytes

    // Open AES provider
    if (!BCRYPT_SUCCESS(status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0))) goto cleanup;

    // Set CBC mode
    if (!BCRYPT_SUCCESS(status = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0))) goto cleanup;

    // Get key object size
    if (!BCRYPT_SUCCESS(status = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&cbKeyObject, sizeof(cbKeyObject), &cbData, 0))) goto cleanup;
    pbKeyObject = (PUCHAR)HeapAlloc(GetProcessHeap(), 0, cbKeyObject);
    if (!pbKeyObject) goto cleanup;

    // Generate key
    if (!BCRYPT_SUCCESS(status = BCryptGenerateSymmetricKey(hAlg, &hKey, pbKeyObject, cbKeyObject, key, sizeof(key), 0))) goto cleanup;

    // Get ciphertext size
    if (!BCRYPT_SUCCESS(status = BCryptEncrypt(hKey, (PUCHAR)plaintext.data(), (ULONG)plaintext.size(), nullptr, iv, sizeof(iv), nullptr, 0, &cbCipherText, 0))) goto cleanup;
    pbCipherText = (PUCHAR)HeapAlloc(GetProcessHeap(), 0, cbCipherText);
    if (!pbCipherText) goto cleanup;

    // Encrypt
    if (!BCRYPT_SUCCESS(status = BCryptEncrypt(hKey, (PUCHAR)plaintext.data(), (ULONG)plaintext.size(), nullptr, iv, sizeof(iv), pbCipherText, cbCipherText, &cbData, 0))) goto cleanup;

    // Return as std::string
    return std::string((char*)pbCipherText, cbData);

cleanup:
    if (hKey) BCryptDestroyKey(hKey);
    if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
    if (pbKeyObject) HeapFree(GetProcessHeap(), 0, pbKeyObject);
    if (pbCipherText) HeapFree(GetProcessHeap(), 0, pbCipherText);
    return ""; // Empty string on error
}


std::string decryptAesCng(const std::string& ciphertext, const std::string& keyStr) {
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCRYPT_KEY_HANDLE hKey = nullptr;
    NTSTATUS status;
    DWORD cbKeyObject = 0, cbData = 0, cbPlainText = 0;
    PBYTE pbKeyObject = nullptr, pbPlainText = nullptr;

    // Convert key to 256-bit (32 bytes)
    BYTE key[32] = { 0 };
    memcpy(key, keyStr.data(), std::min(keyStr.size(), sizeof(key)));

    BYTE iv[16] = { 0 }; // Same IV used in encryption (all zeros here)

    // Open AES algorithm provider
    if (!BCRYPT_SUCCESS(status = BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_AES_ALGORITHM, NULL, 0))) goto cleanup;

    // Set chaining mode to CBC
    if (!BCRYPT_SUCCESS(status = BCryptSetProperty(hAlg, BCRYPT_CHAINING_MODE, (PUCHAR)BCRYPT_CHAIN_MODE_CBC, sizeof(BCRYPT_CHAIN_MODE_CBC), 0))) goto cleanup;

    // Get size for key object
    if (!BCRYPT_SUCCESS(status = BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PUCHAR)&cbKeyObject, sizeof(cbKeyObject), &cbData, 0))) goto cleanup;
    pbKeyObject = (PUCHAR)HeapAlloc(GetProcessHeap(), 0, cbKeyObject);
    if (!pbKeyObject) goto cleanup;

    // Generate key
    if (!BCRYPT_SUCCESS(status = BCryptGenerateSymmetricKey(hAlg, &hKey, pbKeyObject, cbKeyObject, key, sizeof(key), 0))) goto cleanup;

    // Get required size for plaintext
    if (!BCRYPT_SUCCESS(status = BCryptDecrypt(hKey, (PUCHAR)ciphertext.data(), (ULONG)ciphertext.size(), nullptr, iv, sizeof(iv), nullptr, 0, &cbPlainText, 0))) goto cleanup;
    pbPlainText = (PUCHAR)HeapAlloc(GetProcessHeap(), 0, cbPlainText);
    if (!pbPlainText) goto cleanup;

    // Perform decryption
    if (!BCRYPT_SUCCESS(status = BCryptDecrypt(hKey, (PUCHAR)ciphertext.data(), (ULONG)ciphertext.size(), nullptr, iv, sizeof(iv), pbPlainText, cbPlainText, &cbData, 0))) goto cleanup;

    return std::string((char*)pbPlainText, cbData);

cleanup:
    if (hKey) BCryptDestroyKey(hKey);
    if (hAlg) BCryptCloseAlgorithmProvider(hAlg, 0);
    if (pbKeyObject) HeapFree(GetProcessHeap(), 0, pbKeyObject);
    if (pbPlainText) HeapFree(GetProcessHeap(), 0, pbPlainText);
    return ""; // Empty string on failure
}