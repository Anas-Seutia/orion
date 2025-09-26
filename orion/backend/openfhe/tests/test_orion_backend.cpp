#include "orion_openfhe.hpp"
#include <iostream>
#include <vector>
#include <cassert>
#include <iomanip>

/**
 * @brief Test program for the modular OpenFHE Orion backend
 * 
 * This program tests all components of the modular OpenFHE backend:
 * - Scheme initialization
 * - Encoding/decoding
 * - Encryption/decryption
 * - Homomorphic operations
 * - Linear transformations
 * - Memory management
 */

void test_basic_operations() {
    std::cout << "\n=== Testing Basic Operations ===" << std::endl;
    
    // Test parameters (matching typical CKKS parameters)
    int logN = 13;  // Ring dimension 2^13 = 8192
    std::vector<int> logQ = {60, 40, 40, 60};  // Coefficient modulus
    std::vector<int> logP = {60};  // Auxiliary modulus
    int logScale = 40;
    int hammingWeight = 64;
    std::string ringType = "standard";
    std::string keysPath = "./keys";
    std::string ioMode = "memory";

    // Initialize scheme
    std::cout << "Initializing scheme..." << std::endl;
    bool success = g_scheme.Initialize(logN, logQ, logP, logScale, hammingWeight, 
                                      0, keysPath, ioMode);
    assert(success && "Scheme initialization failed");
    
    success = g_scheme.GenerateKeys();
    assert(success && "Key generation failed");
    
    // Initialize components
    std::cout << "Initializing encoder..." << std::endl;
    success = g_encoder.Initialize();
    assert(success && "Encoder initialization failed");
    
    std::cout << "Initializing encryptor/decryptor..." << std::endl;
    success = g_encryptor.Initialize();
    assert(success && "Encryptor initialization failed");
    
    success = g_decryptor.Initialize();
    assert(success && "Decryptor initialization failed");
    
    std::cout << "Basic components initialized successfully!" << std::endl;
    
    // Test values
    std::vector<double> values1 = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0};
    std::vector<double> values2 = {2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0};
    
    std::cout << "Test values 1: ";
    for (double v : values1) std::cout << v << " ";
    std::cout << std::endl;
    
    // Test encoding
    std::cout << "Testing encoding..." << std::endl;
    int pt1_id = g_encoder.EncodeAtLevel(values1, 0);
    int pt2_id = g_encoder.EncodeAtLevel(values2, 0);
    assert(pt1_id >= 0 && "Encoding failed");
    assert(pt2_id >= 0 && "Encoding failed");
    
    // Test decoding
    std::vector<double> decoded1 = g_encoder.Decode(pt1_id);
    std::cout << "Decoded values 1: ";
    for (size_t i = 0; i < std::min(decoded1.size(), values1.size()); ++i) {
        std::cout << std::fixed << std::setprecision(3) << decoded1[i] << " ";
    }
    std::cout << std::endl;
    
    // Test encryption
    std::cout << "Testing encryption..." << std::endl;
    int ct1_id = g_encryptor.Encrypt(pt1_id);
    int ct2_id = g_encryptor.Encrypt(pt2_id);
    assert(ct1_id >= 0 && "Encryption failed");
    assert(ct2_id >= 0 && "Encryption failed");
    
    // Test decryption
    std::cout << "Testing decryption..." << std::endl;
    int decrypted_pt_id = g_decryptor.Decrypt(ct1_id);
    assert(decrypted_pt_id >= 0 && "Decryption failed");
    
    std::vector<double> decrypted_values = g_encoder.Decode(decrypted_pt_id);
    std::cout << "Decrypted values: ";
    for (size_t i = 0; i < std::min(decrypted_values.size(), values1.size()); ++i) {
        std::cout << std::fixed << std::setprecision(3) << decrypted_values[i] << " ";
    }
    std::cout << std::endl;
    
    // Clean up
    DeletePlaintext(pt1_id);
    DeletePlaintext(pt2_id);
    DeletePlaintext(decrypted_pt_id);
    DeleteCiphertext(ct1_id);
    DeleteCiphertext(ct2_id);
    
    std::cout << "Basic operations test completed successfully!" << std::endl;
}

void test_homomorphic_operations() {
    std::cout << "\n=== Testing Homomorphic Operations ===" << std::endl;
    
    OrionOpenFHEBackend backend;
    
    // Initialize backend
    std::vector<int> logQ = {60, 40, 40, 60};
    std::vector<int> logP = {60};
    bool success = backend.Initialize(13, logQ, logP, 40, 64, "standard", "./keys", "memory");
    assert(success && "Backend initialization failed");
    
    // Test values
    std::vector<double> values1 = {1.5, 2.5, 3.5, 4.5};
    std::vector<double> values2 = {0.5, 1.0, 1.5, 2.0};
    
    std::cout << "Values 1: ";
    for (double v : values1) std::cout << v << " ";
    std::cout << std::endl;
    
    std::cout << "Values 2: ";
    for (double v : values2) std::cout << v << " ";
    std::cout << std::endl;
    
    // Encrypt values
    int ct1_id = backend.EncodeAndEncrypt(values1);
    int ct2_id = backend.EncodeAndEncrypt(values2);
    assert(ct1_id >= 0 && "Encryption 1 failed");
    assert(ct2_id >= 0 && "Encryption 2 failed");
    
    // Test addition
    std::cout << "Testing homomorphic addition..." << std::endl;
    int ct_add_id = backend.Add(ct1_id, ct2_id);
    assert(ct_add_id >= 0 && "Addition failed");
    
    std::vector<double> result_add = backend.DecryptAndDecode(ct_add_id);
    std::cout << "Addition result: ";
    for (size_t i = 0; i < std::min(result_add.size(), values1.size()); ++i) {
        std::cout << std::fixed << std::setprecision(3) << result_add[i] << " ";
    }
    std::cout << std::endl;
    
    // Test multiplication
    std::cout << "Testing homomorphic multiplication..." << std::endl;
    int ct_mult_id = backend.Multiply(ct1_id, ct2_id);
    assert(ct_mult_id >= 0 && "Multiplication failed");
    
    std::vector<double> result_mult = backend.DecryptAndDecode(ct_mult_id);
    std::cout << "Multiplication result: ";
    for (size_t i = 0; i < std::min(result_mult.size(), values1.size()); ++i) {
        std::cout << std::fixed << std::setprecision(3) << result_mult[i] << " ";
    }
    std::cout << std::endl;
    
    // Test rotation
    std::cout << "Testing rotation..." << std::endl;
    int ct_rot_id = backend.Rotate(ct1_id, 1);  // Rotate by 1 position
    assert(ct_rot_id >= 0 && "Rotation failed");
    
    std::vector<double> result_rot = backend.DecryptAndDecode(ct_rot_id);
    std::cout << "Rotation result: ";
    for (size_t i = 0; i < std::min(result_rot.size(), values1.size()); ++i) {
        std::cout << std::fixed << std::setprecision(3) << result_rot[i] << " ";
    }
    std::cout << std::endl;
    
    // Test rescaling
    std::cout << "Testing rescaling..." << std::endl;
    int ct_rescaled_id = backend.Rescale(ct_mult_id);
    assert(ct_rescaled_id >= 0 && "Rescaling failed");
    
    std::vector<double> result_rescaled = backend.DecryptAndDecode(ct_rescaled_id);
    std::cout << "Rescaled result: ";
    for (size_t i = 0; i < std::min(result_rescaled.size(), values1.size()); ++i) {
        std::cout << std::fixed << std::setprecision(3) << result_rescaled[i] << " ";
    }
    std::cout << std::endl;
    
    std::cout << "Homomorphic operations test completed successfully!" << std::endl;
}

void test_linear_transformations() {
    std::cout << "\n=== Testing Linear Transformations ===" << std::endl;
    
    OrionOpenFHEBackend backend;
    
    // Initialize backend
    std::vector<int> logQ = {60, 40, 40, 60};
    std::vector<int> logP = {60};
    bool success = backend.Initialize(13, logQ, logP, 40, 64, "standard", "./keys", "memory");
    assert(success && "Backend initialization failed");
    
    // Test values
    std::vector<double> values = {1.0, 2.0, 3.0, 4.0};
    std::vector<double> transform = {2.0, 0.5, 0.25, 3.0};  // Simple transformation
    
    std::cout << "Original values: ";
    for (double v : values) std::cout << v << " ";
    std::cout << std::endl;
    
    std::cout << "Transform matrix: ";
    for (double v : transform) std::cout << v << " ";
    std::cout << std::endl;
    
    // Encrypt values
    int ct_id = backend.EncodeAndEncrypt(values);
    assert(ct_id >= 0 && "Encryption failed");
    
    // Create and apply linear transformation
    int transform_id = backend.CreateLinearTransform(transform);
    assert(transform_id >= 0 && "Transform creation failed");
    
    int ct_transformed_id = backend.ApplyLinearTransform(ct_id, transform_id);
    assert(ct_transformed_id >= 0 && "Transform application failed");
    
    // Decrypt and check result
    std::vector<double> result = backend.DecryptAndDecode(ct_transformed_id);
    std::cout << "Transformed result: ";
    for (size_t i = 0; i < std::min(result.size(), values.size()); ++i) {
        std::cout << std::fixed << std::setprecision(3) << result[i] << " ";
    }
    std::cout << std::endl;
    
    // Clean up transform
    bool deleted = backend.DeleteLinearTransform(transform_id);
    assert(deleted && "Transform deletion failed");
    
    std::cout << "Linear transformations test completed successfully!" << std::endl;
}

void test_memory_management() {
    std::cout << "\n=== Testing Memory Management ===" << std::endl;
    
    // Test heap allocator directly
    HeapAllocator heap;
    
    // Test adding and retrieving objects
    std::vector<double> test_vec1 = {1.0, 2.0, 3.0};
    std::vector<double> test_vec2 = {4.0, 5.0, 6.0};
    
    int id1 = heap.Add(test_vec1);
    int id2 = heap.Add(test_vec2);
    
    std::cout << "Added vectors with IDs: " << id1 << ", " << id2 << std::endl;
    
    // Retrieve and verify
    auto& retrieved1 = heap.Retrieve<std::vector<double>>(id1);
    auto& retrieved2 = heap.Retrieve<std::vector<double>>(id2);
    
    assert(retrieved1.size() == test_vec1.size() && "Retrieved vector 1 size mismatch");
    assert(retrieved2.size() == test_vec2.size() && "Retrieved vector 2 size mismatch");
    
    for (size_t i = 0; i < test_vec1.size(); ++i) {
        assert(retrieved1[i] == test_vec1[i] && "Retrieved vector 1 content mismatch");
    }
    
    std::cout << "Memory retrieval test passed!" << std::endl;
    
    // Test deletion and reuse
    bool deleted1 = heap.Delete(id1);
    assert(deleted1 && "Deletion failed");
    
    // Add another object - should reuse the freed ID
    std::vector<double> test_vec3 = {7.0, 8.0, 9.0};
    int id3 = heap.Add(test_vec3);
    
    std::cout << "Added new vector with ID: " << id3 << " (should reuse freed ID)" << std::endl;
    
    // Get live keys
    std::vector<int> liveKeys = heap.GetLiveKeys();
    std::cout << "Live object IDs: ";
    for (int id : liveKeys) std::cout << id << " ";
    std::cout << std::endl;
    
    std::cout << "Total live objects: " << heap.Size() << std::endl;
    
    // Test reset
    heap.Reset();
    std::cout << "After reset, live objects: " << heap.Size() << std::endl;
    
    std::cout << "Memory management test completed successfully!" << std::endl;
}

void test_error_handling() {
    std::cout << "\n=== Testing Error Handling ===" << std::endl;
    
    // Test operations on uninitialized backend
    OrionOpenFHEBackend backend;
    
    std::vector<double> test_values = {1.0, 2.0, 3.0};
    
    // These should fail gracefully
    int ct_id = backend.EncodeAndEncrypt(test_values);
    std::cout << "Encrypt on uninitialized backend returned: " << ct_id << " (should be -1)" << std::endl;
    assert(ct_id == -1 && "Should fail on uninitialized backend");
    
    // Test invalid IDs
    std::vector<int> logQ = {60, 40, 40, 60};
    std::vector<int> logP = {60};
    bool success = backend.Initialize(13, logQ, logP, 40, 64, "standard", "./keys", "memory");
    assert(success && "Backend initialization failed");
    
    // Try operations with invalid IDs
    int invalid_result = backend.Add(999, 1000);  // Invalid ciphertext IDs
    std::cout << "Add with invalid IDs returned: " << invalid_result << " (should be -1)" << std::endl;
    assert(invalid_result == -1 && "Should fail with invalid IDs");
    
    // Test decoder with invalid ID
    std::vector<double> decoded = backend.DecryptAndDecode(999);
    std::cout << "Decrypt with invalid ID returned " << decoded.size() << " values (should be 0)" << std::endl;
    assert(decoded.empty() && "Should return empty vector for invalid ID");
    
    std::cout << "Error handling test completed successfully!" << std::endl;
}

int main() {
    std::cout << "====== OpenFHE Orion Backend Test Suite ======" << std::endl;
    
    try {
        // Run all tests
        test_memory_management();
        test_basic_operations();
        test_homomorphic_operations();
        test_linear_transformations();
        test_error_handling();
        
        std::cout << "\n====== All Tests Passed Successfully! ======" << std::endl;
        
        // Clean up global state
        g_scheme.CleanUp();
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "\nTest failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "\nTest failed with unknown exception" << std::endl;
        return 1;
    }
}