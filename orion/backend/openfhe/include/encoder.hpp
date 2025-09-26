#pragma once

#include "openfhe/pke/openfhe.h"
#include <vector>

using namespace lbcrypto;

/**
 * @brief OpenFHE Encoder wrapper for Orion compatibility
 * 
 * This class provides encoding and decoding functionality for the CKKS scheme,
 * converting between vectors of real numbers and OpenFHE plaintext objects.
 * It matches the interface and behavior of the Lattigo backend encoder.
 */
class OrionEncoder {
private:
    bool initialized;

public:
    /**
     * @brief Construct a new Orion Encoder object
     */
    OrionEncoder() : initialized(false) {}

    /**
     * @brief Initialize the encoder with the current scheme context
     * 
     * @return true if initialization successful, false otherwise
     */
    bool Initialize();

    /**
     * @brief Check if encoder is initialized
     * 
     * @return true if initialized, false otherwise
     */
    bool IsInitialized() const { return initialized; }

    /**
     * @brief Encode a vector of real values into a plaintext
     * 
     * @param values Vector of real values to encode
     * @param level Target level for the plaintext
     * @param scale Scaling factor for the encoding
     * @return int Plaintext ID if successful, -1 if failed
     */
    int Encode(const std::vector<double>& values, int level, uint64_t scale);

    /**
     * @brief Decode a plaintext into a vector of real values
     * 
     * @param plaintextID ID of the plaintext to decode
     * @return std::vector<double> Decoded values (empty if failed)
     */
    std::vector<double> Decode(int plaintextID);

    /**
     * @brief Encode values with automatic scale determination
     * 
     * @param values Vector of values to encode
     * @param level Target level
     * @return int Plaintext ID if successful, -1 if failed
     */
    int EncodeAtLevel(const std::vector<double>& values, int level);

    /**
     * @brief Get the maximum number of slots that can be encoded
     * 
     * @return uint32_t Maximum slot count
     */
    uint32_t GetSlotCount() const;

    /**
     * @brief Clean up encoder resources
     */
    void CleanUp();

    /**
     * @brief Destructor - ensures proper cleanup
     */
    ~OrionEncoder() {
        CleanUp();
    }
};

// Global encoder instance
extern OrionEncoder g_encoder;

// C interface functions for encoding operations
extern "C" {
    /**
     * @brief Initialize the encoder (C interface)
     */
    void NewEncoder();

    /**
     * @brief Encode values into plaintext (C interface)
     * 
     * @param values Pointer to array of values
     * @param lenValues Number of values in the array
     * @param level Target level for the plaintext
     * @param scale Scaling factor for the encoding
     * @return int Plaintext ID if successful, -1 if failed
     */
    int Encode(double* values, int lenValues, int level, uint64_t scale);

    /**
     * @brief Decode plaintext into values (C interface)
     * 
     * @param plaintextID ID of the plaintext to decode
     * @param output Output buffer for decoded values
     * @param maxSize Maximum number of values to return
     * @return int Number of values decoded (0 if failed)
     */
    int Decode(int plaintextID, double* output, int maxSize);

    /**
     * @brief Create plaintext from values with automatic scaling
     * 
     * @param values Pointer to array of values  
     * @param size Number of values in the array
     * @return int Plaintext ID if successful, -1 if failed
     */
    int CreatePlaintext(double* values, int size);
}