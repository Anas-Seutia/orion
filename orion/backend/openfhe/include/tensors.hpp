#pragma once

#include "minheap.hpp"
#include "openfhe/pke/openfhe.h"

using namespace lbcrypto;

/**
 * @brief Tensor management for OpenFHE plaintexts and ciphertexts
 * 
 * This module provides heap-based memory management for OpenFHE plaintext
 * and ciphertext objects, matching the functionality of the Lattigo backend.
 * It uses HeapAllocator instances to efficiently manage object lifetimes.
 */

// Global heap allocators for plaintexts and ciphertexts
extern HeapAllocator g_ptHeap;  // Plaintext heap
extern HeapAllocator g_ctHeap;  // Ciphertext heap

/**
 * @brief Add a plaintext to the heap and return its ID
 * 
 * @param plaintext Plaintext object to store
 * @return int Unique ID for the stored plaintext
 */
int PushPlaintext(const Plaintext& plaintext);

/**
 * @brief Add a ciphertext to the heap and return its ID
 * 
 * @param ciphertext Ciphertext object to store
 * @return int Unique ID for the stored ciphertext
 */
int PushCiphertext(const Ciphertext<DCRTPoly>& ciphertext);

/**
 * @brief Retrieve a plaintext by its ID
 * 
 * @param plaintextID ID of the plaintext to retrieve
 * @return Plaintext& Reference to the stored plaintext
 * @throws std::runtime_error if ID not found
 */
Plaintext& RetrievePlaintext(int plaintextID);

/**
 * @brief Retrieve a ciphertext by its ID
 * 
 * @param ciphertextID ID of the ciphertext to retrieve
 * @return Ciphertext<DCRTPoly>& Reference to the stored ciphertext
 * @throws std::runtime_error if ID not found
 */
Ciphertext<DCRTPoly>& RetrieveCiphertext(int ciphertextID);

/**
 * @brief Get a shared pointer to a plaintext by its ID
 * 
 * @param plaintextID ID of the plaintext
 * @return std::shared_ptr<Plaintext> Shared pointer to the plaintext
 * @throws std::runtime_error if ID not found
 */
std::shared_ptr<Plaintext> GetPlaintextPtr(int plaintextID);

/**
 * @brief Get a shared pointer to a ciphertext by its ID
 * 
 * @param ciphertextID ID of the ciphertext
 * @return std::shared_ptr<Ciphertext<DCRTPoly>> Shared pointer to the ciphertext
 * @throws std::runtime_error if ID not found
 */
std::shared_ptr<Ciphertext<DCRTPoly>> GetCiphertextPtr(int ciphertextID);

/**
 * @brief Check if a plaintext exists with the given ID
 * 
 * @param plaintextID ID to check
 * @return true if plaintext exists, false otherwise
 */
bool PlaintextExists(int plaintextID);

/**
 * @brief Check if a ciphertext exists with the given ID
 * 
 * @param ciphertextID ID to check
 * @return true if ciphertext exists, false otherwise
 */
bool CiphertextExists(int ciphertextID);

/**
 * @brief Delete a plaintext and free its ID
 * 
 * @param plaintextID ID of the plaintext to delete
 * @return true if deletion successful, false if ID not found
 */
bool DeletePlaintext(int plaintextID);

/**
 * @brief Delete a ciphertext and free its ID
 * 
 * @param ciphertextID ID of the ciphertext to delete
 * @return true if deletion successful, false if ID not found
 */
bool DeleteCiphertext(int ciphertextID);

/**
 * @brief Get all active plaintext IDs
 * 
 * @return std::vector<int> Vector of all currently allocated plaintext IDs
 */
std::vector<int> GetActivePlaintextIDs();

/**
 * @brief Get all active ciphertext IDs
 * 
 * @return std::vector<int> Vector of all currently allocated ciphertext IDs
 */
std::vector<int> GetActiveCiphertextIDs();

/**
 * @brief Reset both tensor heaps, clearing all stored objects
 */
void ResetTensorHeaps();

/**
 * @brief Get statistics about tensor memory usage
 * 
 * @param plaintextCount Output parameter for number of plaintexts
 * @param ciphertextCount Output parameter for number of ciphertexts
 */
void GetTensorStats(size_t& plaintextCount, size_t& ciphertextCount);

// C interface functions for tensor management
extern "C" {
    /**
     * @brief Delete a plaintext by ID (C interface)
     *
     * @param id Plaintext ID to delete
     */
    void DeletePlaintextC(int id);

    /**
     * @brief Delete a ciphertext by ID (C interface)
     *
     * @param id Ciphertext ID to delete
     */
    void DeleteCiphertextC(int id);

    /**
     * @brief Get the scaling factor of a plaintext
     * 
     * @param id Plaintext ID
     * @return double Scaling factor (0.0 if ID not found)
     */
    double GetPlaintextScale(int id);

    /**
     * @brief Set the scaling factor of a plaintext
     * 
     * @param id Plaintext ID
     * @param scale New scaling factor
     */
    void SetPlaintextScale(int id, double scale);

    /**
     * @brief Get the scaling factor of a ciphertext
     * 
     * @param id Ciphertext ID
     * @return double Scaling factor (0.0 if ID not found)
     */
    double GetCiphertextScale(int id);

    /**
     * @brief Set the scaling factor of a ciphertext
     * 
     * @param id Ciphertext ID
     * @param scale New scaling factor
     */
    void SetCiphertextScale(int id, double scale);

    /**
     * @brief Get values from a plaintext
     * 
     * @param id Plaintext ID
     * @param output Output buffer for values
     * @param maxSize Maximum number of values to copy
     * @return int Number of values actually copied (0 if error)
     */
    int GetPlaintextValues(int id, double* output, int maxSize);
}