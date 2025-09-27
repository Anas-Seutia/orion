#pragma once

#include "minheap.hpp"
#include "openfhe/pke/openfhe.h"
#include <vector>
#include <memory>

using namespace lbcrypto;

/**
 * @brief Linear transformation support for OpenFHE CKKS
 *
 * This module provides linear transformation capabilities for homomorphically
 * encrypted data, similar to the functionality provided by Lattigo's linear
 * transformation package. It supports matrix-vector multiplication in the
 * encrypted domain.
 */

// Forward declarations
class OrionLinearTransform;

// Global heap allocator for linear transformations
extern HeapAllocator g_ltHeap;

/**
 * @brief Represents a linear transformation matrix for homomorphic operations
 *
 * This class encapsulates the transformation matrix and provides methods
 * for applying linear transformations to encrypted ciphertexts.
 */
class OrionLinearTransform {
private:
    std::vector<std::vector<double>> matrix;
    size_t rows;
    size_t cols;
    bool initialized;

public:
    /**
     * @brief Construct a new linear transformation
     */
    OrionLinearTransform();

    /**
     * @brief Construct a linear transformation from a matrix
     *
     * @param transform_matrix The transformation matrix (row-major order)
     * @param num_rows Number of rows in the matrix
     * @param num_cols Number of columns in the matrix
     */
    OrionLinearTransform(const std::vector<double>& transform_matrix, size_t num_rows, size_t num_cols);

    /**
     * @brief Construct a linear transformation from a 2D matrix
     *
     * @param transform_matrix The 2D transformation matrix
     */
    OrionLinearTransform(const std::vector<std::vector<double>>& transform_matrix);

    /**
     * @brief Destructor
     */
    ~OrionLinearTransform() = default;

    /**
     * @brief Check if the transformation is properly initialized
     *
     * @return true if initialized, false otherwise
     */
    bool IsInitialized() const { return initialized; }

    /**
     * @brief Get the number of rows in the transformation matrix
     *
     * @return size_t Number of rows
     */
    size_t GetRows() const { return rows; }

    /**
     * @brief Get the number of columns in the transformation matrix
     *
     * @return size_t Number of columns
     */
    size_t GetCols() const { return cols; }

    /**
     * @brief Get the transformation matrix
     *
     * @return const std::vector<std::vector<double>>& The matrix
     */
    const std::vector<std::vector<double>>& GetMatrix() const { return matrix; }

    /**
     * @brief Apply the linear transformation to a plaintext vector
     *
     * @param input Input vector
     * @return std::vector<double> Transformed vector
     */
    std::vector<double> Apply(const std::vector<double>& input) const;

    /**
     * @brief Apply the linear transformation to an encrypted ciphertext
     *
     * This method performs the linear transformation in the encrypted domain
     * using CKKS rotation and addition operations.
     *
     * @param context The crypto context
     * @param ciphertext The input ciphertext
     * @param rotationKeys Map of rotation keys for different rotation amounts
     * @return Ciphertext<DCRTPoly> The transformed ciphertext
     */
    Ciphertext<DCRTPoly> Apply(const CryptoContext<DCRTPoly>& context,
                               const Ciphertext<DCRTPoly>& ciphertext,
                               const std::map<usint, EvalKey<DCRTPoly>>& rotationKeys) const;

    /**
     * @brief Validate the transformation matrix dimensions against input size
     *
     * @param input_size Size of the input vector
     * @return true if dimensions are compatible, false otherwise
     */
    bool ValidateDimensions(size_t input_size) const;

    /**
     * @brief Print the transformation matrix for debugging
     */
    void PrintMatrix() const;
};

// Linear transformation management functions

/**
 * @brief Create and store a new linear transformation
 *
 * @param transform_matrix The transformation matrix in row-major order
 * @param num_rows Number of rows in the matrix
 * @param num_cols Number of columns in the matrix
 * @return int Unique ID for the transformation (< 0 on error)
 */
int CreateLinearTransform(const std::vector<double>& transform_matrix, size_t num_rows, size_t num_cols);

/**
 * @brief Create and store a new linear transformation from 2D matrix
 *
 * @param transform_matrix The 2D transformation matrix
 * @return int Unique ID for the transformation (< 0 on error)
 */
int CreateLinearTransform(const std::vector<std::vector<double>>& transform_matrix);

/**
 * @brief Retrieve a linear transformation by its ID
 *
 * @param transform_id ID of the transformation
 * @return OrionLinearTransform* Pointer to the transformation (nullptr if not found)
 */
OrionLinearTransform* GetLinearTransform(int transform_id);

/**
 * @brief Apply a linear transformation to a ciphertext
 *
 * @param context The crypto context
 * @param ciphertext_id ID of the input ciphertext
 * @param transform_id ID of the transformation to apply
 * @param rotationKeys Map of rotation keys
 * @return int ID of the resulting ciphertext (< 0 on error)
 */
int ApplyLinearTransform(const CryptoContext<DCRTPoly>& context,
                         int ciphertext_id,
                         int transform_id,
                         const std::map<usint, EvalKey<DCRTPoly>>& rotationKeys);

/**
 * @brief Delete a linear transformation and free its ID
 *
 * @param transform_id ID of the transformation to delete
 * @return true if deletion successful, false if ID not found
 */
bool DeleteLinearTransform(int transform_id);

/**
 * @brief Check if a linear transformation exists with the given ID
 *
 * @param transform_id ID to check
 * @return true if transformation exists, false otherwise
 */
bool LinearTransformExists(int transform_id);

/**
 * @brief Get all active linear transformation IDs
 *
 * @return std::vector<int> Vector of all currently allocated transformation IDs
 */
std::vector<int> GetActiveLinearTransformIDs();

/**
 * @brief Reset the linear transformation heap, clearing all stored transformations
 */
void ResetLinearTransformHeap();

/**
 * @brief Get statistics about linear transformation memory usage
 *
 * @param transform_count Output parameter for number of transformations
 */
void GetLinearTransformStats(size_t& transform_count);

// C interface functions for linear transformation management
extern "C" {
    /**
     * @brief Initialize linear transform evaluator (C interface) - matches Lattigo interface
     */
    void NewLinearTransformEvaluator();

    /**
     * @brief Generate linear transformation (C interface) - matches Lattigo interface
     *
     * @param diag_idxs Pointer to diagonal indices
     * @param diag_idxs_len Number of diagonal indices
     * @param diag_data Pointer to diagonal data
     * @param diag_data_len Number of diagonal data elements
     * @param level Level for the transformation
     * @param bsgs_ratio BSGS ratio
     * @param io_mode IO mode string
     * @return int Transformation ID (< 0 on error)
     */
    int GenerateLinearTransform(const int* diag_idxs, int diag_idxs_len,
                               const float* diag_data, int diag_data_len,
                               int level, float bsgs_ratio, const char* io_mode);

    /**
     * @brief Apply linear transformation to a ciphertext (C interface) - matches Lattigo interface
     *
     * @param ciphertext_id ID of the input ciphertext
     * @param transform_id ID of the transformation
     * @return int ID of the resulting ciphertext (< 0 on error)
     */
    int EvaluateLinearTransform(int ciphertext_id, int transform_id);

    /**
     * @brief Get required rotation keys for linear transformation (C interface)
     *
     * @param transform_id ID of the transformation
     * @param rotation_keys_out Output buffer for rotation key values
     * @param max_keys Maximum number of keys to return
     * @return int Number of rotation keys required (< 0 on error)
     */
    int GetLinearTransformRotationKeys(int transform_id, int* rotation_keys_out, size_t max_keys);

    // Lattigo-compatible version that returns ArrayResult
    struct ArrayResultInt {
        int* data;
        size_t length;
    };
    ArrayResultInt* GetLinearTransformRotationKeysArray(int transform_id);

    // Lattigo-compatible byte array result
    struct ArrayResultByte {
        char* Data;
        size_t Length;
    };

    /**
     * @brief Generate specific rotation key for linear transformation (C interface)
     *
     * @param rotation_amount Amount of rotation for the key
     */
    void GenerateLinearTransformRotationKey(int rotation_amount);

    /**
     * @brief Generate and serialize rotation key (C interface) - matches Lattigo interface
     *
     * @param rotation_amount Amount of rotation for the key
     * @return Pointer to serialized key data and length (Lattigo ArrayResultByte format)
     */
    ArrayResultByte* GenerateAndSerializeRotationKey(int rotation_amount);

    /**
     * @brief Load rotation key from bytes (C interface)
     *
     * @param serialized_key Pointer to serialized key data
     * @param key_size Size of serialized key data
     * @param rotation_amount Rotation amount for this key
     * @return int 0 on success, < 0 on error
     */
    int LoadRotationKey(const char* serialized_key, size_t key_size, int rotation_amount);

    /**
     * @brief Serialize diagonal for linear transformation (C interface)
     *
     * @param diagonal_data Pointer to diagonal data
     * @param size Size of diagonal data
     * @param serialized_out Output buffer for serialized diagonal
     * @param max_size Maximum size of output buffer
     * @return int Size of serialized diagonal (< 0 on error)
     */
    int SerializeDiagonal(const double* diagonal_data, size_t size, char* serialized_out, size_t max_size);

    /**
     * @brief Load plaintext diagonal from serialized data (C interface)
     *
     * @param serialized_data Pointer to serialized diagonal data
     * @param data_size Size of serialized data
     * @return int Plaintext ID for the loaded diagonal (< 0 on error)
     */
    int LoadPlaintextDiagonal(const char* serialized_data, size_t data_size);

    /**
     * @brief Remove plaintext diagonals from memory (C interface)
     *
     * @param diagonal_ids Array of diagonal IDs to remove
     * @param count Number of diagonal IDs
     */
    void RemovePlaintextDiagonals(const int* diagonal_ids, size_t count);

    /**
     * @brief Remove rotation keys from memory (C interface)
     *
     * @param rotation_amounts Array of rotation amounts whose keys to remove
     * @param count Number of rotation amounts
     */
    void RemoveRotationKeys(const int* rotation_amounts, size_t count);

    /**
     * @brief Create a linear transformation (C interface)
     *
     * @param matrix_data Pointer to transformation matrix data (row-major order)
     * @param num_rows Number of rows in the matrix
     * @param num_cols Number of columns in the matrix
     * @return int Transformation ID (< 0 on error)
     */
    int CreateLinearTransformC(const double* matrix_data, size_t num_rows, size_t num_cols);

    /**
     * @brief Apply linear transformation to a ciphertext (C interface)
     *
     * @param ciphertext_id ID of the input ciphertext
     * @param transform_id ID of the transformation
     * @return int ID of the resulting ciphertext (< 0 on error)
     */
    int ApplyLinearTransformC(int ciphertext_id, int transform_id);

    /**
     * @brief Delete a linear transformation (C interface)
     *
     * @param transform_id ID of the transformation to delete
     */
    void DeleteLinearTransformC(int transform_id);

    /**
     * @brief Check if a linear transformation exists (C interface)
     *
     * @param transform_id ID to check
     * @return int 1 if exists, 0 if not
     */
    int LinearTransformExistsC(int transform_id);

    /**
     * @brief Get the number of active linear transformations (C interface)
     *
     * @return int Number of active transformations
     */
    int GetLinearTransformCountC();
}