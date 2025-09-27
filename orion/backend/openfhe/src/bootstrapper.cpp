#include "bootstrapper.hpp"
#include "scheme.hpp"
#include "tensors.hpp"
#include "openfhe.h"
#include <iostream>
#include <memory>
#include <map>

using namespace lbcrypto;

// Map to store bootstrapping contexts by slot count
static std::map<int, CryptoContext<DCRTPoly>> bootstrapperMap;

extern "C" {

void NewBootstrapper(const int* LogPs, int lenLogPs, int numSlots) {
    if (!g_scheme.IsInitialized() || !g_scheme.context || !g_scheme.secretKey) {
        std::cerr << "Error: CryptoContext or SecretKey not initialized" << std::endl;
        return;
    }

    // Check if bootstrapper already exists for this slot count
    auto it = bootstrapperMap.find(numSlots);
    if (it != bootstrapperMap.end()) {
        return; // Already initialized
    }

    try {
        // Enable FHE for bootstrapping if not already enabled
        g_scheme.context->Enable(FHE);

        // Set up bootstrapping parameters
        std::vector<uint32_t> levelBudget = {4, 4};
        if (lenLogPs >= 2) {
            levelBudget[0] = LogPs[0];
            levelBudget[1] = LogPs[1];
        }

        // Setup bootstrapping
        g_scheme.context->EvalBootstrapSetup(levelBudget);

        // Generate bootstrapping keys
        g_scheme.context->EvalBootstrapKeyGen(g_scheme.secretKey, numSlots);

        // Store the current context for this slot count
        bootstrapperMap[numSlots] = g_scheme.context;

        std::cout << "Bootstrapper initialized for " << numSlots << " slots" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "Error creating bootstrapper: " << e.what() << std::endl;
        std::cerr << "Note: Bootstrapping requires specific parameter setup and may not work with all configurations" << std::endl;
    }
}

int Bootstrap(int ciphertextID, int numSlots) {
    // Find the appropriate bootstrapper
    auto it = bootstrapperMap.find(numSlots);
    if (it == bootstrapperMap.end()) {
        std::cerr << "Error: No bootstrapper found for " << numSlots << " slots" << std::endl;
        return -1;
    }

    auto ccBootstrap = it->second;

    // Retrieve the input ciphertext
    auto& ctIn = RetrieveCiphertext(ciphertextID);
    if (!ctIn) {
        std::cerr << "Error: Invalid ciphertext ID" << std::endl;
        return -1;
    }

    try {
        // Perform bootstrapping using OpenFHE's EvalBootstrap
        auto ctOut = ccBootstrap->EvalBootstrap(ctIn);

        if (!ctOut) {
            std::cerr << "Error: Bootstrap operation failed" << std::endl;
            return -1;
        }

        // Store the result and return its ID
        int ctOutID = PushCiphertext(ctOut);
        return ctOutID;

    } catch (const std::exception& e) {
        std::cerr << "Error during bootstrap: " << e.what() << std::endl;
        std::cerr << "Note: Bootstrapping may fail if ciphertext parameters don't match bootstrap setup" << std::endl;
        return -1;
    }
}

CryptoContext<DCRTPoly> GetBootstrapper(int numSlots) {
    auto it = bootstrapperMap.find(numSlots);
    if (it == bootstrapperMap.end()) {
        std::cerr << "Error: No bootstrapper found for slot count: " << numSlots << std::endl;
        return nullptr;
    }
    return it->second;
}

void DeleteBootstrappers() {
    bootstrapperMap.clear();
    std::cout << "All bootstrappers deleted" << std::endl;
}

// Additional bootstrapping utility functions

int GetBootstrapperCount() {
    return bootstrapperMap.size();
}

bool HasBootstrapper(int numSlots) {
    return bootstrapperMap.find(numSlots) != bootstrapperMap.end();
}

// Precision-preserving bootstrap variants
int BootstrapWithPrecision(int ciphertextID, int numSlots, double targetPrecision) {
    auto it = bootstrapperMap.find(numSlots);
    if (it == bootstrapperMap.end()) {
        std::cerr << "Error: No bootstrapper found for " << numSlots << " slots" << std::endl;
        return -1;
    }

    auto ccBootstrap = it->second;
    auto& ctIn = RetrieveCiphertext(ciphertextID);
    if (!ctIn) {
        std::cerr << "Error: Invalid ciphertext ID" << std::endl;
        return -1;
    }

    try {
        auto ctBtp = ctIn->Clone();

        // Adjust precision parameters if needed
        // This would typically involve setting specific modulus chain parameters
        auto ctOut = ccBootstrap->EvalBootstrap(ctBtp);

        if (!ctOut) {
            std::cerr << "Error: Precision bootstrap operation failed" << std::endl;
            return -1;
        }

        int ctOutID = PushCiphertext(ctOut);
        return ctOutID;

    } catch (const std::exception& e) {
        std::cerr << "Error during precision bootstrap: " << e.what() << std::endl;
        return -1;
    }
}

} // extern "C"