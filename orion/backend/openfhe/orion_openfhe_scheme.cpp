// orion_openfhe_scheme.cpp
// OpenFHE scheme implementation for Orion compatibility

#include "openfhe/pke/openfhe.h"
#include <vector>
#include <memory>
#include <string>
#include <map>

using namespace lbcrypto;

class OrionOpenFHEScheme {
private:
    CryptoContext<DCRTPoly> context;
    KeyPair<DCRTPoly> keyPair;
    std::map<int, Plaintext> plaintexts;
    std::map<int, Ciphertext<DCRTPoly>> ciphertexts;
    std::map<int, std::vector<double>> linTransforms;
    int nextPlaintextId;
    int nextCiphertextId;
    int nextTransformId;
    bool initialized;

public:
    OrionOpenFHEScheme() : nextPlaintextId(0), nextCiphertextId(0), nextTransformId(0), initialized(false) {}

    // Initialize the scheme with parameters
    bool InitializeScheme(int logN, const std::vector<int>& logQ, const std::vector<int>& logP,
                         int logScale, int hammingWeight, int ringType,
                         const std::string& keysPath, const std::string& ioMode) {
        try {
            CCParams<CryptoContextCKKSRNS> parameters;

            // Set ring dimension
            uint32_t ringDim = 1 << logN;
            parameters.SetRingDim(ringDim);

            // Set multiplicative depth based on logQ size
            uint32_t multDepth = logQ.size() - 1;
            parameters.SetMultiplicativeDepth(multDepth);

            // Set scaling modulus
            uint32_t scaleModSize = logScale;
            parameters.SetScalingModSize(scaleModSize);

            // Set security level
            parameters.SetSecurityLevel(HEStd_128_classic);

            // Set secret key distribution
            parameters.SetSecretKeyDist(UNIFORM_TERNARY);

            // Create context
            context = GenCryptoContext(parameters);
            context->Enable(PKE);
            context->Enable(KEYSWITCH);
            context->Enable(LEVELEDSHE);
            context->Enable(ADVANCEDSHE);
            
            // Generate keys
            keyPair = context->KeyGen();
            context->EvalMultKeyGen(keyPair.secretKey);
            context->EvalRotateKeyGen(keyPair.secretKey, {1, -1});

            initialized = true;
            return true;
        } catch (const std::exception& e) {
            initialized = false;
            return false;
        }
    }

    // Plaintext operations
    int CreatePlaintext(const std::vector<double>& values) {
        if (!initialized) return -1;

        try {
            Plaintext pt = context->MakeCKKSPackedPlaintext(values);
            int id = nextPlaintextId++;
            plaintexts[id] = pt;
            return id;
        } catch (const std::exception& e) {
            return -1;
        }
    }

    bool DeletePlaintext(int id) {
        return plaintexts.erase(id) > 0;
    }

    double GetPlaintextScale(int id) {
        if (!initialized) return 0.0;
        if (plaintexts.find(id) != plaintexts.end()) {
            return plaintexts[id]->GetScalingFactor();
        }
        return 0.0;
    }

    bool SetPlaintextScale(int id, double scale) {
        if (!initialized) return false;
        if (plaintexts.find(id) != plaintexts.end()) {
            plaintexts[id]->SetScalingFactor(scale);
            return true;
        }
        return false;
    }

    // Ciphertext operations
    int Encrypt(int plaintextId) {
        try {
            if (plaintexts.find(plaintextId) == plaintexts.end()) return -1;
            
            Ciphertext<DCRTPoly> ct = context->Encrypt(keyPair.publicKey, plaintexts[plaintextId]);
            int id = nextCiphertextId++;
            ciphertexts[id] = ct;
            return id;
        } catch (const std::exception& e) {
            return -1;
        }
    }

    int Decrypt(int ciphertextId) {
        try {
            if (ciphertexts.find(ciphertextId) == ciphertexts.end()) return -1;
            
            Plaintext pt;
            context->Decrypt(keyPair.secretKey, ciphertexts[ciphertextId], &pt);
            int id = nextPlaintextId++;
            plaintexts[id] = pt;
            return id;
        } catch (const std::exception& e) {
            return -1;
        }
    }

    bool DeleteCiphertext(int id) {
        return ciphertexts.erase(id) > 0;
    }

    double GetCiphertextScale(int id) {
        if (ciphertexts.find(id) != ciphertexts.end()) {
            return ciphertexts[id]->GetScalingFactor();
        }
        return 0.0;
    }

    bool SetCiphertextScale(int id, double scale) {
        if (ciphertexts.find(id) != ciphertexts.end()) {
            ciphertexts[id]->SetScalingFactor(scale);
            return true;
        }
        return false;
    }

    // Homomorphic operations
    int Add(int ct1Id, int ct2Id) {
        try {
            if (ciphertexts.find(ct1Id) == ciphertexts.end() || 
                ciphertexts.find(ct2Id) == ciphertexts.end()) return -1;
            
            Ciphertext<DCRTPoly> result = context->EvalAdd(ciphertexts[ct1Id], ciphertexts[ct2Id]);
            int id = nextCiphertextId++;
            ciphertexts[id] = result;
            return id;
        } catch (const std::exception& e) {
            return -1;
        }
    }

    int AddPlain(int ctId, int ptId) {
        try {
            if (ciphertexts.find(ctId) == ciphertexts.end() || 
                plaintexts.find(ptId) == plaintexts.end()) return -1;
            
            Ciphertext<DCRTPoly> result = context->EvalAdd(ciphertexts[ctId], plaintexts[ptId]);
            int id = nextCiphertextId++;
            ciphertexts[id] = result;
            return id;
        } catch (const std::exception& e) {
            return -1;
        }
    }

    int Multiply(int ct1Id, int ct2Id) {
        try {
            if (ciphertexts.find(ct1Id) == ciphertexts.end() || 
                ciphertexts.find(ct2Id) == ciphertexts.end()) return -1;
            
            Ciphertext<DCRTPoly> result = context->EvalMult(ciphertexts[ct1Id], ciphertexts[ct2Id]);
            result = context->Rescale(result);
            int id = nextCiphertextId++;
            ciphertexts[id] = result;
            return id;
        } catch (const std::exception& e) {
            return -1;
        }
    }

    int MultiplyPlain(int ctId, int ptId) {
        try {
            if (ciphertexts.find(ctId) == ciphertexts.end() || 
                plaintexts.find(ptId) == plaintexts.end()) return -1;
            
            Ciphertext<DCRTPoly> result = context->EvalMult(ciphertexts[ctId], plaintexts[ptId]);
            result = context->Rescale(result);
            int id = nextCiphertextId++;
            ciphertexts[id] = result;
            return id;
        } catch (const std::exception& e) {
            return -1;
        }
    }

    int Rotate(int ctId, int steps) {
        try {
            if (ciphertexts.find(ctId) == ciphertexts.end()) return -1;
            
            Ciphertext<DCRTPoly> result = context->EvalRotate(ciphertexts[ctId], steps);
            int id = nextCiphertextId++;
            ciphertexts[id] = result;
            return id;
        } catch (const std::exception& e) {
            return -1;
        }
    }

    int Rescale(int ctId) {
        try {
            if (ciphertexts.find(ctId) == ciphertexts.end()) return -1;
            
            Ciphertext<DCRTPoly> result = context->Rescale(ciphertexts[ctId]);
            int id = nextCiphertextId++;
            ciphertexts[id] = result;
            return id;
        } catch (const std::exception& e) {
            return -1;
        }
    }

    // Linear transforms
    int CreateLinearTransform(const std::vector<double>& transform) {
        int id = nextTransformId++;
        linTransforms[id] = transform;
        return id;
    }

    bool DeleteLinearTransform(int id) {
        return linTransforms.erase(id) > 0;
    }

    int ApplyLinearTransform(int ctId, int transformId) {
        try {
            if (ciphertexts.find(ctId) == ciphertexts.end() || 
                linTransforms.find(transformId) == linTransforms.end()) return -1;
            
            // Create plaintext from transform vector
            Plaintext transformPt = context->MakeCKKSPackedPlaintext(linTransforms[transformId]);
            
            // Apply transform (multiply)
            Ciphertext<DCRTPoly> result = context->EvalMult(ciphertexts[ctId], transformPt);
            result = context->Rescale(result);
            
            int id = nextCiphertextId++;
            ciphertexts[id] = result;
            return id;
        } catch (const std::exception& e) {
            return -1;
        }
    }

    // Utility functions
    std::vector<double> GetPlaintextValues(int id) {
        if (plaintexts.find(id) != plaintexts.end()) {
            std::vector<double> values = plaintexts[id]->GetRealPackedValue();
            return values;
        }
        return std::vector<double>();
    }

    void CleanUp() {
        plaintexts.clear();
        ciphertexts.clear();
        linTransforms.clear();
        nextPlaintextId = 0;
        nextCiphertextId = 0;
        nextTransformId = 0;
        initialized = false;
    }

    bool IsInitialized() const {
        return initialized;
    }
};

// Global scheme instance
static std::unique_ptr<OrionOpenFHEScheme> g_scheme;

extern "C" {
    // Scheme management
    void NewScheme(int logN, int* logQ, int logQSize, int* logP, int logPSize,
                   int logScale, int hammingWeight, const char* ringType,
                   const char* keysPath, const char* ioMode) {
        try {
            g_scheme = std::make_unique<OrionOpenFHEScheme>();

            std::vector<int> logQVec(logQ, logQ + logQSize);
            std::vector<int> logPVec(logP, logP + logPSize);

            bool success = g_scheme->InitializeScheme(logN, logQVec, logPVec, logScale, hammingWeight,
                                      0, // ringType placeholder
                                      std::string(keysPath ? keysPath : ""),
                                      std::string(ioMode ? ioMode : ""));

            if (!success) {
                g_scheme.reset(); // Clean up on failure
            }
        } catch (const std::exception& e) {
            g_scheme.reset(); // Clean up on exception
        }
    }

    void DeleteScheme() {
        if (g_scheme) {
            g_scheme->CleanUp();
            g_scheme.reset();
        }
    }

    int IsSchemeInitialized() {
        return (g_scheme && g_scheme->IsInitialized()) ? 1 : 0;
    }

    // Plaintext operations
    int CreatePlaintext(double* values, int size) {
        if (!g_scheme) return -1;
        std::vector<double> valVec(values, values + size);
        return g_scheme->CreatePlaintext(valVec);
    }

    void DeletePlaintext(int id) {
        if (g_scheme) g_scheme->DeletePlaintext(id);
    }

    double GetPlaintextScale(int id) {
        if (!g_scheme) return 0.0;
        return g_scheme->GetPlaintextScale(id);
    }

    void SetPlaintextScale(int id, double scale) {
        if (g_scheme) g_scheme->SetPlaintextScale(id, scale);
    }

    // Ciphertext operations
    int Encrypt(int ptId) {
        if (!g_scheme) return -1;
        return g_scheme->Encrypt(ptId);
    }

    int Decrypt(int ctId) {
        if (!g_scheme) return -1;
        return g_scheme->Decrypt(ctId);
    }

    void DeleteCiphertext(int id) {
        if (g_scheme) g_scheme->DeleteCiphertext(id);
    }

    double GetCiphertextScale(int id) {
        if (!g_scheme) return 0.0;
        return g_scheme->GetCiphertextScale(id);
    }

    void SetCiphertextScale(int id, double scale) {
        if (g_scheme) g_scheme->SetCiphertextScale(id, scale);
    }

    // Homomorphic operations
    int Add(int ct1Id, int ct2Id) {
        if (!g_scheme) return -1;
        return g_scheme->Add(ct1Id, ct2Id);
    }

    int AddPlain(int ctId, int ptId) {
        if (!g_scheme) return -1;
        return g_scheme->AddPlain(ctId, ptId);
    }

    int Multiply(int ct1Id, int ct2Id) {
        if (!g_scheme) return -1;
        return g_scheme->Multiply(ct1Id, ct2Id);
    }

    int MultiplyPlain(int ctId, int ptId) {
        if (!g_scheme) return -1;
        return g_scheme->MultiplyPlain(ctId, ptId);
    }

    int Rotate(int ctId, int steps) {
        if (!g_scheme) return -1;
        return g_scheme->Rotate(ctId, steps);
    }

    int Rescale(int ctId) {
        if (!g_scheme) return -1;
        return g_scheme->Rescale(ctId);
    }

    // Linear transform operations
    int CreateLinearTransform(double* transform, int size) {
        if (!g_scheme) return -1;
        std::vector<double> transformVec(transform, transform + size);
        return g_scheme->CreateLinearTransform(transformVec);
    }

    void DeleteLinearTransform(int id) {
        if (g_scheme) g_scheme->DeleteLinearTransform(id);
    }

    int ApplyLinearTransform(int ctId, int transformId) {
        if (!g_scheme) return -1;
        return g_scheme->ApplyLinearTransform(ctId, transformId);
    }

    // Utility functions
    int GetPlaintextValues(int id, double* output, int maxSize) {
        if (!g_scheme) return 0;
        std::vector<double> values = g_scheme->GetPlaintextValues(id);
        int copySize = std::min(maxSize, static_cast<int>(values.size()));
        std::copy(values.begin(), values.begin() + copySize, output);
        return copySize;
    }

    void FreeCArray(void* ptr) {
        free(ptr);
    }
}



