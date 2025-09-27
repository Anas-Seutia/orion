#pragma once

#include "openfhe/pke/openfhe.h"
#include <vector>
#include <map>
#include <string>

using namespace lbcrypto;

/**
 * @brief OpenFHE Polynomial Evaluator for homomorphic computation
 *
 * This module provides polynomial evaluation functionality for homomorphic
 * encryption. It supports both monomial and Chebyshev polynomial evaluation
 * as well as minimax approximation generation for complex functions.
 */

// C interface functions for polynomial evaluation operations
extern "C" {
    /**
     * @brief Add polynomial coefficients to storage
     *
     * @param coeffs Array of polynomial coefficients
     * @param length Number of coefficients
     * @return int Polynomial ID, -1 if failed
     */
    int AddPoly(const double* coeffs, int length);

    /**
     * @brief Retrieve polynomial coefficients from storage
     *
     * @param polyID Polynomial ID
     * @return std::vector<double> Polynomial coefficients (empty if not found)
     */
    std::vector<double> RetrievePoly(int polyID);

    /**
     * @brief Delete polynomial from storage
     *
     * @param polyID Polynomial ID to delete
     */
    void DeletePoly(int polyID);

    /**
     * @brief Initialize a new polynomial evaluator
     */
    void NewPolynomialEvaluator();

    /**
     * @brief Generate monomial polynomial from coefficients
     *
     * @param coeffs Array of monomial coefficients
     * @param lenCoeffs Number of coefficients
     * @return int Polynomial ID, -1 if failed
     */
    int GenerateMonomial(const double* coeffs, int lenCoeffs);

    /**
     * @brief Generate Chebyshev polynomial from coefficients
     *
     * @param coeffs Array of Chebyshev coefficients
     * @param lenCoeffs Number of coefficients
     * @return int Polynomial ID, -1 if failed
     */
    int GenerateChebyshev(const double* coeffs, int lenCoeffs);

    /**
     * @brief Evaluate polynomial on encrypted ciphertext
     *
     * @param ctInID Input ciphertext ID
     * @param polyID Polynomial ID to evaluate
     * @param outScale Output scaling factor
     * @return int Output ciphertext ID, -1 if failed
     */
    int EvaluatePolynomial(int ctInID, int polyID, uint64_t outScale);

    /**
     * @brief Generate minimax approximation coefficients for sign function
     *
     * @param degrees Array of polynomial degrees for different levels
     * @param lenDegrees Length of degrees array
     * @param prec Precision parameter
     * @param logalpha Log of alpha parameter
     * @param logerr Log of error parameter
     * @param debug Debug flag
     * @param outLength Pointer to store output length
     * @return double* Minimax coefficients (caller must free), nullptr if failed
     */
    double* GenerateMinimaxSignCoeffs(
        const int* degrees, int lenDegrees,
        int prec,
        int logalpha,
        int logerr,
        int debug,
        unsigned long* outLength
    );

    /**
     * @brief Delete all stored minimax sign coefficients
     */
    void DeleteMinimaxSignMap();
}