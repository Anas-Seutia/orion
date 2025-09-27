#include "utils.hpp"
#include "scheme.hpp"
#include "tensors.hpp"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cstdlib>
#include <cstring>
#include <sstream>

namespace ArrayUtils {
    std::vector<unsigned char> CArrayToByteVector(void* dataPtr, uint64_t length) {
        if (!dataPtr || length == 0) {
            return std::vector<unsigned char>();
        }

        unsigned char* bytePtr = static_cast<unsigned char*>(dataPtr);
        return std::vector<unsigned char>(bytePtr, bytePtr + length);
    }

    // Template functions are defined in the header file
}

namespace TypeConversion {
    int convertCIntToInt(int v) {
        return v;
    }

    float convertCFloatToFloat(float v) {
        return v;
    }

    double convertFloat64ToCDouble(double v) {
        return v;
    }

    int convertIntToCInt(int v) {
        return v;
    }

    unsigned long convertULongtoCULong(uint64_t v) {
        return static_cast<unsigned long>(v);
    }

    int convertULongtoInt(uint64_t v) {
        return static_cast<int>(v);
    }

    char convertByteToCChar(unsigned char b) {
        return static_cast<char>(b);
    }
}

namespace MapUtils {
    // Template functions are defined in the header file
}

namespace DebugUtils {
    void PrintCiphertext(const Ciphertext<DCRTPoly>& ciphertext, int maxElements) {
        if (!ciphertext) {
            std::cout << "Ciphertext is null" << std::endl;
            return;
        }

        try {
            std::cout << "=== Ciphertext Info ===" << std::endl;
            std::cout << "Level: " << ciphertext->GetLevel() << std::endl;
            std::cout << "Scaling Factor: " << ciphertext->GetScalingFactor() << std::endl;
            std::cout << "Number of elements: " << ciphertext->GetElements().size() << std::endl;

            // If we have access to secret key and context, we could decrypt and show values
            if (g_scheme.IsInitialized() && g_scheme.secretKey) {
                try {
                    Plaintext plaintext;
                    g_scheme.context->Decrypt(g_scheme.secretKey, ciphertext, &plaintext);

                    std::vector<double> values = plaintext->GetRealPackedValue();
                    int printCount = std::min(maxElements, static_cast<int>(values.size()));

                    std::cout << "First " << printCount << " decrypted values: ";
                    for (int i = 0; i < printCount; ++i) {
                        std::cout << values[i];
                        if (i < printCount - 1) std::cout << ", ";
                    }
                    std::cout << std::endl;

                } catch (const std::exception& e) {
                    std::cout << "Could not decrypt for debugging: " << e.what() << std::endl;
                }
            }

        } catch (const std::exception& e) {
            std::cout << "Error printing ciphertext: " << e.what() << std::endl;
        }
    }

    void PrintPlaintext(const Plaintext& plaintext, int maxElements) {
        if (!plaintext) {
            std::cout << "Plaintext is null" << std::endl;
            return;
        }

        try {
            std::cout << "=== Plaintext Info ===" << std::endl;
            std::cout << "Level: " << plaintext->GetLevel() << std::endl;
            std::cout << "Scaling Factor: " << plaintext->GetScalingFactor() << std::endl;

            std::vector<double> values = plaintext->GetRealPackedValue();
            int printCount = std::min(maxElements, static_cast<int>(values.size()));

            std::cout << "First " << printCount << " values: ";
            for (int i = 0; i < printCount; ++i) {
                std::cout << values[i];
                if (i < printCount - 1) std::cout << ", ";
            }
            std::cout << std::endl;

        } catch (const std::exception& e) {
            std::cout << "Error printing plaintext: " << e.what() << std::endl;
        }
    }

    void PrintVector(const std::vector<double>& values, const std::string& label, int maxElements) {
        std::cout << "=== " << label << " ===" << std::endl;

        if (values.empty()) {
            std::cout << "Vector is empty" << std::endl;
            return;
        }

        int printCount = std::min(maxElements, static_cast<int>(values.size()));
        std::cout << "Size: " << values.size() << ", showing first " << printCount << " values: ";

        for (int i = 0; i < printCount; ++i) {
            std::cout << values[i];
            if (i < printCount - 1) std::cout << ", ";
        }

        if (values.size() > static_cast<size_t>(maxElements)) {
            std::cout << "... (and " << (values.size() - maxElements) << " more)";
        }

        std::cout << std::endl;
    }

    std::string GetMemoryStats() {
        std::stringstream ss;

        size_t plaintextCount, ciphertextCount;
        GetTensorStats(plaintextCount, ciphertextCount);

        ss << "Memory Statistics:" << std::endl;
        ss << "  Active Plaintexts: " << plaintextCount << std::endl;
        ss << "  Active Ciphertexts: " << ciphertextCount << std::endl;
        ss << "  Total Objects: " << (plaintextCount + ciphertextCount) << std::endl;

        return ss.str();
    }
}

namespace StringUtils {
    std::string SafeCStringToString(const char* cstr, const std::string& defaultValue) {
        return cstr ? std::string(cstr) : defaultValue;
    }

    std::string ToLowerCase(const std::string& str) {
        std::string result = str;
        std::transform(result.begin(), result.end(), result.begin(),
                      [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    bool IsValidRingType(const std::string& ringType) {
        std::string lower = ToLowerCase(ringType);
        return (lower == "standard" || lower == "conjugate_invariant" ||
                lower == "conjugateinvariant" || lower == "");
    }
}

namespace MathUtils {
    uint32_t NextPowerOfTwo(uint32_t n) {
        if (n == 0) return 1;

        --n;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;

        return n + 1;
    }

    int Log2(uint32_t n) {
        if (n == 0) return -1;

        int log = 0;
        while (n >>= 1) {
            ++log;
        }

        return log;
    }

    bool IsPowerOfTwo(uint32_t n) {
        return n > 0 && (n & (n - 1)) == 0;
    }

    double MinValue(const std::vector<double>& values) {
        if (values.empty()) {
            return 0.0;
        }

        return *std::min_element(values.begin(), values.end());
    }

    double MaxValue(const std::vector<double>& values) {
        if (values.empty()) {
            return 0.0;
        }

        return *std::max_element(values.begin(), values.end());
    }

    double MeanValue(const std::vector<double>& values) {
        if (values.empty()) {
            return 0.0;
        }

        double sum = std::accumulate(values.begin(), values.end(), 0.0);
        return sum / values.size();
    }
}

// C interface implementations
extern "C" {
    void FreeCArray(void* ptr) {
        if (ptr) {
            free(ptr);
        }
    }

    // GetMemoryUsage is defined in minheap.cpp, not here
}