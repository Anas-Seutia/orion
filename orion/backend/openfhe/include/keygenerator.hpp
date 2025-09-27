#pragma once

#include "openfhe/pke/openfhe.h"
#include <vector>
#include <string>

using namespace lbcrypto;

/**
 * @brief OpenFHE Key Generator for cryptographic operations
 *
 * This module provides key generation and management functionality for
 * the OpenFHE backend. It handles the generation of secret keys, public keys,
 * relinearization keys, and evaluation keys (rotation keys).
 */

// C interface functions for key generation operations
extern "C" {
    /**
     * @brief Initialize a new key generator
     */
    void NewKeyGenerator();

    /**
     * @brief Generate secret key (also generates public key in OpenFHE)
     */
    void GenerateSecretKey();

    /**
     * @brief Generate public key (no-op in OpenFHE as it's generated with secret key)
     */
    void GeneratePublicKey();

    /**
     * @brief Generate relinearization keys for multiplication operations
     */
    void GenerateRelinearizationKey();

    /**
     * @brief Generate evaluation keys (rotation keys) for homomorphic operations
     */
    void GenerateEvaluationKeys();

    /**
     * @brief Serialize secret key to binary data
     *
     * @param length Pointer to store the length of serialized data
     * @return char* Serialized key data (caller must free), nullptr if failed
     */
    char* SerializeSecretKey(unsigned long* length);

    /**
     * @brief Load secret key from serialized binary data
     *
     * @param data Serialized key data
     * @param length Length of the data
     */
    void LoadSecretKey(const char* data, unsigned long length);

    /**
     * @brief Serialize public key to binary data
     *
     * @param length Pointer to store the length of serialized data
     * @return char* Serialized key data (caller must free), nullptr if failed
     */
    char* SerializePublicKey(unsigned long* length);

    /**
     * @brief Load public key from serialized binary data
     *
     * @param data Serialized key data
     * @param length Length of the data
     */
    void LoadPublicKey(const char* data, unsigned long length);
}