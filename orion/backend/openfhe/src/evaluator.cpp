#include "evaluator.hpp"
#include "tensors.hpp"
#include "scheme.hpp"
#include <iostream>

// External global scheme reference
extern OrionScheme g_scheme;

// Global evaluator instance
OrionEvaluator g_evaluator;

// OrionEvaluator implementation

bool OrionEvaluator::Initialize() {
    if (!g_scheme.IsInitialized()) {
        std::cerr << "OrionEvaluator: Scheme not initialized" << std::endl;
        return false;
    }

    initialized = true;
    return true;
}

// Ciphertext operations used by Orion
int OrionEvaluator::AddCiphertext(int ct1ID, int ct2ID) {
    if (!initialized) {
        std::cerr << "Evaluator not initialized" << std::endl;
        return -1;
    }

    try {
        if (!CiphertextExists(ct1ID) || !CiphertextExists(ct2ID)) {
            std::cerr << "One or both ciphertext IDs not found" << std::endl;
            return -1;
        }

        auto& ct1 = RetrieveCiphertext(ct1ID);
        auto& ct2 = RetrieveCiphertext(ct2ID);

        auto result = g_scheme.context->EvalAdd(ct1, ct2);
        return PushCiphertext(result);

    } catch (const std::exception& e) {
        std::cerr << "AddCiphertext failed: " << e.what() << std::endl;
        return -1;
    }
}

int OrionEvaluator::AddPlaintext(int ctID, int ptID) {
    if (!initialized) {
        std::cerr << "Evaluator not initialized" << std::endl;
        return -1;
    }

    try {
        if (!CiphertextExists(ctID) || !PlaintextExists(ptID)) {
            std::cerr << "Ciphertext or plaintext ID not found" << std::endl;
            return -1;
        }

        auto& ct = RetrieveCiphertext(ctID);
        auto& pt = RetrievePlaintext(ptID);

        auto result = g_scheme.context->EvalAdd(ct, pt);
        return PushCiphertext(result);

    } catch (const std::exception& e) {
        std::cerr << "AddPlaintext failed: " << e.what() << std::endl;
        return -1;
    }
}

int OrionEvaluator::MulRelinCiphertext(int ct1ID, int ct2ID) {
    if (!initialized) {
        std::cerr << "Evaluator not initialized" << std::endl;
        return -1;
    }

    try {
        if (!CiphertextExists(ct1ID) || !CiphertextExists(ct2ID)) {
            std::cerr << "One or both ciphertext IDs not found" << std::endl;
            return -1;
        }

        auto& ct1 = RetrieveCiphertext(ct1ID);
        auto& ct2 = RetrieveCiphertext(ct2ID);

        auto result = g_scheme.context->EvalMult(ct1, ct2);
        // g_scheme.context->RescaleInPlace(result);

        return PushCiphertext(result);

    } catch (const std::exception& e) {
        std::cerr << "MulRelinCiphertext failed: " << e.what() << std::endl;
        return -1;
    }
}

int OrionEvaluator::MulPlaintext(int ctID, int ptID) {
    if (!initialized) {
        std::cerr << "Evaluator not initialized" << std::endl;
        return -1;
    }

    try {
        if (!CiphertextExists(ctID) || !PlaintextExists(ptID)) {
            std::cerr << "Ciphertext or plaintext ID not found" << std::endl;
            return -1;
        }

        auto& ct = RetrieveCiphertext(ctID);
        auto& pt = RetrievePlaintext(ptID);

        auto result = g_scheme.context->EvalMult(ct, pt);
        // g_scheme.context->RescaleInPlace(result);

        return PushCiphertext(result);

    } catch (const std::exception& e) {
        std::cerr << "MulPlaintext failed: " << e.what() << std::endl;
        return -1;
    }
}

int OrionEvaluator::Negate(int ctID) {
    if (!initialized) {
        std::cerr << "Evaluator not initialized" << std::endl;
        return -1;
    }

    try {
        if (!CiphertextExists(ctID)) {
            std::cerr << "Ciphertext ID not found" << std::endl;
            return -1;
        }

        auto& ct = RetrieveCiphertext(ctID);
        auto result = g_scheme.context->EvalNegate(ct);

        return PushCiphertext(result);

    } catch (const std::exception& e) {
        std::cerr << "Negate failed: " << e.what() << std::endl;
        return -1;
    }
}

int OrionEvaluator::Rotate(int ctID, int steps) {
    if (!initialized) {
        std::cerr << "Evaluator not initialized" << std::endl;
        return -1;
    }

    try {
        if (!CiphertextExists(ctID)) {
            std::cerr << "Ciphertext ID " << ctID << " not found" << std::endl;
            return -1;
        }

        // Ensure rotation key exists
        // g_scheme.GenerateRotationKey(steps);

        auto& ct = RetrieveCiphertext(ctID);
        auto result = g_scheme.context->EvalRotate(ct, steps);

        return PushCiphertext(result);

    } catch (const std::exception& e) {
        std::cerr << "Rotate failed: " << e.what() << std::endl;
        return -1;
    }
}

int OrionEvaluator::Rescale(int ctID) {
    if (!initialized) {
        std::cerr << "Evaluator not initialized" << std::endl;
        return -1;
    }

    try {
        if (!CiphertextExists(ctID)) {
            std::cerr << "Ciphertext ID " << ctID << " not found" << std::endl;
            return -1;
        }

        auto& ct = RetrieveCiphertext(ctID);
        auto result = g_scheme.context->Rescale(ct);

        return PushCiphertext(result);

    } catch (const std::exception& e) {
        std::cerr << "Rescale failed: " << e.what() << std::endl;
        return -1;
    }
}

int OrionEvaluator::MultiplyByScalar(int ctID, double scalar) {
    if (!initialized) {
        std::cerr << "Evaluator not initialized" << std::endl;
        return -1;
    }

    try {
        if (!CiphertextExists(ctID)) {
            std::cerr << "Ciphertext ID " << ctID << " not found" << std::endl;
            return -1;
        }

        auto& ct = RetrieveCiphertext(ctID);
        auto result = g_scheme.context->EvalMult(ct, scalar);

        return PushCiphertext(result);

    } catch (const std::exception& e) {
        std::cerr << "MultiplyByScalar failed: " << e.what() << std::endl;
        return -1;
    }
}

// Additional missing basic arithmetic operations
int OrionEvaluator::Subtract(int ct1ID, int ct2ID) {
    if (!initialized) {
        std::cerr << "Evaluator not initialized" << std::endl;
        return -1;
    }

    try {
        if (!CiphertextExists(ct1ID) || !CiphertextExists(ct2ID)) {
            std::cerr << "One or both ciphertext IDs not found" << std::endl;
            return -1;
        }

        auto& ct1 = RetrieveCiphertext(ct1ID);
        auto& ct2 = RetrieveCiphertext(ct2ID);

        auto result = g_scheme.context->EvalSub(ct1, ct2);
        return PushCiphertext(result);

    } catch (const std::exception& e) {
        std::cerr << "Subtract failed: " << e.what() << std::endl;
        return -1;
    }
}

int OrionEvaluator::SubtractPlain(int ctID, int ptID) {
    if (!initialized) {
        std::cerr << "Evaluator not initialized" << std::endl;
        return -1;
    }

    try {
        if (!CiphertextExists(ctID) || !PlaintextExists(ptID)) {
            std::cerr << "Ciphertext or plaintext ID not found" << std::endl;
            return -1;
        }

        auto& ct = RetrieveCiphertext(ctID);
        auto& pt = RetrievePlaintext(ptID);

        auto result = g_scheme.context->EvalSub(ct, pt);
        return PushCiphertext(result);

    } catch (const std::exception& e) {
        std::cerr << "SubtractPlain failed: " << e.what() << std::endl;
        return -1;
    }
}

int OrionEvaluator::AddScalar(int ctID, double scalar) {
    if (!initialized) {
        std::cerr << "Evaluator not initialized" << std::endl;
        return -1;
    }

    try {
        if (!CiphertextExists(ctID)) {
            std::cerr << "Ciphertext ID " << ctID << " not found" << std::endl;
            return -1;
        }

        auto& ct = RetrieveCiphertext(ctID);
        auto result = g_scheme.context->EvalAdd(ct, scalar);

        return PushCiphertext(result);

    } catch (const std::exception& e) {
        std::cerr << "AddScalar failed: " << e.what() << std::endl;
        return -1;
    }
}

int OrionEvaluator::SubScalar(int ctID, double scalar) {
    if (!initialized) {
        std::cerr << "Evaluator not initialized" << std::endl;
        return -1;
    }

    try {
        if (!CiphertextExists(ctID)) {
            std::cerr << "Ciphertext ID " << ctID << " not found" << std::endl;
            return -1;
        }

        auto& ct = RetrieveCiphertext(ctID);
        auto result = g_scheme.context->EvalSub(ct, scalar);

        return PushCiphertext(result);

    } catch (const std::exception& e) {
        std::cerr << "SubScalar failed: " << e.what() << std::endl;
        return -1;
    }
}

int OrionEvaluator::MulScalarFloat(int ctID, double scalar) {
    return MultiplyByScalar(ctID, scalar);
}

int OrionEvaluator::MulScalarInt(int ctID, int scalar) {
    return MultiplyByScalar(ctID, static_cast<double>(scalar));
}

void OrionEvaluator::CleanUp() {
    initialized = false;
}

// C interface implementations
extern "C" {
    void NewEvaluator() {
        g_evaluator.Initialize();
    }

    // Orion's expected function names
    int AddCiphertext(int ct1ID, int ct2ID) {
        return g_evaluator.AddCiphertext(ct1ID, ct2ID);
    }

    int AddCiphertextNew(int ct1ID, int ct2ID) {
        return g_evaluator.AddCiphertext(ct1ID, ct2ID); // Always creates new in OpenFHE
    }

    int AddPlaintext(int ctID, int ptID) {
        return g_evaluator.AddPlaintext(ctID, ptID);
    }

    int AddPlaintextNew(int ctID, int ptID) {
        return g_evaluator.AddPlaintext(ctID, ptID); // Always creates new in OpenFHE
    }

    int MulRelinCiphertext(int ct1ID, int ct2ID) {
        return g_evaluator.MulRelinCiphertext(ct1ID, ct2ID);
    }

    int MulRelinCiphertextNew(int ct1ID, int ct2ID) {
        return g_evaluator.MulRelinCiphertext(ct1ID, ct2ID); // Always creates new in OpenFHE
    }

    int MulPlaintext(int ctID, int ptID) {
        return g_evaluator.MulPlaintext(ctID, ptID);
    }

    int MulPlaintextNew(int ctID, int ptID) {
        return g_evaluator.MulPlaintext(ctID, ptID); // Always creates new in OpenFHE
    }

    int Negate(int ctID) {
        return g_evaluator.Negate(ctID);
    }

    int Rotate(int ctID, int steps) {
        return g_evaluator.Rotate(ctID, steps);
    }

    int Rescale(int ctID) {
        return g_evaluator.Rescale(ctID);
    }

    // Additional convenience functions matching Lattigo interface
    int RotateNew(int ctID, int steps) {
        return g_evaluator.Rotate(ctID, steps); // Always creates new in OpenFHE
    }

    int RescaleNew(int ctID) {
        return g_evaluator.Rescale(ctID); // Always creates new in OpenFHE
    }

    // Missing basic arithmetic operations
    int SubCiphertext(int ct1ID, int ct2ID) {
        return g_evaluator.Subtract(ct1ID, ct2ID);
    }

    int SubCiphertextNew(int ct1ID, int ct2ID) {
        return g_evaluator.Subtract(ct1ID, ct2ID); // Always creates new in OpenFHE
    }

    int SubPlaintext(int ctID, int ptID) {
        return g_evaluator.SubtractPlain(ctID, ptID);
    }

    int SubPlaintextNew(int ctID, int ptID) {
        return g_evaluator.SubtractPlain(ctID, ptID); // Always creates new in OpenFHE
    }

    int AddScalar(int ctID, double scalar) {
        return g_evaluator.AddScalar(ctID, scalar);
    }

    int AddScalarNew(int ctID, double scalar) {
        return g_evaluator.AddScalar(ctID, scalar);
    }

    int SubScalar(int ctID, double scalar) {
        return g_evaluator.SubScalar(ctID, scalar);
    }

    int SubScalarNew(int ctID, double scalar) {
        return g_evaluator.SubScalar(ctID, scalar);
    }

    int MulScalarInt(int ctID, int scalar) {
        return g_evaluator.MulScalarInt(ctID, scalar);
    }

    int MulScalarIntNew(int ctID, int scalar) {
        return g_evaluator.MulScalarInt(ctID, scalar);
    }

    int MulScalarFloat(int ctID, double scalar) {
        return g_evaluator.MulScalarFloat(ctID, scalar);
    }

    int MulScalarFloatNew(int ctID, double scalar) {
        return g_evaluator.MulScalarFloat(ctID, scalar);
    }

    int Subtract(int ct1ID, int ct2ID) {
        return g_evaluator.Subtract(ct1ID, ct2ID);
    }

    int SubtractPlain(int ctID, int ptID) {
        return g_evaluator.SubtractPlain(ctID, ptID);
    }

    int MultiplyByScalar(int ctID, double scalar) {
        return g_evaluator.MultiplyByScalar(ctID, scalar);
    }

    // Simple aliases for bindings compatibility
    int Add(int ct1ID, int ct2ID) {
        return g_evaluator.AddCiphertext(ct1ID, ct2ID);
    }

    int AddPlain(int ctID, int ptID) {
        return g_evaluator.AddPlaintext(ctID, ptID);
    }

    int Multiply(int ct1ID, int ct2ID) {
        return g_evaluator.MulRelinCiphertext(ct1ID, ct2ID);
    }

    int MultiplyPlain(int ctID, int ptID) {
        return g_evaluator.MulPlaintext(ctID, ptID);
    }

    // AddRotationKey is defined in scheme.cpp, not here
}