#include "tensors.hpp"
#include "minheap.hpp"
#include "scheme.hpp"
#include <algorithm>
#include <iostream>

// Heap management functions are now in minheap.cpp
// This file only contains tensor-specific utility functions

// C interface implementations for tensor operations
extern "C" {
    // C interface wrappers for delete functions (void return for C compatibility)
    void DeletePlaintextC(int id) {
        ::DeletePlaintext(id);
    }

    void DeleteCiphertextC(int id) {
        ::DeleteCiphertext(id);
    }

    double GetPlaintextScale(int id) {
        try {
            if (!g_ptHeap.Exists(id)) {
                return 0.0;
            }
            
            auto& plaintext = RetrievePlaintext(id);
            return plaintext->GetScalingFactor();
            
        } catch (const std::exception& e) {
            std::cerr << "GetPlaintextScale error for ID " << id << ": " << e.what() << std::endl;
            return 0.0;
        }
    }

    void SetPlaintextScale(int id, double scale) {
        try {
            if (!g_ptHeap.Exists(id)) {
                std::cerr << "SetPlaintextScale: Plaintext ID " << id << " not found" << std::endl;
                return;
            }
            
            auto& plaintext = RetrievePlaintext(id);
            plaintext->SetScalingFactor(scale);
            
        } catch (const std::exception& e) {
            std::cerr << "SetPlaintextScale error for ID " << id << ": " << e.what() << std::endl;
        }
    }

    double GetCiphertextScale(int id) {
        try {
            if (!g_ctHeap.Exists(id)) {
                return 0.0;
            }
            
            auto& ciphertext = RetrieveCiphertext(id);
            return ciphertext->GetScalingFactor();
            
        } catch (const std::exception& e) {
            std::cerr << "GetCiphertextScale error for ID " << id << ": " << e.what() << std::endl;
            return 0.0;
        }
    }

    void SetCiphertextScale(int id, double scale) {
        try {
            if (!g_ctHeap.Exists(id)) {
                std::cerr << "SetCiphertextScale: Ciphertext ID " << id << " not found" << std::endl;
                return;
            }
            
            auto& ciphertext = RetrieveCiphertext(id);
            ciphertext->SetScalingFactor(scale);
            
        } catch (const std::exception& e) {
            std::cerr << "SetCiphertextScale error for ID " << id << ": " << e.what() << std::endl;
        }
    }

    int GetPlaintextValues(int id, double* output, int maxSize) {
        try {
            if (!output || maxSize <= 0) {
                return 0;
            }

            if (!g_ptHeap.Exists(id)) {
                std::cerr << "GetPlaintextValues: Plaintext ID " << id << " not found" << std::endl;
                return 0;
            }

            auto& plaintext = RetrievePlaintext(id);

            // For CKKS, get the packed values
            std::vector<double> values = plaintext->GetRealPackedValue();

            if (values.empty()) {
                return 0;
            }

            // Copy values to output buffer, respecting the maximum size
            int copySize = std::min(maxSize, static_cast<int>(values.size()));
            std::copy(values.begin(), values.begin() + copySize, output);

            return copySize;

        } catch (const std::exception& e) {
            std::cerr << "GetPlaintextValues error for ID " << id << ": " << e.what() << std::endl;
            return 0;
        }
    }

    // Level Management Functions
    int GetPlaintextLevel(int id) {
        try {
            if (!g_ptHeap.Exists(id)) {
                return -1;
            }

            auto& plaintext = RetrievePlaintext(id);
            return static_cast<int>(plaintext->GetLevel());

        } catch (const std::exception& e) {
            std::cerr << "GetPlaintextLevel error for ID " << id << ": " << e.what() << std::endl;
            return -1;
        }
    }

    int GetCiphertextLevel(int id) {
        try {
            if (!g_ctHeap.Exists(id)) {
                return -1;
            }

            auto& ciphertext = RetrieveCiphertext(id);
            return static_cast<int>(ciphertext->GetLevel());

        } catch (const std::exception& e) {
            std::cerr << "GetCiphertextLevel error for ID " << id << ": " << e.what() << std::endl;
            return -1;
        }
    }

    // Slot Count Functions
    int GetPlaintextSlots(int id) {
        try {
            if (!g_ptHeap.Exists(id)) {
                return -1;
            }

            // For CKKS, slot count is typically half the ring dimension
            // We get it from the scheme context
            if (g_scheme.context) {
                auto ringDim = g_scheme.context->GetRingDimension();
                return static_cast<int>(ringDim / 2);
            }

            return -1;

        } catch (const std::exception& e) {
            std::cerr << "GetPlaintextSlots error for ID " << id << ": " << e.what() << std::endl;
            return -1;
        }
    }

    int GetCiphertextSlots(int id) {
        try {
            if (!g_ctHeap.Exists(id)) {
                return -1;
            }

            // For CKKS, slot count is typically half the ring dimension
            // We get it from the scheme context
            if (g_scheme.context) {
                auto ringDim = g_scheme.context->GetRingDimension();
                return static_cast<int>(ringDim / 2);
            }

            return -1;

        } catch (const std::exception& e) {
            std::cerr << "GetCiphertextSlots error for ID " << id << ": " << e.what() << std::endl;
            return -1;
        }
    }

    int GetCiphertextDegree(int id) {
        try {
            if (!g_ctHeap.Exists(id)) {
                return -1;
            }

            // Use the RetrieveCiphertext function for consistent access pattern
            auto& ciphertext = RetrieveCiphertext(id);

            // OpenFHE: degree = NumberCiphertextElements() - 1
            // This matches Lattigo's ciphertext.Degree() behavior
            return static_cast<int>(ciphertext->NumberCiphertextElements() - 1);
        } catch (const std::exception& e) {
            std::cerr << "GetCiphertextDegree error for ID " << id << ": " << e.what() << std::endl;
            return -1;
        }
    }

    // Memory and System Information Functions
    const char* GetModuliChain() {
        static std::string moduli_info;

        try {
            if (!g_scheme.context) {
                moduli_info = "Error: Scheme context not initialized";
                return moduli_info.c_str();
            }

            moduli_info.clear();
            moduli_info += "Moduli Chain Information:\n";

            // Get Q moduli (coefficient modulus)
            auto paramsQ = g_scheme.context->GetElementParams()->GetParams();
            moduli_info += "Q Moduli (" + std::to_string(paramsQ.size()) + " primes):\n";

            for (size_t i = 0; i < paramsQ.size(); i++) {
                moduli_info += "  q" + std::to_string(i) + ": " +
                              paramsQ[i]->GetModulus().ToString() + "\n";
            }

            // Get overall modulus information
            auto totalModulus = g_scheme.context->GetModulus();
            auto bitLength = totalModulus.GetLengthForBase(2);
            moduli_info += "Total Q modulus bit-length: " + std::to_string(bitLength) + "\n";

            // Try to get P moduli (auxiliary modulus) if available
            try {
                // Cast to CKKS parameters to access P moduli
                const auto cryptoParamsCKKS =
                    std::dynamic_pointer_cast<lbcrypto::CryptoParametersCKKSRNS>(
                        g_scheme.context->GetCryptoParameters());

                if (cryptoParamsCKKS) {
                    auto paramsQP = cryptoParamsCKKS->GetParamsQP();
                    if (paramsQP && paramsQP->GetParams().size() > paramsQ.size()) {
                        moduli_info += "P Moduli (auxiliary primes):\n";

                        for (size_t i = paramsQ.size(); i < paramsQP->GetParams().size(); i++) {
                            moduli_info += "  p" + std::to_string(i - paramsQ.size()) + ": " +
                                          paramsQP->GetParams()[i]->GetModulus().ToString() + "\n";
                        }
                    }
                }
            } catch (const std::exception& e) {
                moduli_info += "Note: P moduli information not available\n";
            }

            return moduli_info.c_str();

        } catch (const std::exception& e) {
            moduli_info = "Error retrieving moduli chain: " + std::string(e.what());
            return moduli_info.c_str();
        }
    }

    int* GetLivePlaintexts(int* count) {
        if (!count) {
            std::cerr << "GetLivePlaintexts error: count parameter is NULL" << std::endl;
            return nullptr;
        }

        try {
            auto liveKeys = g_ptHeap.GetLiveKeys();
            *count = static_cast<int>(liveKeys.size());

            if (liveKeys.empty()) {
                return nullptr;
            }

            // Allocate C array
            int* result = new int[liveKeys.size()];

            // Copy keys to C array
            for (size_t i = 0; i < liveKeys.size(); i++) {
                result[i] = liveKeys[i];
            }

            return result;

        } catch (const std::exception& e) {
            std::cerr << "GetLivePlaintexts error: " << e.what() << std::endl;
            *count = 0;
            return nullptr;
        }
    }

    int* GetLiveCiphertexts(int* count) {
        if (!count) {
            std::cerr << "GetLiveCiphertexts error: count parameter is NULL" << std::endl;
            return nullptr;
        }

        try {
            auto liveKeys = g_ctHeap.GetLiveKeys();
            *count = static_cast<int>(liveKeys.size());

            if (liveKeys.empty()) {
                return nullptr;
            }

            // Allocate C array
            int* result = new int[liveKeys.size()];

            // Copy keys to C array
            for (size_t i = 0; i < liveKeys.size(); i++) {
                result[i] = liveKeys[i];
            }

            return result;

        } catch (const std::exception& e) {
            std::cerr << "GetLiveCiphertexts error: " << e.what() << std::endl;
            *count = 0;
            return nullptr;
        }
    }

    void FreeCIntArray(int* array) {
        delete[] array;
    }
}