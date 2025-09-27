#pragma once

#include "openfhe/pke/openfhe.h"
#include <vector>
#include <map>

using namespace lbcrypto;

/**
 * @brief OpenFHE Bootstrapper for noise management
 *
 * This module provides bootstrapping functionality to refresh ciphertexts
 * and manage noise accumulation in homomorphic encryption operations.
 * Bootstrapping allows for deeper computation by resetting the noise level.
 */

// C interface functions for bootstrapping operations
extern "C" {
    /**
     * @brief Initialize a new bootstrapper for given slot count
     *
     * @param LogPs Array of level budget parameters
     * @param lenLogPs Length of LogPs array
     * @param numSlots Number of slots for this bootstrapper
     */
    void NewBootstrapper(const int* LogPs, int lenLogPs, int numSlots);

    /**
     * @brief Perform bootstrapping on a ciphertext
     *
     * @param ciphertextID ID of the ciphertext to bootstrap
     * @param numSlots Number of slots (must match bootstrapper)
     * @return int ID of the bootstrapped ciphertext, -1 if failed
     */
    int Bootstrap(int ciphertextID, int numSlots);

    /**
     * @brief Get bootstrapper context for specific slot count
     *
     * @param numSlots Number of slots
     * @return CryptoContext<DCRTPoly> Bootstrapper context, nullptr if not found
     */
    CryptoContext<DCRTPoly> GetBootstrapper(int numSlots);

    /**
     * @brief Delete all bootstrappers and clean up resources
     */
    void DeleteBootstrappers();

    /**
     * @brief Set custom bootstrap parameters for specific slot count
     *
     * @param numSlots Number of slots
     * @param multDepth Multiplicative depth
     * @param scaleModSize Scaling modulus size
     */
    void SetBootstrapParameters(int numSlots, int multDepth, int scaleModSize);

    /**
     * @brief Get the number of active bootstrappers
     *
     * @return int Number of bootstrappers
     */
    int GetBootstrapperCount();

    /**
     * @brief Check if bootstrapper exists for given slot count
     *
     * @param numSlots Number of slots to check
     * @return bool True if bootstrapper exists, false otherwise
     */
    bool HasBootstrapper(int numSlots);

    /**
     * @brief Perform precision-preserving bootstrap
     *
     * @param ciphertextID ID of the ciphertext to bootstrap
     * @param numSlots Number of slots
     * @param targetPrecision Target precision after bootstrap
     * @return int ID of the bootstrapped ciphertext, -1 if failed
     */
    int BootstrapWithPrecision(int ciphertextID, int numSlots, double targetPrecision);
}