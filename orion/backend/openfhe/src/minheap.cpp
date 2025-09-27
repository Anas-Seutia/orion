#include "minheap.hpp"
#include <iostream>
#include <algorithm>

// HeapAllocator is not a template class, so no explicit instantiation needed

// Global heap allocators for different object types
HeapAllocator g_ptHeap;   // Plaintext heap
HeapAllocator g_ctHeap;   // Ciphertext heap
HeapAllocator g_ltHeap;   // Linear transform heap

// Utility functions for debugging and statistics
void PrintHeapStats() {
    std::cout << "=== Heap Allocator Statistics ===" << std::endl;
    std::cout << "Plaintexts: " << g_ptHeap.Size() << " allocated" << std::endl;
    std::cout << "Ciphertexts: " << g_ctHeap.Size() << " allocated" << std::endl;
    std::cout << "Linear Transforms: " << g_ltHeap.Size() << " allocated" << std::endl;
    std::cout << "=================================" << std::endl;
}

void ResetAllHeaps() {
    g_ptHeap.Reset();
    g_ctHeap.Reset();
    g_ltHeap.Reset();
    std::cout << "All heaps reset successfully" << std::endl;
}

size_t GetTotalAllocatedObjects() {
    return g_ptHeap.Size() + g_ctHeap.Size() + g_ltHeap.Size();
}

std::vector<int> GetAllPlaintextIDs() {
    return g_ptHeap.GetLiveKeys();
}

std::vector<int> GetAllCiphertextIDs() {
    return g_ctHeap.GetLiveKeys();
}

std::vector<int> GetAllLinearTransformIDs() {
    return g_ltHeap.GetLiveKeys();
}

// Memory management utilities
void CleanupExpiredObjects() {
    // This function could be extended to clean up objects based on
    // reference counting or other criteria in the future

    // For now, we just provide debugging information
    auto pt_ids = GetAllPlaintextIDs();
    auto ct_ids = GetAllCiphertextIDs();
    auto lt_ids = GetAllLinearTransformIDs();

    std::cout << "Active object IDs:" << std::endl;

    if (!pt_ids.empty()) {
        std::cout << "  Plaintexts: ";
        for (size_t i = 0; i < std::min(pt_ids.size(), size_t(10)); ++i) {
            std::cout << pt_ids[i] << " ";
        }
        if (pt_ids.size() > 10) std::cout << "... (" << pt_ids.size() << " total)";
        std::cout << std::endl;
    }

    if (!ct_ids.empty()) {
        std::cout << "  Ciphertexts: ";
        for (size_t i = 0; i < std::min(ct_ids.size(), size_t(10)); ++i) {
            std::cout << ct_ids[i] << " ";
        }
        if (ct_ids.size() > 10) std::cout << "... (" << ct_ids.size() << " total)";
        std::cout << std::endl;
    }

    if (!lt_ids.empty()) {
        std::cout << "  Linear Transforms: ";
        for (size_t i = 0; i < std::min(lt_ids.size(), size_t(10)); ++i) {
            std::cout << lt_ids[i] << " ";
        }
        if (lt_ids.size() > 10) std::cout << "... (" << lt_ids.size() << " total)";
        std::cout << std::endl;
    }
}

// Memory monitoring utilities
class MemoryMonitor {
private:
    static size_t peak_plaintexts;
    static size_t peak_ciphertexts;
    static size_t peak_linear_transforms;

public:
    static void UpdatePeaks() {
        peak_plaintexts = std::max(peak_plaintexts, g_ptHeap.Size());
        peak_ciphertexts = std::max(peak_ciphertexts, g_ctHeap.Size());
        peak_linear_transforms = std::max(peak_linear_transforms, g_ltHeap.Size());
    }

    static void PrintPeakUsage() {
        std::cout << "=== Peak Memory Usage ===" << std::endl;
        std::cout << "Peak Plaintexts: " << peak_plaintexts << std::endl;
        std::cout << "Peak Ciphertexts: " << peak_ciphertexts << std::endl;
        std::cout << "Peak Linear Transforms: " << peak_linear_transforms << std::endl;
        std::cout << "=========================" << std::endl;
    }

    static void ResetPeaks() {
        peak_plaintexts = 0;
        peak_ciphertexts = 0;
        peak_linear_transforms = 0;
    }
};

// Static member definitions
size_t MemoryMonitor::peak_plaintexts = 0;
size_t MemoryMonitor::peak_ciphertexts = 0;
size_t MemoryMonitor::peak_linear_transforms = 0;

// Public interface for memory monitoring
void UpdateMemoryPeaks() {
    MemoryMonitor::UpdatePeaks();
}

void PrintPeakMemoryUsage() {
    MemoryMonitor::PrintPeakUsage();
}

void ResetMemoryPeaks() {
    MemoryMonitor::ResetPeaks();
}

// C interface for heap management
extern "C" {
    void PrintHeapStatsC() {
        PrintHeapStats();
    }

    void ResetAllHeapsC() {
        ResetAllHeaps();
    }

    size_t GetTotalAllocatedObjectsC() {
        return GetTotalAllocatedObjects();
    }

    void GetMemoryUsage(int* plaintextCount, int* ciphertextCount) {
        if (plaintextCount) {
            *plaintextCount = static_cast<int>(g_ptHeap.Size());
        }
        if (ciphertextCount) {
            *ciphertextCount = static_cast<int>(g_ctHeap.Size());
        }
    }

    void CleanupExpiredObjectsC() {
        CleanupExpiredObjects();
    }

    void UpdateMemoryPeaksC() {
        UpdateMemoryPeaks();
    }

    void PrintPeakMemoryUsageC() {
        PrintPeakMemoryUsage();
    }

    void ResetMemoryPeaksC() {
        ResetMemoryPeaks();
    }
}

// HeapAllocator template specializations for debugging
template<>
void HeapAllocator::DebugPrint<int>() const {
    std::cout << "HeapAllocator<int>: " << Size() << " objects allocated" << std::endl;
    auto keys = GetLiveKeys();
    if (!keys.empty()) {
        std::cout << "  Live IDs: ";
        for (size_t i = 0; i < std::min(keys.size(), size_t(10)); ++i) {
            std::cout << keys[i] << " ";
        }
        if (keys.size() > 10) std::cout << "...";
        std::cout << std::endl;
    }
}

// Global heap accessor functions (moved from other modules)

// Plaintext heap operations
int PushPlaintext(const Plaintext& plaintext) {
    UpdateMemoryPeaks();
    return g_ptHeap.Add(plaintext);
}

Plaintext& RetrievePlaintext(int plaintextID) {
    return g_ptHeap.Retrieve<Plaintext>(plaintextID);
}

std::shared_ptr<Plaintext> GetPlaintextPtr(int plaintextID) {
    return g_ptHeap.GetSharedPtr<Plaintext>(plaintextID);
}

bool PlaintextExists(int plaintextID) {
    return g_ptHeap.Exists(plaintextID);
}

bool DeletePlaintext(int plaintextID) {
    return g_ptHeap.Delete(plaintextID);
}

// Ciphertext heap operations
int PushCiphertext(const Ciphertext<DCRTPoly>& ciphertext) {
    UpdateMemoryPeaks();
    return g_ctHeap.Add(ciphertext);
}

Ciphertext<DCRTPoly>& RetrieveCiphertext(int ciphertextID) {
    return g_ctHeap.Retrieve<Ciphertext<DCRTPoly>>(ciphertextID);
}

std::shared_ptr<Ciphertext<DCRTPoly>> GetCiphertextPtr(int ciphertextID) {
    return g_ctHeap.GetSharedPtr<Ciphertext<DCRTPoly>>(ciphertextID);
}

bool CiphertextExists(int ciphertextID) {
    return g_ctHeap.Exists(ciphertextID);
}

bool DeleteCiphertext(int ciphertextID) {
    return g_ctHeap.Delete(ciphertextID);
}

void ResetTensorHeaps() {
    g_ptHeap.Reset();
    g_ctHeap.Reset();
}

void GetTensorStats(size_t& plaintextCount, size_t& ciphertextCount) {
    plaintextCount = g_ptHeap.Size();
    ciphertextCount = g_ctHeap.Size();
}