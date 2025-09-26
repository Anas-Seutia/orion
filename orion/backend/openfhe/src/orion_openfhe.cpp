#include "orion_openfhe.hpp"
#include <iostream>
#include <algorithm>

// Global backend instance
static std::unique_ptr<OrionOpenFHEBackend> g_backend;

bool OrionOpenFHEBackend::Initialize(int logN, const std::vector<int>& logQ, const std::vector<int>& logP,
                                    int logScale, int hammingWeight, const std::string& ringType,
                                    const std::string& keysPath, const std::string& ioMode) {
    try {
        // Initialize the scheme
        int ringTypeInt = (StringUtils::ToLowerCase(ringType) == "conjugate_invariant") ? 1 : 0;
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

        // Initialize encryptor and decryptor
        bool encryptorSuccess = g_encryptor.Initialize();
        bool decryptorSuccess = g_decryptor.Initialize();
        
        if (!encryptorSuccess || !decryptorSuccess) {
            std::cerr << "Failed to initialize encryptor/decryptor" << std::endl;
            return false;
        }

        // Generate power-of-two rotation keys
        g_scheme.GeneratePowerOfTwoRotationKeys();

        initialized = true;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Backend initialization failed: " << e.what() << std::endl;
        initialized = false;
        return false;
    }
}

int OrionOpenFHEBackend::EncodeAndEncrypt(const std::vector<double>& values) {
    if (!initialized) {
        std::cerr << "Backend not initialized" << std::endl;
        return -1;
    }

    try {
        return g_encryptor.EncryptValues(values);
    } catch (const std::exception& e) {
        std::cerr << "EncodeAndEncrypt failed: " << e.what() << std::endl;
        return -1;
    }
}

std::vector<double> OrionOpenFHEBackend::DecryptAndDecode(int ciphertextID) {
    if (!initialized) {
        std::cerr << "Backend not initialized" << std::endl;
        return std::vector<double>();
    }

    try {
        return g_encryptor.DecryptValues(ciphertextID);
    } catch (const std::exception& e) {
        std::cerr << "DecryptAndDecode failed: " << e.what() << std::endl;
        return std::vector<double>();
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

int OrionOpenFHEBackend::Add(int ct1ID, int ct2ID) {
    if (!initialized) return -1;

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
        std::cerr << "Add failed: " << e.what() << std::endl;
        return -1;
    }
}

int OrionOpenFHEBackend::AddPlain(int ctID, int ptID) {
    if (!initialized) return -1;

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
        std::cerr << "AddPlain failed: " << e.what() << std::endl;
        return -1;
    }
}

int OrionOpenFHEBackend::Multiply(int ct1ID, int ct2ID) {
    if (!initialized) return -1;

    try {
        if (!CiphertextExists(ct1ID) || !CiphertextExists(ct2ID)) {
            std::cerr << "One or both ciphertext IDs not found" << std::endl;
            return -1;
        }

        auto& ct1 = RetrieveCiphertext(ct1ID);
        auto& ct2 = RetrieveCiphertext(ct2ID);

        auto result = g_scheme.context->EvalMult(ct1, ct2);
        g_scheme.context->RescaleInPlace(result);
        
        return PushCiphertext(result);

    } catch (const std::exception& e) {
        std::cerr << "Multiply failed: " << e.what() << std::endl;
        return -1;
    }
}

int OrionOpenFHEBackend::MultiplyPlain(int ctID, int ptID) {
    if (!initialized) return -1;

    try {
        if (!CiphertextExists(ctID) || !PlaintextExists(ptID)) {
            std::cerr << "Ciphertext or plaintext ID not found" << std::endl;
            return -1;
        }

        auto& ct = RetrieveCiphertext(ctID);
        auto& pt = RetrievePlaintext(ptID);

        auto result = g_scheme.context->EvalMult(ct, pt);
        g_scheme.context->RescaleInPlace(result);
        
        return PushCiphertext(result);

    } catch (const std::exception& e) {
        std::cerr << "MultiplyPlain failed: " << e.what() << std::endl;
        return -1;
    }
}

int OrionOpenFHEBackend::Rotate(int ctID, int steps) {
    if (!initialized) return -1;

    try {
        if (!CiphertextExists(ctID)) {
            std::cerr << "Ciphertext ID " << ctID << " not found" << std::endl;
            return -1;
        }

        // Ensure rotation key exists
        g_scheme.GenerateRotationKey(steps);

        auto& ct = RetrieveCiphertext(ctID);
        auto result = g_scheme.context->EvalRotate(ct, steps);
        
        return PushCiphertext(result);

    } catch (const std::exception& e) {
        std::cerr << "Rotate failed: " << e.what() << std::endl;
        return -1;
    }
}

int OrionOpenFHEBackend::Rescale(int ctID) {
    if (!initialized) return -1;

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

void OrionOpenFHEBackend::CleanUp() {
    if (initialized) {
        // Reset linear transformation heap
        ResetLinearTransformHeap();
        
        g_encryptor.CleanUp();
        g_decryptor.CleanUp();
        g_encoder.CleanUp();
        g_scheme.CleanUp();
        
        initialized = false;
    }
}

std::string OrionOpenFHEBackend::GetStats() const {
    return DebugUtils::GetMemoryStats();
}

// C interface implementations
extern "C" {
    int Add(int ct1ID, int ct2ID) {
        if (g_backend) {
            return g_backend->Add(ct1ID, ct2ID);
        }
        return -1;
    }

    int AddPlain(int ctID, int ptID) {
        if (g_backend) {
            return g_backend->AddPlain(ctID, ptID);
        }
        return -1;
    }

    int Multiply(int ct1ID, int ct2ID) {
        if (g_backend) {
            return g_backend->Multiply(ct1ID, ct2ID);
        }
        return -1;
    }

    int MultiplyPlain(int ctID, int ptID) {
        if (g_backend) {
            return g_backend->MultiplyPlain(ctID, ptID);
        }
        return -1;
    }

    int Rotate(int ctID, int steps) {
        if (g_backend) {
            return g_backend->Rotate(ctID, steps);
        }
        return -1;
    }

    int Rescale(int ctID) {
        if (g_backend) {
            return g_backend->Rescale(ctID);
        }
        return -1;
    }

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

            std::string ringTypeStr = StringUtils::SafeCStringToString(ringType, "standard");
            std::string keysPathStr = StringUtils::SafeCStringToString(keysPath, "");
            std::string ioModeStr = StringUtils::SafeCStringToString(ioMode, "memory");

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