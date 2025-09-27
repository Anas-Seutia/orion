#pragma once

#include "openfhe/pke/openfhe.h"

using namespace lbcrypto;

/**
 * @brief OpenFHE Evaluator for homomorphic operations
 *
 * This class provides a dedicated evaluator for all homomorphic operations
 * on ciphertexts and plaintexts. It matches the Lattigo evaluator interface
 * and centralizes all arithmetic operations.
 */
class OrionEvaluator {
private:
    bool initialized;

public:
    /**
     * @brief Construct a new Orion Evaluator
     */
    OrionEvaluator() : initialized(false) {}

    /**
     * @brief Initialize the evaluator
     *
     * @return true if initialization successful, false otherwise
     */
    bool Initialize();

    /**
     * @brief Check if evaluator is initialized
     *
     * @return true if initialized, false otherwise
     */
    bool IsInitialized() const { return initialized; }

    // Lattigo compatibility (create new copy operations)
    int AddCiphertextNew(int ct1ID, int ct2ID) { return AddCiphertext(ct1ID, ct2ID); }
    int AddPlaintextNew(int ctID, int ptID) { return AddPlaintext(ctID, ptID); }
    int MulPlaintextNew(int ctID, int ptID) { return MulPlaintext(ctID, ptID); }
    int MulRelinCiphertextNew(int ct1ID, int ct2ID) { return MulRelinCiphertext(ct1ID, ct2ID); }
    int SubCiphertextNew(int ct1ID, int ct2ID) { return Subtract(ct1ID, ct2ID); }
    int SubPlaintextNew(int ctID, int ptID) { return SubtractPlain(ctID, ptID); }
    int AddScalarNew(int ctID, double scalar) { return AddScalar(ctID, scalar); }
    int SubScalarNew(int ctID, double scalar) { return SubScalar(ctID, scalar); }
    int MulScalarIntNew(int ctID, int scalar) { return MulScalarInt(ctID, scalar); }
    int MulScalarFloatNew(int ctID, double scalar) { return MulScalarFloat(ctID, scalar); }
    int RotateNew(int ctID, int steps) { return Rotate(ctID, steps); }
    int RescaleNew(int ctID) { return Rescale(ctID); }

    // Basic arithmetic operations

    /**
     * @brief Homomorphic addition of two ciphertexts
     *
     * @param ct1ID First ciphertext ID
     * @param ct2ID Second ciphertext ID
     * @return int Result ciphertext ID if successful, -1 if failed
     */
    int Add(int ct1ID, int ct2ID);

    /**
     * @brief Homomorphic addition of two ciphertexts (alias)
     *
     * @param ct1ID First ciphertext ID
     * @param ct2ID Second ciphertext ID
     * @return int Result ciphertext ID if successful, -1 if failed
     */
    int AddCiphertext(int ct1ID, int ct2ID);

    /**
     * @brief Homomorphic addition of ciphertext and plaintext
     *
     * @param ctID Ciphertext ID
     * @param ptID Plaintext ID
     * @return int Result ciphertext ID if successful, -1 if failed
     */
    int AddPlain(int ctID, int ptID);

    /**
     * @brief Homomorphic addition of ciphertext and plaintext (alias)
     *
     * @param ctID Ciphertext ID
     * @param ptID Plaintext ID
     * @return int Result ciphertext ID if successful, -1 if failed
     */
    int AddPlaintext(int ctID, int ptID);

    /**
     * @brief Homomorphic subtraction of two ciphertexts
     *
     * @param ct1ID First ciphertext ID
     * @param ct2ID Second ciphertext ID
     * @return int Result ciphertext ID if successful, -1 if failed
     */
    int Subtract(int ct1ID, int ct2ID);

    /**
     * @brief Homomorphic subtraction of ciphertext and plaintext
     *
     * @param ctID Ciphertext ID
     * @param ptID Plaintext ID
     * @return int Result ciphertext ID if successful, -1 if failed
     */
    int SubtractPlain(int ctID, int ptID);

    /**
     * @brief Homomorphic multiplication of two ciphertexts
     *
     * @param ct1ID First ciphertext ID
     * @param ct2ID Second ciphertext ID
     * @return int Result ciphertext ID if successful, -1 if failed
     */
    int Multiply(int ct1ID, int ct2ID);

    /**
     * @brief Homomorphic multiplication of two ciphertexts with relinearization (alias)
     *
     * @param ct1ID First ciphertext ID
     * @param ct2ID Second ciphertext ID
     * @return int Result ciphertext ID if successful, -1 if failed
     */
    int MulRelinCiphertext(int ct1ID, int ct2ID);

    /**
     * @brief Homomorphic multiplication of ciphertext and plaintext
     *
     * @param ctID Ciphertext ID
     * @param ptID Plaintext ID
     * @return int Result ciphertext ID if successful, -1 if failed
     */
    int MultiplyPlain(int ctID, int ptID);

    /**
     * @brief Homomorphic multiplication of ciphertext and plaintext (alias)
     *
     * @param ctID Ciphertext ID
     * @param ptID Plaintext ID
     * @return int Result ciphertext ID if successful, -1 if failed
     */
    int MulPlaintext(int ctID, int ptID);

    /**
     * @brief Multiply ciphertext by a scalar value
     *
     * @param ctID Ciphertext ID
     * @param scalar Scalar value to multiply by
     * @return int Result ciphertext ID if successful, -1 if failed
     */
    int MultiplyByScalar(int ctID, double scalar);

    /**
     * @brief Negate a ciphertext
     *
     * @param ctID Ciphertext ID
     * @return int Result ciphertext ID if successful, -1 if failed
     */
    int Negate(int ctID);

    /**
     * @brief Add scalar value to ciphertext
     *
     * @param ctID Ciphertext ID
     * @param scalar Scalar value to add
     * @return int Result ciphertext ID if successful, -1 if failed
     */
    int AddScalar(int ctID, double scalar);

    /**
     * @brief Subtract scalar value from ciphertext
     *
     * @param ctID Ciphertext ID
     * @param scalar Scalar value to subtract
     * @return int Result ciphertext ID if successful, -1 if failed
     */
    int SubScalar(int ctID, double scalar);

    /**
     * @brief Multiply ciphertext by integer scalar
     *
     * @param ctID Ciphertext ID
     * @param scalar Integer scalar value
     * @return int Result ciphertext ID if successful, -1 if failed
     */
    int MulScalarInt(int ctID, int scalar);

    /**
     * @brief Multiply ciphertext by float scalar
     *
     * @param ctID Ciphertext ID
     * @param scalar Float scalar value
     * @return int Result ciphertext ID if successful, -1 if failed
     */
    int MulScalarFloat(int ctID, double scalar);

    // Advanced operations

    /**
     * @brief Rotate ciphertext by specified steps
     *
     * @param ctID Ciphertext ID
     * @param steps Number of rotation steps (positive or negative)
     * @return int Result ciphertext ID if successful, -1 if failed
     */
    int Rotate(int ctID, int steps);

    /**
     * @brief Rescale a ciphertext to reduce noise
     *
     * @param ctID Ciphertext ID
     * @return int Result ciphertext ID if successful, -1 if failed
     */
    int Rescale(int ctID);

    /**
     * @brief Clean up evaluator resources
     */
    void CleanUp();

    /**
     * @brief Destructor - automatically cleans up
     */
    ~OrionEvaluator() { CleanUp(); }
};

// Global evaluator instance declaration
extern OrionEvaluator g_evaluator;

// C interface declarations
extern "C" {
    void NewEvaluator();

    // Basic operations
    int Add(int ct1ID, int ct2ID);
    int AddPlain(int ctID, int ptID);
    int Subtract(int ct1ID, int ct2ID);
    int SubtractPlain(int ctID, int ptID);
    int Multiply(int ct1ID, int ct2ID);
    int MultiplyPlain(int ctID, int ptID);
    int MultiplyByScalar(int ctID, double scalar);
    int Negate(int ctID);

    // Missing basic arithmetic operations from Lattigo backend
    int SubCiphertext(int ct1ID, int ct2ID);
    int SubCiphertextNew(int ct1ID, int ct2ID);
    int SubPlaintext(int ctID, int ptID);
    int SubPlaintextNew(int ctID, int ptID);
    int AddCiphertextNew(int ct1ID, int ct2ID);
    int AddPlaintextNew(int ctID, int ptID);
    int MulPlaintextNew(int ctID, int ptID);
    int MulRelinCiphertextNew(int ct1ID, int ct2ID);
    int AddScalar(int ctID, double scalar);
    int AddScalarNew(int ctID, double scalar);
    int SubScalar(int ctID, double scalar);
    int SubScalarNew(int ctID, double scalar);
    int MulScalarInt(int ctID, int scalar);
    int MulScalarIntNew(int ctID, int scalar);
    int MulScalarFloat(int ctID, double scalar);
    int MulScalarFloatNew(int ctID, double scalar);

    // Advanced operations
    int Rotate(int ctID, int steps);
    int Rescale(int ctID);

    // Lattigo compatibility functions
    int RotateNew(int ctID, int steps);
    int RescaleNew(int ctID);
    int AddScalar(int ctID, double scalar);
    int AddScalarNew(int ctID, double scalar);
    int SubScalar(int ctID, double scalar);
    int SubScalarNew(int ctID, double scalar);
    int MulScalarInt(int ctID, int scalar);
    int MulScalarIntNew(int ctID, int scalar);
    int MulScalarFloat(int ctID, double scalar);
    int MulScalarFloatNew(int ctID, double scalar);
    int SubCiphertext(int ct1ID, int ct2ID);
    int SubCiphertextNew(int ct1ID, int ct2ID);
    int SubPlaintext(int ctID, int ptID);
    int SubPlaintextNew(int ctID, int ptID);
    int AddCiphertextNew(int ct1ID, int ct2ID);
    int AddPlaintextNew(int ctID, int ptID);
    int MulPlaintextNew(int ctID, int ptID);
    int MulRelinCiphertextNew(int ct1ID, int ct2ID);
    int MultiplyByScalar(int ctID, double scalar);

    // Key management
    void AddRotationKey(int step);
}