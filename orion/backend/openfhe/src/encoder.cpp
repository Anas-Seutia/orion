#include "encoder.hpp"
#include "scheme.hpp"
#include "tensors.hpp"
#include "utils.hpp"
#include <iostream>
#include <algorithm>

// Global encoder instance
OrionEncoder g_encoder;

bool OrionEncoder::Initialize() {
    if (!g_scheme.IsInitialized()) {
        std::cerr << "Cannot initialize encoder: scheme not initialized" << std::endl;
        return false;
    }

    try {
        // OpenFHE handles encoding internally through the CryptoContext
        // No separate encoder object needed - just mark as initialized
        initialized = true;
        return true;

    } catch (const std::exception& e) {
        std::cerr << "Encoder initialization failed: " << e.what() << std::endl;
        initialized = false;
        return false;
    }
}

int OrionEncoder::Encode(const std::vector<double>& values, int level, uint64_t scale) {
    if (!initialized || !g_scheme.IsInitialized()) {
        std::cerr << "Encoder or scheme not initialized" << std::endl;
        return -1;
    }

    try {
        // Create plaintext at the specified level
        Plaintext plaintext = g_scheme.context->MakeCKKSPackedPlaintext(values, 1, level);
        
        // Set the scaling factor
        plaintext->SetScalingFactor(static_cast<double>(scale));

        // Store the plaintext and return its ID
        return PushPlaintext(plaintext);

    } catch (const std::exception& e) {
        std::cerr << "Encode failed: " << e.what() << std::endl;
        return -1;
    }
}

std::vector<double> OrionEncoder::Decode(int plaintextID) {
    if (!initialized || !g_scheme.IsInitialized()) {
        std::cerr << "Encoder or scheme not initialized" << std::endl;
        return std::vector<double>();
    }

    try {
        if (!PlaintextExists(plaintextID)) {
            std::cerr << "Plaintext ID " << plaintextID << " not found" << std::endl;
            return std::vector<double>();
        }

        auto& plaintext = RetrievePlaintext(plaintextID);
        
        // For CKKS, get the real packed values
        std::vector<double> result = plaintext->GetRealPackedValue();
        return result;

    } catch (const std::exception& e) {
        std::cerr << "Decode failed: " << e.what() << std::endl;
        return std::vector<double>();
    }
}

int OrionEncoder::EncodeAtLevel(const std::vector<double>& values, int level) {
    if (!initialized || !g_scheme.IsInitialized()) {
        std::cerr << "Encoder or scheme not initialized" << std::endl;
        return -1;
    }

    try {
        // Use a default scaling factor (2^50 is commonly used in CKKS)
        uint64_t defaultScale = 1ULL << 50;
        return Encode(values, level, defaultScale);

    } catch (const std::exception& e) {
        std::cerr << "EncodeAtLevel failed: " << e.what() << std::endl;
        return -1;
    }
}

uint32_t OrionEncoder::GetSlotCount() const {
    if (!initialized || !g_scheme.IsInitialized()) {
        return 0;
    }

    try {
        return g_scheme.context->GetEncodingParams()->GetBatchSize();
    } catch (const std::exception& e) {
        std::cerr << "GetSlotCount failed: " << e.what() << std::endl;
        return 0;
    }
}

void OrionEncoder::CleanUp() {
    initialized = false;
}

// C interface implementations
extern "C" {
    void NewEncoder() {
        g_encoder.Initialize();
    }

    int Encode(double* values, int lenValues, int level, uint64_t scale) {
        if (!values || lenValues <= 0) {
            std::cerr << "Invalid input to Encode" << std::endl;
            return -1;
        }

        try {
            // Convert C array to C++ vector
            std::vector<double> valueVec(values, values + lenValues);
            return g_encoder.Encode(valueVec, level, scale);

        } catch (const std::exception& e) {
            std::cerr << "Encode C interface failed: " << e.what() << std::endl;
            return -1;
        }
    }

    int Decode(int plaintextID, double* output, int maxSize) {
        if (!output || maxSize <= 0) {
            std::cerr << "Invalid output parameters for Decode" << std::endl;
            return 0;
        }

        try {
            std::vector<double> decoded = g_encoder.Decode(plaintextID);
            
            if (decoded.empty()) {
                return 0;
            }

            // Copy values to output buffer, respecting maximum size
            int copySize = std::min(maxSize, static_cast<int>(decoded.size()));
            std::copy(decoded.begin(), decoded.begin() + copySize, output);

            return copySize;

        } catch (const std::exception& e) {
            std::cerr << "Decode C interface failed: " << e.what() << std::endl;
            return 0;
        }
    }

    int CreatePlaintext(double* values, int size) {
        if (!values || size <= 0) {
            std::cerr << "Invalid input to CreatePlaintext" << std::endl;
            return -1;
        }

        try {
            if (!g_scheme.IsInitialized()) {
                std::cerr << "Scheme not initialized for CreatePlaintext" << std::endl;
                return -1;
            }

            // Convert C array to C++ vector
            std::vector<double> valueVec(values, values + size);
            
            // Create plaintext with default parameters
            Plaintext plaintext = g_scheme.context->MakeCKKSPackedPlaintext(valueVec);
            
            // Store and return ID
            return PushPlaintext(plaintext);

        } catch (const std::exception& e) {
            std::cerr << "CreatePlaintext failed: " << e.what() << std::endl;
            return -1;
        }
    }
}