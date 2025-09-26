#pragma once

#include "openfhe/pke/openfhe.h"
#include <vector>
#include <string>
#include <map>
#include <memory>
#include <functional>

using namespace lbcrypto;

/**
 * @brief Utility functions for OpenFHE backend
 * 
 * This module provides various utility functions for type conversions,
 * array management, and debugging support. It matches the functionality
 * of the Lattigo backend utilities.
 */

// Type conversion functions for C interface compatibility
namespace TypeConverter {
    /**
     * @brief Convert C int to C++ int
     */
    inline int CIntToInt(int v) { return v; }

    /**
     * @brief Convert C float to C++ double
     */
    inline double CFloatToDouble(float v) { return static_cast<double>(v); }

    /**
     * @brief Convert C double to C++ double
     */
    inline double CDoubleToDouble(double v) { return v; }

    /**
     * @brief Convert C++ double to C float
     */
    inline float DoubleToCFloat(double v) { return static_cast<float>(v); }

    /**
     * @brief Convert C++ double to C double
     */
    inline double DoubleToCDouble(double v) { return v; }

    /**
     * @brief Convert C++ int to C int
     */
    inline int IntToCInt(int v) { return v; }

    /**
     * @brief Convert uint64_t to unsigned long
     */
    inline unsigned long UInt64ToCULong(uint64_t v) { return static_cast<unsigned long>(v); }

    /**
     * @brief Convert uint64_t to int
     */
    inline int UInt64ToInt(uint64_t v) { return static_cast<int>(v); }

    /**
     * @brief Convert byte to char
     */
    inline char ByteToCChar(unsigned char b) { return static_cast<char>(b); }
}

// Array conversion utilities
namespace ArrayUtils {
    /**
     * @brief Convert C array to C++ vector
     * 
     * @tparam T Target type
     * @tparam U Source type
     * @param ptr Pointer to C array
     * @param length Length of the array
     * @param converter Conversion function
     * @return std::vector<T> Converted vector
     */
    template<typename T, typename U>
    std::vector<T> CArrayToVector(const U* ptr, int length, std::function<T(U)> converter) {
        if (!ptr || length <= 0) {
            return std::vector<T>();
        }

        std::vector<T> result;
        result.reserve(length);

        for (int i = 0; i < length; ++i) {
            result.push_back(converter(ptr[i]));
        }

        return result;
    }

    /**
     * @brief Convert C++ vector to C array (allocated with malloc)
     * 
     * @tparam T Source type
     * @tparam U Target type
     * @param vec Source vector
     * @param converter Conversion function
     * @return std::pair<U*, size_t> Pointer to allocated array and its length
     */
    template<typename T, typename U>
    std::pair<U*, size_t> VectorToCArray(const std::vector<T>& vec, std::function<U(T)> converter) {
        if (vec.empty()) {
            return std::make_pair(nullptr, 0);
        }

        size_t length = vec.size();
        U* array = static_cast<U*>(malloc(length * sizeof(U)));
        
        if (!array) {
            return std::make_pair(nullptr, 0);
        }

        for (size_t i = 0; i < length; ++i) {
            array[i] = converter(vec[i]);
        }

        return std::make_pair(array, length);
    }

    /**
     * @brief Convert byte array to C++ byte vector
     * 
     * @param dataPtr Pointer to byte data
     * @param length Length of data
     * @return std::vector<unsigned char> Byte vector
     */
    std::vector<unsigned char> CArrayToByteVector(void* dataPtr, uint64_t length);
}

// Map utility functions
namespace MapUtils {
    /**
     * @brief Get keys from a map
     * 
     * @tparam K Key type
     * @tparam V Value type
     * @param map Input map
     * @return std::vector<K> Vector of keys
     */
    template<typename K, typename V>
    std::vector<K> GetKeysFromMap(const std::map<K, V>& map) {
        std::vector<K> keys;
        keys.reserve(map.size());

        for (const auto& pair : map) {
            keys.push_back(pair.first);
        }

        return keys;
    }

    /**
     * @brief Get values from a map
     * 
     * @tparam K Key type
     * @tparam V Value type
     * @param map Input map
     * @return std::vector<V> Vector of values
     */
    template<typename K, typename V>
    std::vector<V> GetValuesFromMap(const std::map<K, V>& map) {
        std::vector<V> values;
        values.reserve(map.size());

        for (const auto& pair : map) {
            values.push_back(pair.second);
        }

        return values;
    }
}

// Debugging utilities
namespace DebugUtils {
    /**
     * @brief Print ciphertext information for debugging
     * 
     * @param ciphertext Ciphertext to examine
     * @param maxElements Maximum number of elements to print
     */
    void PrintCiphertext(const Ciphertext<DCRTPoly>& ciphertext, int maxElements = 16);

    /**
     * @brief Print plaintext information for debugging
     * 
     * @param plaintext Plaintext to examine
     * @param maxElements Maximum number of elements to print
     */
    void PrintPlaintext(const Plaintext& plaintext, int maxElements = 16);

    /**
     * @brief Print vector values for debugging
     * 
     * @param values Vector to print
     * @param label Label for the output
     * @param maxElements Maximum number of elements to print
     */
    void PrintVector(const std::vector<double>& values, const std::string& label = "Vector", int maxElements = 16);

    /**
     * @brief Get memory usage statistics as a string
     * 
     * @return std::string Memory usage information
     */
    std::string GetMemoryStats();
}

// String utilities
namespace StringUtils {
    /**
     * @brief Convert C string to C++ string safely
     * 
     * @param cstr C string pointer (may be null)
     * @param defaultValue Default value if cstr is null
     * @return std::string Converted string
     */
    std::string SafeCStringToString(const char* cstr, const std::string& defaultValue = "");

    /**
     * @brief Convert C++ string to lowercase
     * 
     * @param str Input string
     * @return std::string Lowercase string
     */
    std::string ToLowerCase(const std::string& str);

    /**
     * @brief Check if string represents a valid ring type
     * 
     * @param ringType Ring type string
     * @return true if valid, false otherwise
     */
    bool IsValidRingType(const std::string& ringType);
}

// Mathematical utilities
namespace MathUtils {
    /**
     * @brief Calculate the next power of 2 greater than or equal to n
     * 
     * @param n Input value
     * @return uint32_t Next power of 2
     */
    uint32_t NextPowerOfTwo(uint32_t n);

    /**
     * @brief Calculate log base 2 of n (assuming n is a power of 2)
     * 
     * @param n Input value
     * @return int Log base 2
     */
    int Log2(uint32_t n);

    /**
     * @brief Check if n is a power of 2
     * 
     * @param n Input value
     * @return true if power of 2, false otherwise
     */
    bool IsPowerOfTwo(uint32_t n);

    /**
     * @brief Calculate the minimum value in a vector
     * 
     * @param values Input vector
     * @return double Minimum value
     */
    double MinValue(const std::vector<double>& values);

    /**
     * @brief Calculate the maximum value in a vector
     * 
     * @param values Input vector
     * @return double Maximum value
     */
    double MaxValue(const std::vector<double>& values);

    /**
     * @brief Calculate the mean of values in a vector
     * 
     * @param values Input vector
     * @return double Mean value
     */
    double MeanValue(const std::vector<double>& values);
}

// C interface utility functions
extern "C" {
    /**
     * @brief Free a C array allocated by VectorToCArray
     * 
     * @param ptr Pointer to free
     */
    void FreeCArray(void* ptr);

    /**
     * @brief Get current memory usage statistics
     * 
     * @param plaintextCount Output: number of active plaintexts
     * @param ciphertextCount Output: number of active ciphertexts
     */
    void GetMemoryUsage(int* plaintextCount, int* ciphertextCount);
}