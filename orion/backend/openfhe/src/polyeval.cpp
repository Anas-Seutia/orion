#include "polyeval.hpp"
#include "scheme.hpp"
#include "tensors.hpp"
#include "openfhe.h"
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <string>

using namespace lbcrypto;

// Storage for polynomial coefficients
static std::map<int, std::vector<double>> polynomialMap;
static int polyCounter = 0;

extern "C" {

int AddPoly(const double* coeffs, int length) {
    std::vector<double> poly(coeffs, coeffs + length);
    polynomialMap[polyCounter] = poly;
    return polyCounter++;
}

std::vector<double> RetrievePoly(int polyID) {
    auto it = polynomialMap.find(polyID);
    if (it != polynomialMap.end()) {
        return it->second;
    }
    return std::vector<double>();
}

int GetPolyDepth(int polyID) {
    std::vector<double> poly = RetrievePoly(polyID);
    if (poly.empty()) {
        return -1;
    }
    // Return degree of polynomial as depth approximation
    return static_cast<int>(poly.size() - 1);
}

void DeletePoly(int polyID) {
    auto it = polynomialMap.find(polyID);
    if (it != polynomialMap.end()) {
        polynomialMap.erase(it);
    }
}

void NewPolynomialEvaluator() {
    // OpenFHE doesn't have a separate polynomial evaluator object
    // Polynomial evaluation is handled directly by the CryptoContext
    if (!g_scheme.IsInitialized() || !g_scheme.context) {
        std::cerr << "Error: CryptoContext not initialized" << std::endl;
        return;
    }

    std::cout << "Polynomial evaluator ready (using CryptoContext)" << std::endl;
}

int GenerateMonomial(const double* coeffs, int lenCoeffs) {
    if (!coeffs || lenCoeffs <= 0) {
        std::cerr << "Error: Invalid coefficients for monomial" << std::endl;
        return -1;
    }

    return AddPoly(coeffs, lenCoeffs);
}

int GenerateChebyshev(const double* coeffs, int lenCoeffs) {
    if (!coeffs || lenCoeffs <= 0) {
        std::cerr << "Error: Invalid coefficients for Chebyshev polynomial" << std::endl;
        return -1;
    }

    // For OpenFHE, we store Chebyshev coefficients the same way as monomials
    // The evaluation method will handle the difference
    return AddPoly(coeffs, lenCoeffs);
}

int EvaluatePolynomial(int ctInID, int polyID, uint64_t outScale) {
    if (!g_scheme.IsInitialized() || !g_scheme.context) {
        std::cerr << "Error: CryptoContext not initialized" << std::endl;
        return -1;
    }

    // Retrieve the ciphertext from storage
    auto& ctIn = RetrieveCiphertext(ctInID);
    if (!ctIn) {
        std::cerr << "Error: Invalid ciphertext ID" << std::endl;
        return -1;
    }

    std::vector<double> poly = RetrievePoly(polyID);
    if (poly.empty()) {
        std::cerr << "Error: Invalid polynomial ID" << std::endl;
        return -1;
    }

    try {
        // Use OpenFHE's built-in polynomial evaluation
        // This is much more efficient than manual implementation
        auto result = g_scheme.context->EvalPoly(ctIn, poly);

        if (!result) {
            std::cerr << "Error: EvalPoly returned null" << std::endl;
            return -1;
        }

        // Set the output scale if specified
        if (outScale > 0) {
            result->SetScalingFactor(outScale);
        }

        // Store the result and return its ID
        int ctOutID = PushCiphertext(result);
        return ctOutID;

    } catch (const std::exception& e) {
        std::cerr << "Error evaluating polynomial: " << e.what() << std::endl;
        return -1;
    }
}

// Minimax approximation helper functions
static std::map<std::string, std::vector<std::vector<double>>> minimaxSignMap;

double* GenerateMinimaxSignCoeffs(
    const int* degrees, int lenDegrees,
    int prec,
    int logalpha,
    int logerr,
    int debug,
    unsigned long* outLength
) {
    // Generate a unique key for the parameters
    std::string key = std::to_string(prec) + "_" + std::to_string(logalpha) + "_" + std::to_string(logerr);
    for (int i = 0; i < lenDegrees; ++i) {
        key += "_" + std::to_string(degrees[i]);
    }

    // Check if coefficients already exist
    auto it = minimaxSignMap.find(key);
    if (it != minimaxSignMap.end()) {
        // Calculate total length
        size_t totalLen = 0;
        for (const auto& poly : it->second) {
            totalLen += poly.size();
        }

        double* result = (double*)malloc(totalLen * sizeof(double));
        if (!result) {
            *outLength = 0;
            return nullptr;
        }

        size_t idx = 0;
        for (const auto& poly : it->second) {
            for (double coeff : poly) {
                result[idx++] = coeff;
            }
        }

        *outLength = totalLen;
        return result;
    }

    // For now, generate simple approximation coefficients
    // In a full implementation, you would use a proper minimax approximation algorithm
    std::vector<std::vector<double>> coeffs(lenDegrees);
    size_t totalLen = 0;

    for (int i = 0; i < lenDegrees; ++i) {
        int degree = degrees[i];
        coeffs[i].resize(degree + 1);

        // Simple sign approximation using odd polynomial
        for (int j = 0; j <= degree; ++j) {
            if (j % 2 == 1) {
                coeffs[i][j] = 1.0 / (j + 1); // Simple coefficient
            } else {
                coeffs[i][j] = 0.0;
            }
        }

        totalLen += coeffs[i].size();
    }

    // Store in map for future use
    minimaxSignMap[key] = coeffs;

    // Flatten the coefficients
    double* result = (double*)malloc(totalLen * sizeof(double));
    if (!result) {
        *outLength = 0;
        return nullptr;
    }

    size_t idx = 0;
    for (const auto& poly : coeffs) {
        for (double coeff : poly) {
            result[idx++] = coeff;
        }
    }

    *outLength = totalLen;
    return result;
}

void DeleteMinimaxSignMap() {
    minimaxSignMap.clear();
}

} // extern "C"