#pragma once

#include "openfhe/pke/openfhe.h"

using namespace lbcrypto;

/**
 * @brief OpenFHE Encryptor wrapper for Orion compatibility
 * 
 * This class provides encryption and decryption functionality using OpenFHE's
 * CKKS scheme. It manages the encryption/decryption operations and maintains
 * compatibility with the Lattigo backend interface.
 */
class OrionEncryptor {
private:
    bool initialized;

public:
    /**
     * @brief Construct a new Orion Encryptor object
     */
    OrionEncryptor() : initialized(false) {}

    /**
     * @brief Initialize the encryptor with the current scheme
     * 
     * @return true if initialization successful, false otherwise
     */
    bool Initialize();

    /**
     * @brief Check if encryptor is initialized
     * 
     * @return true if initialized, false otherwise
     */
    bool IsInitialized() const { return initialized; }

    /**
     * @brief Encrypt a plaintext to produce a ciphertext
     * 
     * @param plaintextID ID of the plaintext to encrypt
     * @return int Ciphertext ID if successful, -1 if failed
     */
    int Encrypt(int plaintextID);

    /**
     * @brief Decrypt a ciphertext to produce a plaintext
     * 
     * @param ciphertextID ID of the ciphertext to decrypt
     * @return int Plaintext ID if successful, -1 if failed
     */
    int Decrypt(int ciphertextID);

    // EncryptValues and DecryptValues functions removed - not used by Orion
    // Use Encode + Encrypt and Decrypt + Decode instead

    /**
     * @brief Clean up encryptor resources
     */
    void CleanUp();

    /**
     * @brief Destructor - ensures proper cleanup
     */
    ~OrionEncryptor() {
        CleanUp();
    }
};

// OrionDecryptor class removed - functionality merged into OrionEncryptor

// Global encryptor instance (handles both encryption and decryption)
extern OrionEncryptor g_encryptor;

// C interface functions for encryption operations
extern "C" {
    /**
     * @brief Initialize the encryptor (C interface)
     */
    void NewEncryptor();

    /**
     * @brief Initialize the decryptor (C interface)
     */
    void NewDecryptor();

    /**
     * @brief Encrypt a plaintext (C interface)
     * 
     * @param plaintextID ID of the plaintext to encrypt
     * @return int Ciphertext ID if successful, -1 if failed
     */
    int Encrypt(int plaintextID);

    /**
     * @brief Decrypt a ciphertext (C interface)
     * 
     * @param ciphertextID ID of the ciphertext to decrypt
     * @return int Plaintext ID if successful, -1 if failed
     */
    int Decrypt(int ciphertextID);
}