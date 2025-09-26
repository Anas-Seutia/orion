#include "tensors.hpp"
#include "scheme.hpp"
#include <algorithm>
#include <iostream>

// Global heap allocators
HeapAllocator g_ptHeap;
HeapAllocator g_ctHeap;

int PushPlaintext(const Plaintext& plaintext) {
    return g_ptHeap.Add(plaintext);
}

int PushCiphertext(const Ciphertext<DCRTPoly>& ciphertext) {
    return g_ctHeap.Add(ciphertext);
}

Plaintext& RetrievePlaintext(int plaintextID) {
    return g_ptHeap.Retrieve<Plaintext>(plaintextID);
}

Ciphertext<DCRTPoly>& RetrieveCiphertext(int ciphertextID) {
    return g_ctHeap.Retrieve<Ciphertext<DCRTPoly>>(ciphertextID);
}

std::shared_ptr<Plaintext> GetPlaintextPtr(int plaintextID) {
    return g_ptHeap.GetSharedPtr<Plaintext>(plaintextID);
}

std::shared_ptr<Ciphertext<DCRTPoly>> GetCiphertextPtr(int ciphertextID) {
    return g_ctHeap.GetSharedPtr<Ciphertext<DCRTPoly>>(ciphertextID);
}

bool PlaintextExists(int plaintextID) {
    return g_ptHeap.Exists(plaintextID);
}

bool CiphertextExists(int ciphertextID) {
    return g_ctHeap.Exists(ciphertextID);
}

bool DeletePlaintext(int plaintextID) {
    return g_ptHeap.Delete(plaintextID);
}

bool DeleteCiphertext(int ciphertextID) {
    return g_ctHeap.Delete(ciphertextID);
}

std::vector<int> GetActivePlaintextIDs() {
    return g_ptHeap.GetLiveKeys();
}

std::vector<int> GetActiveCiphertextIDs() {
    return g_ctHeap.GetLiveKeys();
}

void ResetTensorHeaps() {
    g_ptHeap.Reset();
    g_ctHeap.Reset();
}

void GetTensorStats(size_t& plaintextCount, size_t& ciphertextCount) {
    plaintextCount = g_ptHeap.Size();
    ciphertextCount = g_ctHeap.Size();
}

// C interface implementations
extern "C" {
    void DeletePlaintextC(int id) {
        g_ptHeap.Delete(id);
    }

    void DeleteCiphertextC(int id) {
        g_ctHeap.Delete(id);
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
}