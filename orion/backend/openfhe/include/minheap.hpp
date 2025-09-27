#pragma once

#include <queue>
#include <unordered_map>
#include <memory>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <iostream>
#include "openfhe/pke/openfhe.h"

using namespace lbcrypto;

/**
 * @brief Min-heap based memory allocator for managing object lifetimes with integer IDs
 * 
 * This class provides efficient allocation and deallocation of integer IDs for objects,
 * reusing freed IDs to minimize memory fragmentation. It's designed to match the 
 * Lattigo backend's HeapAllocator functionality.
 */
class HeapAllocator {
private:
    int nextInt;                                         // Next integer to allocate
    std::priority_queue<int, std::vector<int>, std::greater<int>> freedIntegers;  // Min-heap for freed IDs
    std::unordered_map<int, std::shared_ptr<void>> objectMap;  // Map to store objects by ID

public:
    /**
     * @brief Construct a new Heap Allocator
     */
    HeapAllocator() : nextInt(0) {}

    /**
     * @brief Add an object to the allocator and return its ID
     * 
     * @tparam T Type of the object to store
     * @param obj Object to store
     * @return int Unique ID assigned to the object
     */
    template<typename T>
    int Add(const T& obj) {
        int allocated;
        
        if (!freedIntegers.empty()) {
            // Reuse the smallest available integer from the heap
            allocated = freedIntegers.top();
            freedIntegers.pop();
        } else {
            // Allocate a new integer
            allocated = nextInt++;
        }
        
        // Store the object as a shared_ptr
        objectMap[allocated] = std::make_shared<T>(obj);
        return allocated;
    }

    /**
     * @brief Retrieve an object by its ID
     * 
     * @tparam T Type of the object to retrieve
     * @param id ID of the object
     * @return T& Reference to the stored object
     * @throws std::runtime_error if ID not found
     */
    template<typename T>
    T& Retrieve(int id) {
        auto it = objectMap.find(id);
        if (it == objectMap.end()) {
            throw std::runtime_error("Heap object not found for ID: " + std::to_string(id));
        }
        
        auto ptr = std::static_pointer_cast<T>(it->second);
        if (!ptr) {
            throw std::runtime_error("Invalid type cast for object ID: " + std::to_string(id));
        }
        
        return *ptr;
    }

    /**
     * @brief Get a shared pointer to an object by its ID
     * 
     * @tparam T Type of the object
     * @param id ID of the object
     * @return std::shared_ptr<T> Shared pointer to the object
     * @throws std::runtime_error if ID not found
     */
    template<typename T>
    std::shared_ptr<T> GetSharedPtr(int id) {
        auto it = objectMap.find(id);
        if (it == objectMap.end()) {
            throw std::runtime_error("Heap object not found for ID: " + std::to_string(id));
        }
        
        auto ptr = std::static_pointer_cast<T>(it->second);
        if (!ptr) {
            throw std::runtime_error("Invalid type cast for object ID: " + std::to_string(id));
        }
        
        return ptr;
    }

    /**
     * @brief Delete an object and free its ID for reuse
     * 
     * @param id ID of the object to delete
     * @return true if object was deleted, false if ID not found
     */
    bool Delete(int id) {
        auto it = objectMap.find(id);
        if (it != objectMap.end()) {
            objectMap.erase(it);
            freedIntegers.push(id);
            return true;
        }
        return false;
    }

    /**
     * @brief Check if an object exists with the given ID
     * 
     * @param id ID to check
     * @return true if object exists, false otherwise
     */
    bool Exists(int id) const {
        return objectMap.find(id) != objectMap.end();
    }

    /**
     * @brief Get all live (allocated) IDs
     *
     * @return std::vector<int> Vector of all currently allocated IDs
     */
    std::vector<int> GetLiveKeys() const {
        std::vector<int> keys;
        keys.reserve(objectMap.size());

        for (const auto& pair : objectMap) {
            keys.push_back(pair.first);
        }

        return keys;
    }

    /**
     * @brief Get all active (allocated) IDs - alias for GetLiveKeys()
     *
     * @return std::vector<int> Vector of all currently allocated IDs
     */
    std::vector<int> GetActiveIDs() const {
        return GetLiveKeys();
    }

    /**
     * @brief Get the number of currently allocated objects
     * 
     * @return size_t Number of allocated objects
     */
    size_t Size() const {
        return objectMap.size();
    }

    /**
     * @brief Reset the allocator, clearing all objects and resetting state
     */
    void Reset() {
        nextInt = 0;
        objectMap.clear();
        
        // Clear the priority queue
        while (!freedIntegers.empty()) {
            freedIntegers.pop();
        }
    }

    /**
     * @brief Print debug information about this allocator
     */
    template<typename T>
    void DebugPrint() const;

    /**
     * @brief Destructor - automatically cleans up all stored objects
     */
    ~HeapAllocator() {
        Reset();
    }
};

// Global heap allocators
extern HeapAllocator g_ptHeap;   // Plaintext heap
extern HeapAllocator g_ctHeap;   // Ciphertext heap
extern HeapAllocator g_ltHeap;   // Linear transform heap

// Utility functions
void PrintHeapStats();
void ResetAllHeaps();
size_t GetTotalAllocatedObjects();
std::vector<int> GetAllPlaintextIDs();
std::vector<int> GetAllCiphertextIDs();
std::vector<int> GetAllLinearTransformIDs();
void CleanupExpiredObjects();

// Memory monitoring
void UpdateMemoryPeaks();
void PrintPeakMemoryUsage();
void ResetMemoryPeaks();

// Tensor heap operations (moved from tensors.hpp)
int PushPlaintext(const Plaintext& plaintext);
Plaintext& RetrievePlaintext(int plaintextID);
std::shared_ptr<Plaintext> GetPlaintextPtr(int plaintextID);
bool PlaintextExists(int plaintextID);
bool DeletePlaintext(int plaintextID);

int PushCiphertext(const Ciphertext<DCRTPoly>& ciphertext);
Ciphertext<DCRTPoly>& RetrieveCiphertext(int ciphertextID);
std::shared_ptr<Ciphertext<DCRTPoly>> GetCiphertextPtr(int ciphertextID);
bool CiphertextExists(int ciphertextID);
bool DeleteCiphertext(int ciphertextID);

void ResetTensorHeaps();
void GetTensorStats(size_t& plaintextCount, size_t& ciphertextCount);

// C interface
extern "C" {
    void PrintHeapStatsC();
    void ResetAllHeapsC();
    size_t GetTotalAllocatedObjectsC();
    void GetMemoryUsage(int* plaintextCount, int* ciphertextCount);
    int* GetLivePlaintexts(int* count);
    int* GetLiveCiphertexts(int* count);
    void CleanupExpiredObjectsC();
    void UpdateMemoryPeaksC();
    void PrintPeakMemoryUsageC();
    void ResetMemoryPeaksC();
}