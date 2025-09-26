#include "linear_transform.hpp"
#include "tensors.hpp"
#include "scheme.hpp"
#include <iostream>
#include <algorithm>
#include <iomanip>

// Global heap allocator for linear transformations
HeapAllocator g_ltHeap;

// External reference to global scheme
extern OrionScheme g_scheme;

// OrionLinearTransform implementation

OrionLinearTransform::OrionLinearTransform()
    : rows(0), cols(0), initialized(false) {}

OrionLinearTransform::OrionLinearTransform(const std::vector<double>& transform_matrix,
                                           size_t num_rows, size_t num_cols)
    : rows(num_rows), cols(num_cols), initialized(false) {

    if (transform_matrix.size() != num_rows * num_cols) {
        std::cerr << "OrionLinearTransform: Matrix size mismatch. Expected "
                  << num_rows * num_cols << " elements, got " << transform_matrix.size() << std::endl;
        return;
    }

    // Convert from row-major order to 2D matrix
    matrix.resize(rows);
    for (size_t i = 0; i < rows; i++) {
        matrix[i].resize(cols);
        for (size_t j = 0; j < cols; j++) {
            matrix[i][j] = transform_matrix[i * cols + j];
        }
    }

    initialized = true;
}

OrionLinearTransform::OrionLinearTransform(const std::vector<std::vector<double>>& transform_matrix)
    : matrix(transform_matrix), initialized(false) {

    if (transform_matrix.empty()) {
        rows = cols = 0;
        return;
    }

    rows = transform_matrix.size();
    cols = transform_matrix[0].size();

    // Validate that all rows have the same number of columns
    for (size_t i = 0; i < rows; i++) {
        if (transform_matrix[i].size() != cols) {
            std::cerr << "OrionLinearTransform: Inconsistent matrix dimensions. "
                      << "Row " << i << " has " << transform_matrix[i].size()
                      << " columns, expected " << cols << std::endl;
            rows = cols = 0;
            matrix.clear();
            return;
        }
    }

    initialized = true;
}

std::vector<double> OrionLinearTransform::Apply(const std::vector<double>& input) const {
    if (!initialized) {
        std::cerr << "OrionLinearTransform: Transformation not initialized" << std::endl;
        return std::vector<double>();
    }

    if (input.size() != cols) {
        std::cerr << "OrionLinearTransform: Input size " << input.size()
                  << " doesn't match matrix columns " << cols << std::endl;
        return std::vector<double>();
    }

    std::vector<double> result(rows, 0.0);

    // Matrix-vector multiplication: result = matrix * input
    for (size_t i = 0; i < rows; i++) {
        for (size_t j = 0; j < cols; j++) {
            result[i] += matrix[i][j] * input[j];
        }
    }

    return result;
}

Ciphertext<DCRTPoly> OrionLinearTransform::Apply(const CryptoContext<DCRTPoly>& context,
                                                 const Ciphertext<DCRTPoly>& ciphertext,
                                                 const std::map<usint, EvalKey<DCRTPoly>>& rotationKeys) const {
    if (!initialized) {
        throw std::runtime_error("OrionLinearTransform: Transformation not initialized");
    }

    // For CKKS linear transformations, we use the baby-step giant-step algorithm
    // This is a simplified implementation for demonstration

    try {
        // Create a zero ciphertext for accumulation
        auto result = context->EvalMult(ciphertext, 0.0);

        // Apply each row of the transformation matrix
        for (size_t i = 0; i < rows; i++) {
            auto row_result = context->EvalMult(ciphertext, 0.0); // Start with zero

            for (size_t j = 0; j < cols; j++) {
                if (std::abs(matrix[i][j]) > 1e-10) { // Skip near-zero coefficients
                    auto rotated_ct = ciphertext;

                    // Apply rotation if needed (j != 0)
                    if (j > 0) {
                        rotated_ct = context->EvalRotate(ciphertext, static_cast<int32_t>(j));
                    }

                    // Multiply by the matrix coefficient
                    auto weighted_ct = context->EvalMult(rotated_ct, matrix[i][j]);

                    // Add to the row result
                    row_result = context->EvalAdd(row_result, weighted_ct);
                }
            }

            // For this simplified version, we only return the first row
            // A complete implementation would pack multiple rows into slots
            if (i == 0) {
                result = row_result;
                break; // For now, only handle single row transformations
            }
        }

        return result;

    } catch (const std::exception& e) {
        throw std::runtime_error("Linear transformation failed: " + std::string(e.what()));
    }
}

bool OrionLinearTransform::ValidateDimensions(size_t input_size) const {
    return initialized && (input_size == cols);
}

void OrionLinearTransform::PrintMatrix() const {
    if (!initialized) {
        std::cout << "Transformation not initialized" << std::endl;
        return;
    }

    std::cout << "Linear Transformation Matrix (" << rows << "x" << cols << "):" << std::endl;
    for (size_t i = 0; i < rows; i++) {
        std::cout << "  [";
        for (size_t j = 0; j < cols; j++) {
            std::cout << std::fixed << std::setprecision(3) << std::setw(8) << matrix[i][j];
            if (j < cols - 1) std::cout << ", ";
        }
        std::cout << "]" << std::endl;
    }
}

// Management functions implementation

int CreateLinearTransform(const std::vector<double>& transform_matrix, size_t num_rows, size_t num_cols) {
    try {
        auto transform = std::make_shared<OrionLinearTransform>(transform_matrix, num_rows, num_cols);

        if (!transform->IsInitialized()) {
            std::cerr << "Failed to initialize linear transformation" << std::endl;
            return -1;
        }

        return g_ltHeap.Add(transform);

    } catch (const std::exception& e) {
        std::cerr << "CreateLinearTransform error: " << e.what() << std::endl;
        return -1;
    }
}

int CreateLinearTransform(const std::vector<std::vector<double>>& transform_matrix) {
    try {
        auto transform = std::make_shared<OrionLinearTransform>(transform_matrix);

        if (!transform->IsInitialized()) {
            std::cerr << "Failed to initialize linear transformation" << std::endl;
            return -1;
        }

        return g_ltHeap.Add(transform);

    } catch (const std::exception& e) {
        std::cerr << "CreateLinearTransform error: " << e.what() << std::endl;
        return -1;
    }
}

OrionLinearTransform* GetLinearTransform(int transform_id) {
    try {
        if (!g_ltHeap.Exists(transform_id)) {
            return nullptr;
        }

        auto transform_ptr = g_ltHeap.GetSharedPtr<OrionLinearTransform>(transform_id);
        return transform_ptr.get();

    } catch (const std::exception& e) {
        std::cerr << "GetLinearTransform error for ID " << transform_id << ": " << e.what() << std::endl;
        return nullptr;
    }
}

int ApplyLinearTransform(const CryptoContext<DCRTPoly>& context,
                         int ciphertext_id,
                         int transform_id,
                         const std::map<usint, EvalKey<DCRTPoly>>& rotationKeys) {
    try {
        // Get the transformation
        auto* transform = GetLinearTransform(transform_id);
        if (!transform) {
            std::cerr << "Linear transformation " << transform_id << " not found" << std::endl;
            return -1;
        }

        // Get the ciphertext
        if (!CiphertextExists(ciphertext_id)) {
            std::cerr << "Ciphertext " << ciphertext_id << " not found" << std::endl;
            return -1;
        }

        auto& ciphertext = RetrieveCiphertext(ciphertext_id);

        // Apply the transformation
        auto result_ct = transform->Apply(context, ciphertext, rotationKeys);

        // Store the result
        return PushCiphertext(result_ct);

    } catch (const std::exception& e) {
        std::cerr << "ApplyLinearTransform error: " << e.what() << std::endl;
        return -1;
    }
}

bool DeleteLinearTransform(int transform_id) {
    return g_ltHeap.Delete(transform_id);
}

bool LinearTransformExists(int transform_id) {
    return g_ltHeap.Exists(transform_id);
}

std::vector<int> GetActiveLinearTransformIDs() {
    return g_ltHeap.GetLiveKeys();
}

void ResetLinearTransformHeap() {
    g_ltHeap.Reset();
}

void GetLinearTransformStats(size_t& transform_count) {
    transform_count = g_ltHeap.Size();
}

// C interface implementations

extern "C" {
    int CreateLinearTransformC(const double* matrix_data, size_t num_rows, size_t num_cols) {
        if (!matrix_data || num_rows == 0 || num_cols == 0) {
            return -1;
        }

        try {
            std::vector<double> matrix_vec(matrix_data, matrix_data + (num_rows * num_cols));
            return CreateLinearTransform(matrix_vec, num_rows, num_cols);
        } catch (const std::exception& e) {
            std::cerr << "CreateLinearTransformC error: " << e.what() << std::endl;
            return -1;
        }
    }

    int ApplyLinearTransformC(int ciphertext_id, int transform_id) {
        if (!g_scheme.IsInitialized()) {
            std::cerr << "Scheme not initialized" << std::endl;
            return -1;
        }

        try {
            return ApplyLinearTransform(g_scheme.context, ciphertext_id, transform_id, g_scheme.rotationKeys);
        } catch (const std::exception& e) {
            std::cerr << "ApplyLinearTransformC error: " << e.what() << std::endl;
            return -1;
        }
    }

    void DeleteLinearTransformC(int transform_id) {
        DeleteLinearTransform(transform_id);
    }

    int LinearTransformExistsC(int transform_id) {
        return LinearTransformExists(transform_id) ? 1 : 0;
    }

    int GetLinearTransformCountC() {
        size_t count;
        GetLinearTransformStats(count);
        return static_cast<int>(count);
    }
}