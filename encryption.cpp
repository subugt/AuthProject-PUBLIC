#include "encryption.hpp"
#include <openssl/evp.h>
#include <openssl/aes.h>
#include <stdexcept>
#include <vector>

std::string Encryption::decrypt(const std::vector<uint8_t>& data, const std::string& key) {
    if (key.size() != 32) {
        throw std::runtime_error("Invalid key size, must be 32 bytes");
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_CIPHER_CTX");
    }

    std::vector<uint8_t> decrypted(data.size() + AES_BLOCK_SIZE);
    int len;
    int decrypted_len;

    if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, reinterpret_cast<const uint8_t*>(key.data()), NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptInit_ex failed");
    }

    if (1 != EVP_DecryptUpdate(ctx, decrypted.data(), &len, data.data(), data.size())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptUpdate failed");
    }
    decrypted_len = len;

    if (1 != EVP_DecryptFinal_ex(ctx, decrypted.data() + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_DecryptFinal_ex failed");
    }
    decrypted_len += len;

    EVP_CIPHER_CTX_free(ctx);
    decrypted.resize(decrypted_len);
    return std::string(decrypted.begin(), decrypted.end());
}

std::vector<uint8_t> Encryption::encrypt(const std::string& data, const std::string& key) {
    if (key.size() != 32) {
        throw std::runtime_error("Invalid key size, must be 32 bytes");
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (!ctx) {
        throw std::runtime_error("Failed to create EVP_CIPHER_CTX");
    }

    std::vector<uint8_t> encrypted(data.size() + AES_BLOCK_SIZE);
    int len;
    int encrypted_len;

    if (1 != EVP_EncryptInit_ex(ctx, EVP_aes_256_ecb(), NULL, reinterpret_cast<const uint8_t*>(key.data()), NULL)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptInit_ex failed");
    }

    if (1 != EVP_EncryptUpdate(ctx, encrypted.data(), &len, reinterpret_cast<const uint8_t*>(data.data()), data.size())) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptUpdate failed");
    }
    encrypted_len = len;

    if (1 != EVP_EncryptFinal_ex(ctx, encrypted.data() + len, &len)) {
        EVP_CIPHER_CTX_free(ctx);
        throw std::runtime_error("EVP_EncryptFinal_ex failed");
    }
    encrypted_len += len;

    EVP_CIPHER_CTX_free(ctx);
    encrypted.resize(encrypted_len);
    return encrypted;
}
