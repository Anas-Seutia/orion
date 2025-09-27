#include "encryptor.hpp"
#include "scheme.hpp"
#include "tensors.hpp"
#include <iostream>

// Global encryptor instance (handles both encryption and decryption)
OrionEncryptor g_encryptor;

// OrionEncryptor implementation
bool OrionEncryptor::Initialize() {
    if (!g_scheme.IsInitialized()) {
        std::cerr << "Cannot initialize encryptor: scheme not initialized" << std::endl;
        return false;
    }

    if (!g_scheme.publicKey) {
        std::cerr << "Cannot initialize encryptor: public key not available" << std::endl;
        return false;
    }

    try {
        // OpenFHE handles encryption through the CryptoContext
        // No separate encryptor object needed - just mark as initialized
        initialized = true;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Encryptor initialization failed: " << e.what() << std::endl;
        initialized = false;
        return false;
    }
}

int OrionEncryptor::Encrypt(int plaintextID) {
    if (!initialized || !g_scheme.IsInitialized()) {
        std::cerr << "Encryptor or scheme not initialized" << std::endl;
        return -1;
    }

    try {
        if (!PlaintextExists(plaintextID)) {
            std::cerr << "Plaintext ID " << plaintextID << " not found" << std::endl;
            return -1;
        }

        auto& plaintext = RetrievePlaintext(plaintextID);
        
        // Encrypt the plaintext using the public key
        auto ciphertext = g_scheme.context->Encrypt(g_scheme.publicKey, plaintext);

        // Store the ciphertext and return its ID
        return PushCiphertext(ciphertext);

    } catch (const std::exception& e) {
        std::cerr << "Encryption failed: " << e.what() << std::endl;
        return -1;
    }
}

int OrionEncryptor::Decrypt(int ciphertextID) {
    if (!initialized || !g_scheme.IsInitialized()) {
        std::cerr << "Encryptor or scheme not initialized" << std::endl;
        return -1;
    }

    if (!g_scheme.secretKey) {
        std::cerr << "Cannot decrypt: secret key not available" << std::endl;
        return -1;
    }

    try {
        if (!CiphertextExists(ciphertextID)) {
            std::cerr << "Ciphertext ID " << ciphertextID << " not found" << std::endl;
            return -1;
        }

        auto& ciphertext = RetrieveCiphertext(ciphertextID);
        
        // Decrypt the ciphertext using the secret key
        Plaintext plaintext;
        g_scheme.context->Decrypt(g_scheme.secretKey, ciphertext, &plaintext);

        // Store the plaintext and return its ID
        return PushPlaintext(plaintext);

    } catch (const std::exception& e) {
        std::cerr << "Decryption failed: " << e.what() << std::endl;
        return -1;
    }
}

// EncryptValues and DecryptValues functions removed - not used by Orion
// Use Encode + Encrypt and Decrypt + Decode instead

void OrionEncryptor::CleanUp() {
    initialized = false;
}

// OrionDecryptor class removed - functionality merged into OrionEncryptor

// C interface implementations
extern "C" {
    void NewEncryptor() {
        g_encryptor.Initialize();
    }

    void NewDecryptor() {
        // Decryptor functionality handled by encryptor
        g_encryptor.Initialize();
    }

    int Encrypt(int plaintextID) {
        return g_encryptor.Encrypt(plaintextID);
    }

    int Decrypt(int ciphertextID) {
        return g_encryptor.Decrypt(ciphertextID);
    }
}