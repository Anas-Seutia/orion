#pragma once

/**
 * @file orion_openfhe.hpp
 * @brief Main header for OpenFHE Orion backend
 * 
 * This header includes all components of the OpenFHE backend for Orion,
 * providing a complete interface that matches the Lattigo backend structure.
 * 
 * Components included:
 * - MinHeap: Memory management with heap allocation
 * - Scheme: Cryptographic scheme setup and management
 * - Encoder: Encoding/decoding operations
 * - Encryptor: Encryption/decryption operations
 * - Tensors: Plaintext/ciphertext tensor management
 * - Utils: Utility functions and type conversions
 */

// Include all OpenFHE backend components
#include "minheap.hpp"
#include "scheme.hpp"
#include "encoder.hpp"
#include "encryptor.hpp"
#include "evaluator.hpp"
#include "tensors.hpp"
#include "utils.hpp"
#include "linear_transform.hpp"

// Standard includes
#include <vector>
#include <memory>
#include <string>
#include <map>

/**
 * @brief OpenFHE Backend for Orion - Main Interface
 * 
 * This class provides a high-level interface that wraps all the OpenFHE
 * backend components, providing compatibility with the Orion framework
 * and matching the Lattigo backend API.
 */
class OrionOpenFHEBackend {
private:
    bool initialized;

public:
    /**
     * @brief Construct a new Orion OpenFHE Backend
     */
    OrionOpenFHEBackend() : initialized(false) {}

    /**
     * @brief Initialize the backend with given parameters
     * 
     * @param logN Log of ring dimension
     * @param logQ Vector of coefficient modulus bit-lengths
     * @param logP Vector of auxiliary modulus bit-lengths
     * @param logScale Scaling factor bit-length
     * @param hammingWeight Hamming weight for secret key
     * @param ringType Ring type ("standard" or "conjugate_invariant")
     * @param keysPath Path for key storage
     * @param ioMode I/O mode ("memory" or "file")
     * @return true if initialization successful
     */
    bool Initialize(int logN, const std::vector<int>& logQ, const std::vector<int>& logP,
                   int logScale, int hammingWeight, const std::string& ringType,
                   const std::string& keysPath, const std::string& ioMode);

    /**
     * @brief Check if backend is initialized
     * 
     * @return true if initialized, false otherwise
     */
    bool IsInitialized() const { return initialized; }

    /**
     * @brief Create a linear transformation matrix
     * 
     * @param matrix Matrix data (flattened)
     * @return int Transform ID if successful, -1 if failed
     */
    int CreateLinearTransform(const std::vector<double>& matrix);

    /**
     * @brief Apply linear transformation to ciphertext
     * 
     * @param ciphertextID ID of ciphertext
     * @param transformID ID of transformation
     * @return int Result ciphertext ID if successful, -1 if failed
     */
    int ApplyLinearTransform(int ciphertextID, int transformID);

    /**
     * @brief Delete a linear transformation
     * 
     * @param transformID ID of transformation to delete
     * @return true if successful, false if not found
     */
    bool DeleteLinearTransform(int transformID);

    // Evaluator functions moved to evaluator.hpp/cpp

    /**
     * @brief Clean up all resources
     */
    void CleanUp();

    /**
     * @brief Get memory usage statistics
     * 
     * @return std::string Memory usage information
     */
    std::string GetStats() const;

    /**
     * @brief Destructor - ensures cleanup
     */
    ~OrionOpenFHEBackend() {
        CleanUp();
    }
};

// C interface for non-evaluator operations
extern "C" {
    // Evaluator operations moved to evaluator.hpp

    // Linear transform operations
    int CreateLinearTransform(double* transform, int size);
    int ApplyLinearTransform(int ctID, int transformID);
}