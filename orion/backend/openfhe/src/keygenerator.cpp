#include "keygenerator.hpp"
#include "scheme.hpp"
#include "tensors.hpp"
#include "openfhe.h"
#include "cryptocontext-ser.h"
#include "ciphertext-ser.h"
#include "key/key-ser.h"
#include "scheme/ckksrns/ckksrns-ser.h"
#include <iostream>
#include <memory>
#include <sstream>
#include <cstring>

using namespace lbcrypto;

extern "C" {

void NewKeyGenerator() {
    if (!g_scheme.IsInitialized() || !g_scheme.context) {
        std::cerr << "Error: CryptoContext not initialized" << std::endl;
        return;
    }

    // In OpenFHE, key generation is handled directly by the CryptoContext
    // We'll generate the key pair when needed
    std::cout << "Key generator ready (using CryptoContext)" << std::endl;
}

void GenerateSecretKey() {
    if (!g_scheme.IsInitialized() || !g_scheme.context) {
        std::cerr << "Error: CryptoContext not initialized" << std::endl;
        return;
    }

    // In OpenFHE, KeyGen() generates both keys at once
    g_scheme.keyPair = g_scheme.context->KeyGen();
    g_scheme.secretKey = g_scheme.keyPair.secretKey;
    g_scheme.publicKey = g_scheme.keyPair.publicKey;

    if (!g_scheme.secretKey) {
        std::cerr << "Error: Failed to generate secret key" << std::endl;
    } else {
        std::cout << "Secret and public keys generated successfully" << std::endl;
    }
}

void GeneratePublicKey() {
    // This is a no-op since OpenFHE generates both keys together
    // But we maintain the interface for compatibility
    if (!g_scheme.keyPair.publicKey) {
        std::cerr << "Error: Key pair not generated - call GenerateSecretKey first" << std::endl;
        return;
    }

    std::cout << "Public key already available from key pair" << std::endl;
}

void GenerateRelinearizationKey() {
    if (!g_scheme.IsInitialized() || !g_scheme.context || !g_scheme.secretKey) {
        std::cerr << "Error: CryptoContext or SecretKey not initialized" << std::endl;
        return;
    }

    g_scheme.context->EvalMultKeyGen(g_scheme.secretKey);
    std::cout << "Relinearization keys generated" << std::endl;
}

void GenerateEvaluationKeys() {
    if (!g_scheme.IsInitialized() || !g_scheme.context || !g_scheme.secretKey) {
        std::cerr << "Error: CryptoContext or SecretKey not initialized" << std::endl;
        return;
    }

    // Generate rotation keys for common rotation amounts
    std::vector<int32_t> indexList;
    uint32_t batchSize = g_scheme.context->GetEncodingParams()->GetBatchSize();

    // Add common rotation indices
    for (uint32_t i = 1; i < batchSize; i *= 2) {
        indexList.push_back(i);
        indexList.push_back(-i);
    }

    g_scheme.context->EvalRotateKeyGen(g_scheme.secretKey, indexList);
    std::cout << "Evaluation keys (rotation keys) generated" << std::endl;
}

char* SerializeSecretKey(unsigned long* length) {
    if (!g_scheme.secretKey) {
        std::cerr << "Error: SecretKey not initialized" << std::endl;
        *length = 0;
        return nullptr;
    }

    try {
        std::ostringstream os;
        Serial::Serialize(g_scheme.secretKey, os, SerType::BINARY);
        std::string serialized = os.str();

        *length = serialized.length();
        char* result = (char*)malloc(*length);
        if (result) {
            std::memcpy(result, serialized.c_str(), *length);
        }

        return result;
    } catch (const std::exception& e) {
        std::cerr << "Error serializing secret key: " << e.what() << std::endl;
        *length = 0;
        return nullptr;
    }
}

void LoadSecretKey(const char* data, unsigned long length) {
    if (!data || length == 0) {
        std::cerr << "Error: Invalid serialized data" << std::endl;
        return;
    }

    try {
        std::string serialized(data, length);
        std::istringstream is(serialized);

        PrivateKey<DCRTPoly> sk;
        Serial::Deserialize(sk, is, SerType::BINARY);
        g_scheme.secretKey = sk;
    } catch (const std::exception& e) {
        std::cerr << "Error loading secret key: " << e.what() << std::endl;
    }
}

char* SerializePublicKey(unsigned long* length) {
    if (!g_scheme.publicKey) {
        std::cerr << "Error: PublicKey not initialized" << std::endl;
        *length = 0;
        return nullptr;
    }

    try {
        std::ostringstream os;
        Serial::Serialize(g_scheme.publicKey, os, SerType::BINARY);
        std::string serialized = os.str();

        *length = serialized.length();
        char* result = (char*)malloc(*length);
        if (result) {
            std::memcpy(result, serialized.c_str(), *length);
        }

        return result;
    } catch (const std::exception& e) {
        std::cerr << "Error serializing public key: " << e.what() << std::endl;
        *length = 0;
        return nullptr;
    }
}

void LoadPublicKey(const char* data, unsigned long length) {
    if (!data || length == 0) {
        std::cerr << "Error: Invalid serialized data" << std::endl;
        return;
    }

    try {
        std::string serialized(data, length);
        std::istringstream is(serialized);

        PublicKey<DCRTPoly> pk;
        Serial::Deserialize(pk, is, SerType::BINARY);
        g_scheme.publicKey = pk;
    } catch (const std::exception& e) {
        std::cerr << "Error loading public key: " << e.what() << std::endl;
    }
}

} // extern "C"