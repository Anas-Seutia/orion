#pragma once

#include "openfhe/pke/openfhe.h"
#include <vector>
#include <string>
#include <memory>

using namespace lbcrypto;

/**
 * @brief OpenFHE Scheme wrapper for Orion compatibility
 * 
 * This class encapsulates all the OpenFHE cryptographic components needed
 * for homomorphic encryption operations. It manages the crypto context,
 * keys, and provides a unified interface matching the Lattigo backend structure.
 */
class OrionScheme {
private:
    bool initialized;

public:
    // Core OpenFHE components
    CryptoContext<DCRTPoly> context;
    KeyPair<DCRTPoly> keyPair;
    
    // Individual key components for convenience
    PublicKey<DCRTPoly> publicKey;
    PrivateKey<DCRTPoly> secretKey;
    EvalKey<DCRTPoly> relinKey;
    std::map<usint, EvalKey<DCRTPoly>> rotationKeys;

    /**
     * @brief Construct a new Orion Scheme object
     */
    OrionScheme() : initialized(false) {}

    /**
     * @brief Initialize the cryptographic scheme with given parameters
     * 
     * @param logN Log of ring dimension
     * @param logQ Vector of coefficient modulus bit-lengths
     * @param logP Vector of auxiliary modulus bit-lengths  
     * @param logScale Scaling factor bit-length
     * @param hammingWeight Hamming weight for secret key distribution
     * @param ringType Ring type (0 for standard, 1 for conjugate invariant)
     * @param keysPath Path for key storage (unused in memory mode)
     * @param ioMode I/O mode ("memory" or "file")
     * @return true if initialization successful, false otherwise
     */
    bool Initialize(int logN, const std::vector<int>& logQ, const std::vector<int>& logP,
                   int logScale, int hammingWeight, int ringType,
                   const std::string& keysPath, const std::string& ioMode);

    /**
     * @brief Generate and setup all necessary keys
     * 
     * @return true if key generation successful, false otherwise
     */
    bool GenerateKeys();

    /**
     * @brief Generate rotation key for a specific step
     * 
     * @param step Rotation step
     * @return true if key generation successful, false otherwise
     */
    bool GenerateRotationKey(int step);

    /**
     * @brief Generate all power-of-two rotation keys for efficient operations
     */
    void GeneratePowerOfTwoRotationKeys();

    // Bulk rotation key generation functions removed - Orion uses individual AddRotationKey calls

    /**
     * @brief Check if the scheme is properly initialized
     * 
     * @return true if initialized, false otherwise
     */
    bool IsInitialized() const { return initialized; }

    /**
     * @brief Get the maximum number of slots available
     * 
     * @return uint32_t Maximum slot count
     */
    uint32_t GetMaxSlots() const {
        return initialized ? context->GetEncodingParams()->GetBatchSize() : 0;
    }

    /**
     * @brief Get the current ring dimension
     * 
     * @return uint32_t Ring dimension
     */
    uint32_t GetRingDim() const {
        return initialized ? context->GetRingDimension() : 0;
    }

    /**
     * @brief Get the multiplicative depth
     * 
     * @return uint32_t Multiplicative depth
     */
    uint32_t GetMultiplicativeDepth() const {
        if (!initialized) return 0;

        auto cryptoParams = context->GetCryptoParameters();
        size_t numModuli = cryptoParams->GetElementParams()->GetParams().size();

        // Conservative estimate: multiplicative depth = numModuli - 1
        // This works for most CKKS configurations
        return (numModuli > 1) ? (numModuli - 1) : 0;
    }

    /**
     * @brief Clean up all resources and reset the scheme
     */
    void CleanUp();

    /**
     * @brief Destructor - ensures proper cleanup
     */
    ~OrionScheme() {
        CleanUp();
    }
};

// Global scheme instance (matching Lattigo backend pattern)
extern OrionScheme g_scheme;

// C interface functions for scheme management
extern "C" {
    /**
     * @brief Initialize a new cryptographic scheme
     * 
     * @param logN Log of ring dimension
     * @param logQ Array of coefficient modulus bit-lengths
     * @param logQSize Size of logQ array
     * @param logP Array of auxiliary modulus bit-lengths
     * @param logPSize Size of logP array
     * @param logScale Scaling factor bit-length
     * @param hammingWeight Hamming weight for secret key
     * @param ringType Ring type string
     * @param keysPath Path for key storage
     * @param ioMode I/O mode string
     */
    void NewScheme(int logN, int* logQ, int logQSize, int* logP, int logPSize,
                   int logScale, int hammingWeight, const char* ringType,
                   const char* keysPath, const char* ioMode);

    /**
     * @brief Delete the current scheme and clean up resources
     */
    void DeleteScheme();

    /**
     * @brief Check if scheme is initialized
     * 
     * @return int 1 if initialized, 0 otherwise
     */
    int IsSchemeInitialized();

    /**
     * @brief Generate rotation key for specific step
     * 
     * @param step Rotation step
     */
    void AddRotationKey(int step);
}