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

    /**
     * @brief Encrypt values directly (encode then encrypt)
     * 
     * @param values Vector of values to encrypt
     * @return int Ciphertext ID if successful, -1 if failed
     */
    int EncryptValues(const std::vector<double>& values);

    /**
     * @brief Decrypt and decode ciphertext to values
     * 
     * @param ciphertextID ID of the ciphertext to decrypt and decode
     * @return std::vector<double> Decrypted values (empty if failed)
     */
    std::vector<double> DecryptValues(int ciphertextID);

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

/**
 * @brief OpenFHE Decryptor wrapper (separate from encryptor in some contexts)
 * 
 * In OpenFHE, decryption uses the secret key while encryption uses the public key.
 * This class provides a separate interface for decryption operations.
 */
class OrionDecryptor {
private:
    bool initialized;

public:
    /**
     * @brief Construct a new Orion Decryptor object
     */
    OrionDecryptor() : initialized(false) {}

    /**
     * @brief Initialize the decryptor with the current scheme
     * 
     * @return true if initialization successful, false otherwise
     */
    bool Initialize();

    /**
     * @brief Check if decryptor is initialized
     * 
     * @return true if initialized, false otherwise
     */
    bool IsInitialized() const { return initialized; }

    /**
     * @brief Decrypt a ciphertext to produce a plaintext
     * 
     * @param ciphertextID ID of the ciphertext to decrypt
     * @return int Plaintext ID if successful, -1 if failed
     */
    int Decrypt(int ciphertextID);

    /**
     * @brief Clean up decryptor resources
     */
    void CleanUp();

    /**
     * @brief Destructor - ensures proper cleanup  
     */
    ~OrionDecryptor() {
        CleanUp();
    }
};

// Global encryptor and decryptor instances
extern OrionEncryptor g_encryptor;
extern OrionDecryptor g_decryptor;

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