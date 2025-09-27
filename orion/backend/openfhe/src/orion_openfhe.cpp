#include "orion_openfhe.hpp"
#include "scheme.hpp"
#include "encoder.hpp"
#include "encryptor.hpp"
#include "evaluator.hpp"
#include "linear_transform.hpp"
#include "minheap.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

// Global backend instance
static std::unique_ptr<OrionOpenFHEBackend> g_backend;

bool OrionOpenFHEBackend::Initialize(int logN, const std::vector<int>& logQ, const std::vector<int>& logP,
                                    int logScale, int hammingWeight, const std::string& ringType,
                                    const std::string& keysPath, const std::string& ioMode) {
    try {
        // Initialize the scheme
        std::string ringTypeLower = ringType;
        std::transform(ringTypeLower.begin(), ringTypeLower.end(), ringTypeLower.begin(), ::tolower);
        int ringTypeInt = (ringTypeLower == "conjugate_invariant") ? 1 : 0;

        bool schemeSuccess = g_scheme.Initialize(logN, logQ, logP, logScale, hammingWeight,
                                               ringTypeInt, keysPath, ioMode);
        
        if (!schemeSuccess) {
            std::cerr << "Failed to initialize scheme" << std::endl;
            return false;
        }

        // Generate keys
        bool keysSuccess = g_scheme.GenerateKeys();
        if (!keysSuccess) {
            std::cerr << "Failed to generate keys" << std::endl;
            return false;
        }

        // Initialize encoder
        bool encoderSuccess = g_encoder.Initialize();
        if (!encoderSuccess) {
            std::cerr << "Failed to initialize encoder" << std::endl;
            return false;
        }

        // Initialize encryptor
        bool encryptorSuccess = g_encryptor.Initialize();

        if (!encryptorSuccess) {
            std::cerr << "Failed to initialize encryptor" << std::endl;
            return false;
        }

        // Initialize evaluator
        bool evaluatorSuccess = g_evaluator.Initialize();
        if (!evaluatorSuccess) {
            std::cerr << "Failed to initialize evaluator" << std::endl;
            return false;
        }

        // Generate power-of-two rotation keys (commonly used rotations)
        g_scheme.GeneratePowerOfTwoRotationKeys();

        initialized = true;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Backend initialization failed: " << e.what() << std::endl;
        initialized = false;
        return false;
    }
}

int OrionOpenFHEBackend::CreateLinearTransform(const std::vector<double>& matrix) {
    if (!initialized) {
        std::cerr << "Backend not initialized" << std::endl;
        return -1;
    }

    // Assume it's a square matrix for simplicity (can be enhanced later)
    size_t size = matrix.size();
    size_t dim = static_cast<size_t>(std::sqrt(size));

    if (dim * dim != size) {
        std::cerr << "Invalid matrix size: " << size << " (must be square)" << std::endl;
        return -1;
    }

    return ::CreateLinearTransform(matrix, dim, dim);
}

int OrionOpenFHEBackend::ApplyLinearTransform(int ciphertextID, int transformID) {
    if (!initialized) {
        std::cerr << "Backend not initialized" << std::endl;
        return -1;
    }

    return ::ApplyLinearTransform(g_scheme.context, ciphertextID, transformID, g_scheme.rotationKeys);
}

bool OrionOpenFHEBackend::DeleteLinearTransform(int transformID) {
    return ::DeleteLinearTransform(transformID);
}

// Evaluator functions moved to evaluator.cpp

void OrionOpenFHEBackend::CleanUp() {
    if (initialized) {
        // Reset heaps
        ResetTensorHeaps();
        g_ltHeap.Reset();

        g_evaluator.CleanUp();
        g_encryptor.CleanUp();
        g_encoder.CleanUp();
        g_scheme.CleanUp();

        initialized = false;
    }
}

std::string OrionOpenFHEBackend::GetStats() const {
    // Simple memory stats without external dependencies
    return "Backend initialized: " + std::string(initialized ? "true" : "false");
}

// C interface implementations for non-evaluator functions
extern "C" {
    int CreateLinearTransform(double* transform, int size) {
        if (!g_backend || !transform || size <= 0) {
            return -1;
        }

        std::vector<double> transformVec(transform, transform + size);
        return g_backend->CreateLinearTransform(transformVec);
    }


    int ApplyLinearTransform(int ctID, int transformID) {
        if (g_backend) {
            return g_backend->ApplyLinearTransform(ctID, transformID);
        }
        return -1;
    }

}

// Backend management functions (for the updated NewScheme/DeleteScheme)
namespace {
    void InitializeBackend() {
        if (!g_backend) {
            g_backend = std::make_unique<OrionOpenFHEBackend>();
        }
    }

    void CleanupBackend() {
        if (g_backend) {
            g_backend->CleanUp();
            g_backend.reset();
        }
    }
}

// Update the scheme management to use the backend
extern "C" {
    // This will be called from the updated NewScheme function
    void InitializeOrionBackend(int logN, int* logQ, int logQSize, int* logP, int logPSize,
                               int logScale, int hammingWeight, const char* ringType,
                               const char* keysPath, const char* ioMode) {
        try {
            InitializeBackend();

            std::vector<int> logQVec;
            if (logQ && logQSize > 0) {
                logQVec.assign(logQ, logQ + logQSize);
            }

            std::vector<int> logPVec;
            if (logP && logPSize > 0) {
                logPVec.assign(logP, logP + logPSize);
            }

            std::string ringTypeStr = ringType ? std::string(ringType) : "standard";
            std::string keysPathStr = keysPath ? std::string(keysPath) : "";
            std::string ioModeStr = ioMode ? std::string(ioMode) : "memory";

            bool success = g_backend->Initialize(logN, logQVec, logPVec, logScale, 
                                               hammingWeight, ringTypeStr, keysPathStr, ioModeStr);

            if (!success) {
                std::cerr << "Failed to initialize Orion OpenFHE backend" << std::endl;
                CleanupBackend();
            }

        } catch (const std::exception& e) {
            std::cerr << "InitializeOrionBackend exception: " << e.what() << std::endl;
            CleanupBackend();
        }
    }

    void CleanupOrionBackend() {
        CleanupBackend();
    }
}