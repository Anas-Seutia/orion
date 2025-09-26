#include "scheme.hpp"
#include "tensors.hpp"
#include "utils.hpp"
#include <iostream>
#include <stdexcept>

// Global scheme instance
OrionScheme g_scheme;

bool OrionScheme::Initialize(int logN, const std::vector<int>& logQ, const std::vector<int>& logP,
                            int logScale, int hammingWeight, int ringType,
                            const std::string& keysPath, const std::string& ioMode) {
    try {
        // Create CKKS parameters
        CCParams<CryptoContextCKKSRNS> parameters;

        // Set ring dimension
        uint32_t ringDim = 1 << logN;
        parameters.SetRingDim(ringDim);

        // Set multiplicative depth based on logQ size
        uint32_t multDepth = logQ.size() > 1 ? logQ.size() - 1 : 1;
        parameters.SetMultiplicativeDepth(multDepth);

        // Set scaling modulus bit-length
        uint32_t scaleModSize = logScale;
        parameters.SetScalingModSize(scaleModSize);

        // Set security level
        parameters.SetSecurityLevel(HEStd_128_classic);

        // Set secret key distribution based on Hamming weight
        if (hammingWeight > 0) {
            parameters.SetSecretKeyDist(UNIFORM_TERNARY);
        } else {
            parameters.SetSecretKeyDist(GAUSSIAN);
        }

        // Set first modulus size (usually larger for better precision)
        if (!logQ.empty()) {
            parameters.SetFirstModSize(logQ[0]);
        }

        // Create custom modulus chain if specified
        if (logQ.size() > 1) {
            std::vector<uint32_t> levelBudget;
            // Skip first element as it's used for first mod size
            for (size_t i = 1; i < logQ.size(); ++i) {
                levelBudget.push_back(logQ[i]);
            }
            if (!levelBudget.empty()) {
                parameters.SetScalingTechnique(FLEXIBLEAUTO);
            }
        }

        // Generate crypto context
        context = GenCryptoContext(parameters);

        // Enable required features
        context->Enable(PKE);
        context->Enable(KEYSWITCH);
        context->Enable(LEVELEDSHE);
        context->Enable(ADVANCEDSHE);

        initialized = true;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Scheme initialization failed: " << e.what() << std::endl;
        initialized = false;
        return false;
    }
}

bool OrionScheme::GenerateKeys() {
    if (!initialized) {
        std::cerr << "Scheme not initialized" << std::endl;
        return false;
    }

    try {
        // Generate key pair
        keyPair = context->KeyGen();
        
        // Extract individual keys for convenience
        publicKey = keyPair.publicKey;
        secretKey = keyPair.secretKey;

        // Generate relinearization key
        context->EvalMultKeyGen(secretKey);
        
        // Generate some basic rotation keys (can be extended as needed)
        std::vector<int32_t> indexList = {1, -1};
        context->EvalRotateKeyGen(secretKey, indexList);

        return true;

    } catch (const std::exception& e) {
        std::cerr << "Key generation failed: " << e.what() << std::endl;
        return false;
    }
}

bool OrionScheme::GenerateRotationKey(int step) {
    if (!initialized) {
        std::cerr << "Scheme not initialized" << std::endl;
        return false;
    }

    try {
        // Check if key already exists
        uint32_t autoIndex = static_cast<uint32_t>(step);
        if (rotationKeys.find(autoIndex) != rotationKeys.end()) {
            return true; // Key already exists
        }

        // Generate the rotation key
        std::vector<int32_t> indexList = {step};
        context->EvalRotateKeyGen(secretKey, indexList);

        return true;

    } catch (const std::exception& e) {
        std::cerr << "Rotation key generation failed for step " << step << ": " << e.what() << std::endl;
        return false;
    }
}

void OrionScheme::GeneratePowerOfTwoRotationKeys() {
    if (!initialized) {
        std::cerr << "Scheme not initialized" << std::endl;
        return;
    }

    try {
        uint32_t maxSlots = GetMaxSlots();
        std::vector<int32_t> indexList;

        // Generate all positive power-of-two rotation keys
        for (uint32_t i = 1; i < maxSlots; i *= 2) {
            indexList.push_back(static_cast<int32_t>(i));
            indexList.push_back(-static_cast<int32_t>(i)); // Also negative rotations
        }

        if (!indexList.empty()) {
            context->EvalRotateKeyGen(secretKey, indexList);
        }

    } catch (const std::exception& e) {
        std::cerr << "Power-of-two rotation key generation failed: " << e.what() << std::endl;
    }
}

void OrionScheme::CleanUp() {
    if (initialized) {
        // Clear all components
        context = nullptr;
        keyPair.publicKey = nullptr;
        keyPair.secretKey = nullptr;
        publicKey = nullptr;
        secretKey = nullptr;
        relinKey = nullptr;
        rotationKeys.clear();
        
        // Reset tensor heaps
        ResetTensorHeaps();
        
        initialized = false;
    }
}

// C interface implementations
extern "C" {
    void NewScheme(int logN, int* logQ, int logQSize, int* logP, int logPSize,
                   int logScale, int hammingWeight, const char* ringType,
                   const char* keysPath, const char* ioMode) {
        try {
            // Convert C arrays to C++ vectors
            std::vector<int> logQVec;
            if (logQ && logQSize > 0) {
                logQVec.assign(logQ, logQ + logQSize);
            }

            std::vector<int> logPVec;
            if (logP && logPSize > 0) {
                logPVec.assign(logP, logP + logPSize);
            }

            // Convert C strings to C++ strings
            std::string ringTypeStr = ringType ? std::string(ringType) : "standard";
            std::string keysPathStr = keysPath ? std::string(keysPath) : "";
            std::string ioModeStr = ioMode ? std::string(ioMode) : "memory";

            // Determine ring type (0 = standard, 1 = conjugate invariant)
            int ringTypeInt = (ringTypeStr == "conjugate_invariant") ? 1 : 0;

            // Initialize the scheme
            bool success = g_scheme.Initialize(logN, logQVec, logPVec, logScale, 
                                             hammingWeight, ringTypeInt, 
                                             keysPathStr, ioModeStr);

            if (success) {
                // Generate keys after successful initialization
                g_scheme.GenerateKeys();
                g_scheme.GeneratePowerOfTwoRotationKeys();
            } else {
                std::cerr << "Failed to initialize OpenFHE scheme" << std::endl;
            }

        } catch (const std::exception& e) {
            std::cerr << "NewScheme exception: " << e.what() << std::endl;
        }
    }

    void DeleteScheme() {
        g_scheme.CleanUp();
    }

    int IsSchemeInitialized() {
        return g_scheme.IsInitialized() ? 1 : 0;
    }

    void AddRotationKey(int step) {
        if (g_scheme.IsInitialized()) {
            g_scheme.GenerateRotationKey(step);
        }
    }
}