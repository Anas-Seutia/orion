#pragma once

#include "minheap.hpp"
#include "openfhe/pke/openfhe.h"

using namespace lbcrypto;

/**
 * @brief Tensor utilities for OpenFHE plaintexts and ciphertexts
 *
 * This module provides tensor-specific operations like scale management
 * and value extraction. Heap management functions are now in minheap.hpp.
 */

// Heap management functions moved to minheap.hpp

// C interface functions for tensor operations
extern "C" {
    // Delete functions moved to minheap.hpp

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

    // Level Management Functions

    /**
     * @brief Get the level of a plaintext
     *
     * @param id Plaintext ID
     * @return int Level (-1 if ID not found)
     */
    int GetPlaintextLevel(int id);

    /**
     * @brief Get the level of a ciphertext
     *
     * @param id Ciphertext ID
     * @return int Level (-1 if ID not found)
     */
    int GetCiphertextLevel(int id);

    /**
     * @brief Get the number of slots in a plaintext
     *
     * @param id Plaintext ID
     * @return int Number of slots (-1 if ID not found)
     */
    int GetPlaintextSlots(int id);

    /**
     * @brief Get the number of slots in a ciphertext
     *
     * @param id Ciphertext ID
     * @return int Number of slots (-1 if ID not found)
     */
    int GetCiphertextSlots(int id);

    /**
     * @brief Get the degree of a ciphertext
     *
     * @param id Ciphertext ID
     * @return int Degree of the ciphertext (-1 if ID not found)
     */
    int GetCiphertextDegree(int id);

    // Memory and System Information Functions

    /**
     * @brief Get moduli chain information as a string
     *
     * @return const char* Moduli chain information
     */
    const char* GetModuliChain();

    /**
     * @brief Get the IDs of live plaintexts in memory
     *
     * @param count Output parameter for the number of live plaintexts
     * @return int* Array of live plaintext IDs (caller must free)
     */
    int* GetLivePlaintexts(int* count);

    /**
     * @brief Get the IDs of live ciphertexts in memory
     *
     * @param count Output parameter for the number of live ciphertexts
     * @return int* Array of live ciphertext IDs (caller must free)
     */
    int* GetLiveCiphertexts(int* count);

    /**
     * @brief Free an integer array allocated by C functions
     *
     * @param array Array to free
     */
    void FreeCIntArray(int* array);
}