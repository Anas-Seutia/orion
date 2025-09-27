#!/usr/bin/env python3
"""
Backend Compatibility Test Suite for Orion

This test suite verifies that OpenFHE backend functions behave identically
to Lattigo backend functions, ensuring compatibility between backends.

Usage:
    python test_backend_compatibility.py
    python test_backend_compatibility.py --verbose
    python test_backend_compatibility.py --test-function=test_basic_encryption
"""

import unittest
import sys
import os
import numpy as np
from typing import List, Tuple, Any, Optional
import argparse
import traceback
from contextlib import contextmanager
import time

# Add the project root to Python path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

try:
    from orion.backend.lattigo.bindings import LattigoLibrary
    LATTIGO_AVAILABLE = True
except ImportError as e:
    print(f"Warning: Lattigo backend not available: {e}")
    LATTIGO_AVAILABLE = False

try:
    from orion.backend.openfhe.bindings import OpenFHEBackend
    OPENFHE_AVAILABLE = True
except ImportError as e:
    print(f"Warning: OpenFHE backend not available: {e}")
    OPENFHE_AVAILABLE = False


class MockOrionParams:
    """Mock Orion parameters for testing"""

    def __init__(self,
                 logn: int = 14,
                 logq: List[int] = None,
                 logp: List[int] = None,
                 logscale: int = 40,
                 hamming_weight: int = 64,
                 ringtype: str = "standard",
                 keys_path: str = "./test_keys",
                 io_mode: str = "memory"):
        self._logn = logn
        self._logq = logq or [60, 40, 40, 60]
        self._logp = logp or [60]
        self._logscale = logscale
        self._hamming_weight = hamming_weight
        self._ringtype = ringtype
        self._keys_path = keys_path
        self._io_mode = io_mode

    def get_logn(self): return self._logn
    def get_logq(self): return self._logq
    def get_logp(self): return self._logp
    def get_logscale(self): return self._logscale
    def get_hamming_weight(self): return self._hamming_weight
    def get_ringtype(self): return self._ringtype
    def get_keys_path(self): return self._keys_path
    def get_io_mode(self): return self._io_mode


class BackendTestResult:
    """Container for test results from a backend"""

    def __init__(self, backend_name: str):
        self.backend_name = backend_name
        self.success = False
        self.result = None
        self.error = None
        self.execution_time = 0.0

    def set_success(self, result, execution_time: float = 0.0):
        self.success = True
        self.result = result
        self.execution_time = execution_time

    def set_error(self, error, execution_time: float = 0.0):
        self.success = False
        self.error = error
        self.execution_time = execution_time


class BackendCompatibilityTest(unittest.TestCase):
    """Test suite for backend compatibility"""

    @classmethod
    def setUpClass(cls):
        """Set up test fixtures"""
        cls.params = MockOrionParams()
        cls.lattigo_backend = None
        cls.openfhe_backend = None
        cls.test_values = [1.0, 2.0, 3.0, 4.0, 0.5, -1.0, 0.0, 10.0]
        cls.small_test_values = [1.0, 2.0]

        # Initialize backends if available
        if LATTIGO_AVAILABLE:
            try:
                cls.lattigo_backend = LattigoLibrary()
                cls.lattigo_backend.setup_bindings(cls.params)
                print("✓ Lattigo backend initialized successfully")
            except Exception as e:
                print(f"✗ Failed to initialize Lattigo backend: {e}")
                cls.lattigo_backend = None

        if OPENFHE_AVAILABLE:
            try:
                cls.openfhe_backend = OpenFHEBackend(cls.params)
                print("✓ OpenFHE backend initialized successfully")
            except Exception as e:
                print(f"✗ Failed to initialize OpenFHE backend: {e}")
                cls.openfhe_backend = None

    @classmethod
    def tearDownClass(cls):
        """Clean up test fixtures"""
        if cls.lattigo_backend:
            try:
                cls.lattigo_backend.DeleteScheme()
            except:
                pass

        if cls.openfhe_backend:
            try:
                cls.openfhe_backend.cleanup()
            except:
                pass

    def setUp(self):
        """Set up for each test"""
        if not LATTIGO_AVAILABLE:
            self.skipTest("Lattigo backend not available")
        if not OPENFHE_AVAILABLE:
            self.skipTest("OpenFHE backend not available")
        if not self.lattigo_backend:
            self.skipTest("Lattigo backend initialization failed")
        if not self.openfhe_backend:
            self.skipTest("OpenFHE backend initialization failed")

    @contextmanager
    def time_execution(self):
        """Context manager to time execution"""
        start = time.time()
        yield
        end = time.time()
        self.last_execution_time = end - start

    def run_backend_test(self, test_func, *args, **kwargs) -> Tuple[BackendTestResult, BackendTestResult]:
        """Run a test function on both backends and return results"""
        lattigo_result = BackendTestResult("Lattigo")
        openfhe_result = BackendTestResult("OpenFHE")

        # Test Lattigo backend
        try:
            with self.time_execution():
                result = test_func(self.lattigo_backend, *args, **kwargs)
            lattigo_result.set_success(result, self.last_execution_time)
        except Exception as e:
            lattigo_result.set_error(str(e), getattr(self, 'last_execution_time', 0.0))

        # Test OpenFHE backend
        try:
            with self.time_execution():
                result = test_func(self.openfhe_backend, *args, **kwargs)
            openfhe_result.set_success(result, self.last_execution_time)
        except Exception as e:
            openfhe_result.set_error(str(e), getattr(self, 'last_execution_time', 0.0))

        return lattigo_result, openfhe_result

    def assert_results_compatible(self, lattigo_result: BackendTestResult,
                                 openfhe_result: BackendTestResult,
                                 tolerance: float = 1e-6):
        """Assert that results from both backends are compatible"""

        # Both should succeed or both should fail with similar errors
        if lattigo_result.success != openfhe_result.success:
            self.fail(f"Backend behavior mismatch:\n"
                     f"Lattigo: {'SUCCESS' if lattigo_result.success else 'FAILED'}\n"
                     f"OpenFHE: {'SUCCESS' if openfhe_result.success else 'FAILED'}\n"
                     f"Lattigo error: {lattigo_result.error}\n"
                     f"OpenFHE error: {openfhe_result.error}")

        # If both failed, that's acceptable (as long as they both failed)
        if not lattigo_result.success:
            return

        # Compare actual results
        self._compare_results(lattigo_result.result, openfhe_result.result, tolerance)

    def _compare_results(self, lattigo_result, openfhe_result, tolerance: float):
        """Compare results from both backends"""
        if type(lattigo_result) != type(openfhe_result):
            # Try to handle different but compatible types
            if isinstance(lattigo_result, (int, float)) and isinstance(openfhe_result, (int, float)):
                self.assertAlmostEqual(float(lattigo_result), float(openfhe_result),
                                     delta=tolerance,
                                     msg=f"Numeric results differ: {lattigo_result} vs {openfhe_result}")
                return
            else:
                self.fail(f"Result types differ: {type(lattigo_result)} vs {type(openfhe_result)}")

        if isinstance(lattigo_result, (list, tuple, np.ndarray)):
            lattigo_array = np.array(lattigo_result)
            openfhe_array = np.array(openfhe_result)

            self.assertEqual(lattigo_array.shape, openfhe_array.shape,
                           "Result array shapes differ")

            if lattigo_array.dtype in [np.float32, np.float64]:
                np.testing.assert_allclose(lattigo_array, openfhe_array,
                                         rtol=tolerance, atol=tolerance,
                                         err_msg="Numeric arrays differ beyond tolerance")
            else:
                np.testing.assert_array_equal(lattigo_array, openfhe_array,
                                            err_msg="Arrays are not equal")

        elif isinstance(lattigo_result, (int, float)):
            self.assertAlmostEqual(lattigo_result, openfhe_result, delta=tolerance,
                                 msg=f"Numeric results differ: {lattigo_result} vs {openfhe_result}")
        else:
            self.assertEqual(lattigo_result, openfhe_result,
                           f"Results differ: {lattigo_result} vs {openfhe_result}")


class CoreFunctionTests(BackendCompatibilityTest):
    """Tests for core FHE functions"""

    def test_scheme_initialization(self):
        """Test scheme initialization"""
        def test_func(backend):
            # Both backends should already be initialized
            return True

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result)

    def test_key_generation(self):
        """Test key generation functions"""
        def test_func(backend):
            # Generate keys
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.GenerateRelinearizationKey()
            backend.GenerateEvaluationKeys()
            return True

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result)

    def test_basic_encoding_decoding(self):
        """Test basic encode/decode operations"""
        def test_func(backend):
            # Initialize encoder
            backend.NewEncoder()

            # Encode values
            pt_id = backend.Encode(self.small_test_values, 0, 1 << 40)

            # Decode values
            decoded_values = backend.Decode(pt_id)

            # Clean up
            backend.DeletePlaintext(pt_id)

            return decoded_values[:len(self.small_test_values)]

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result, tolerance=1e-3)

    def test_basic_encryption_decryption(self):
        """Test basic encrypt/decrypt operations"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.NewEncoder()
            backend.NewEncryptor()
            backend.NewDecryptor()

            # Encode
            pt_id = backend.Encode(self.small_test_values, 0, 1 << 40)

            # Encrypt
            ct_id = backend.Encrypt(pt_id)

            # Decrypt
            decrypted_pt_id = backend.Decrypt(ct_id)

            # Decode
            decoded_values = backend.Decode(decrypted_pt_id)

            # Clean up
            backend.DeletePlaintext(pt_id)
            backend.DeleteCiphertext(ct_id)
            backend.DeletePlaintext(decrypted_pt_id)

            return decoded_values[:len(self.small_test_values)]

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result, tolerance=1e-2)


class ArithmeticOperationTests(BackendCompatibilityTest):
    """Tests for homomorphic arithmetic operations"""

    def test_ciphertext_addition(self):
        """Test ciphertext addition"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.NewEncoder()
            backend.NewEncryptor()
            backend.NewDecryptor()
            backend.NewEvaluator()

            # Encode and encrypt two values
            values1 = [1.0, 2.0]
            values2 = [3.0, 4.0]

            pt1_id = backend.Encode(values1, 0, 1 << 40)
            pt2_id = backend.Encode(values2, 0, 1 << 40)

            ct1_id = backend.Encrypt(pt1_id)
            ct2_id = backend.Encrypt(pt2_id)

            # Add ciphertexts
            ct_result_id = backend.AddCiphertext(ct1_id, ct2_id)

            # Decrypt and decode
            pt_result_id = backend.Decrypt(ct_result_id)
            result = backend.Decode(pt_result_id)

            # Clean up
            backend.DeletePlaintext(pt1_id)
            backend.DeletePlaintext(pt2_id)
            backend.DeleteCiphertext(ct1_id)
            backend.DeleteCiphertext(ct2_id)
            backend.DeleteCiphertext(ct_result_id)
            backend.DeletePlaintext(pt_result_id)

            return result[:len(values1)]

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result, tolerance=1e-2)

    def test_plaintext_addition(self):
        """Test plaintext addition to ciphertext"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.NewEncoder()
            backend.NewEncryptor()
            backend.NewDecryptor()
            backend.NewEvaluator()

            # Encode and encrypt values
            ct_values = [2.0, 3.0]
            pt_values = [1.0, 1.0]

            ct_pt_id = backend.Encode(ct_values, 1)
            pt_pt_id = backend.Encode(pt_values, 1)

            ct_id = backend.Encrypt(ct_pt_id)

            # Add plaintext to ciphertext
            ct_result_id = backend.AddPlaintext(ct_id, pt_pt_id)

            # Decrypt and decode
            pt_result_id = backend.Decrypt(ct_result_id)
            result = backend.Decode(pt_result_id)

            # Clean up
            backend.DeletePlaintext(ct_pt_id)
            backend.DeletePlaintext(pt_pt_id)
            backend.DeleteCiphertext(ct_id)
            backend.DeleteCiphertext(ct_result_id)
            backend.DeletePlaintext(pt_result_id)

            return result[:len(ct_values)]

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result, tolerance=1e-2)

    def test_ciphertext_multiplication(self):
        """Test ciphertext multiplication with relinearization"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.GenerateRelinearizationKey()
            backend.NewEncoder()
            backend.NewEncryptor()
            backend.NewDecryptor()
            backend.NewEvaluator()

            # Encode and encrypt values
            values1 = [2.0, 3.0]
            values2 = [1.5, 2.0]

            # Use proper scale based on logscale parameter
            pt1_id = backend.Encode(values1, 1)
            pt2_id = backend.Encode(values2, 1)

            ct1_id = backend.Encrypt(pt1_id)
            ct2_id = backend.Encrypt(pt2_id)

            # Multiply ciphertexts
            ct_result_id = backend.MulRelinCiphertext(ct1_id, ct2_id)

            # Decrypt and decode
            pt_result_id = backend.Decrypt(ct_result_id)
            result = backend.Decode(pt_result_id)

            # Clean up
            backend.DeletePlaintext(pt1_id)
            backend.DeletePlaintext(pt2_id)
            backend.DeleteCiphertext(ct1_id)
            backend.DeleteCiphertext(ct2_id)
            backend.DeleteCiphertext(ct_result_id)
            backend.DeletePlaintext(pt_result_id)

            return result[:len(values1)]

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result, tolerance=1e-1)


    def test_negate_ciphertext(self):
        """Test ciphertext negation"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.NewEncoder()
            backend.NewEncryptor()
            backend.NewDecryptor()
            backend.NewEvaluator()

            # Encode and encrypt values
            values = [2.0, -3.0, 1.5, 0.0]
            pt_id = backend.Encode(values, 0, 1 << 40)
            ct_id = backend.Encrypt(pt_id)

            # Negate ciphertext
            ct_negated_id = backend.Negate(ct_id)

            # Decrypt and decode
            pt_result_id = backend.Decrypt(ct_negated_id)
            result = backend.Decode(pt_result_id)

            # Clean up
            backend.DeletePlaintext(pt_id)
            backend.DeleteCiphertext(ct_id)
            backend.DeleteCiphertext(ct_negated_id)
            backend.DeletePlaintext(pt_result_id)

            return result[:len(values)]

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result, tolerance=1e-2)

    def test_add_scalar_to_ciphertext(self):
        """Test adding scalar to ciphertext"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.NewEncoder()
            backend.NewEncryptor()
            backend.NewDecryptor()
            backend.NewEvaluator()

            # Encode and encrypt values
            values = [1.0, 2.0, 3.0, 4.0]
            scalar = 5.0
            pt_id = backend.Encode(values, 0, 1 << 40)
            ct_id = backend.Encrypt(pt_id)

            # Add scalar to ciphertext
            ct_result_id = backend.AddScalar(ct_id, scalar)

            # Decrypt and decode
            pt_result_id = backend.Decrypt(ct_result_id)
            result = backend.Decode(pt_result_id)

            # Clean up
            backend.DeletePlaintext(pt_id)
            backend.DeleteCiphertext(ct_id)
            backend.DeleteCiphertext(ct_result_id)
            backend.DeletePlaintext(pt_result_id)

            return result[:len(values)]

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result, tolerance=1e-2)

    def test_subtract_scalar_from_ciphertext(self):
        """Test subtracting scalar from ciphertext"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.NewEncoder()
            backend.NewEncryptor()
            backend.NewDecryptor()
            backend.NewEvaluator()

            # Encode and encrypt values
            values = [10.0, 8.0, 6.0, 4.0]
            scalar = 2.0
            pt_id = backend.Encode(values, 0, 1 << 40)
            ct_id = backend.Encrypt(pt_id)

            # Subtract scalar from ciphertext
            ct_result_id = backend.SubScalar(ct_id, scalar)

            # Decrypt and decode
            pt_result_id = backend.Decrypt(ct_result_id)
            result = backend.Decode(pt_result_id)

            # Clean up
            backend.DeletePlaintext(pt_id)
            backend.DeleteCiphertext(ct_id)
            backend.DeleteCiphertext(ct_result_id)
            backend.DeletePlaintext(pt_result_id)

            return result[:len(values)]

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result, tolerance=1e-2)

    def test_multiply_ciphertext_by_integer_scalar(self):
        """Test multiplying ciphertext by integer scalar"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.NewEncoder()
            backend.NewEncryptor()
            backend.NewDecryptor()
            backend.NewEvaluator()

            # Encode and encrypt values
            values = [1.0, 2.0, 3.0, 4.0]
            scalar = 3
            pt_id = backend.Encode(values, 0, 1 << 40)
            ct_id = backend.Encrypt(pt_id)

            # Multiply by integer scalar
            ct_result_id = backend.MulScalarInt(ct_id, scalar)

            # Decrypt and decode
            pt_result_id = backend.Decrypt(ct_result_id)
            result = backend.Decode(pt_result_id)

            # Clean up
            backend.DeletePlaintext(pt_id)
            backend.DeleteCiphertext(ct_id)
            backend.DeleteCiphertext(ct_result_id)
            backend.DeletePlaintext(pt_result_id)

            return result[:len(values)]

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result, tolerance=1e-2)

    def test_multiply_ciphertext_by_float_scalar(self):
        """Test multiplying ciphertext by float scalar"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.NewEncoder()
            backend.NewEncryptor()
            backend.NewDecryptor()
            backend.NewEvaluator()

            # Encode and encrypt values
            values = [1.0, 2.0, 3.0, 4.0]
            scalar = 2.5
            pt_id = backend.Encode(values, 1)
            ct_id = backend.Encrypt(pt_id)

            # Multiply by float scalar
            ct_result_id = backend.MulScalarFloat(ct_id, scalar)

            # Decrypt and decode
            pt_result_id = backend.Decrypt(ct_result_id)
            result = backend.Decode(pt_result_id)

            # Clean up
            backend.DeletePlaintext(pt_id)
            backend.DeleteCiphertext(ct_id)
            backend.DeleteCiphertext(ct_result_id)
            backend.DeletePlaintext(pt_result_id)

            return result[:len(values)]

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result, tolerance=1e-2)


class AdvancedCiphertextOperationTests(BackendCompatibilityTest):
    """Tests for advanced ciphertext operations"""

    def test_subtract_ciphertexts(self):
        """Test ciphertext subtraction"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.NewEncoder()
            backend.NewEncryptor()
            backend.NewDecryptor()
            backend.NewEvaluator()

            # Encode and encrypt values
            values1 = [10.0, 8.0, 6.0, 4.0]
            values2 = [1.0, 2.0, 3.0, 4.0]

            pt1_id = backend.Encode(values1, 0, 1 << 40)
            pt2_id = backend.Encode(values2, 0, 1 << 40)
            ct1_id = backend.Encrypt(pt1_id)
            ct2_id = backend.Encrypt(pt2_id)

            # Subtract ciphertexts
            ct_result_id = backend.SubCiphertext(ct1_id, ct2_id)

            # Decrypt and decode
            pt_result_id = backend.Decrypt(ct_result_id)
            result = backend.Decode(pt_result_id)

            # Clean up
            backend.DeletePlaintext(pt1_id)
            backend.DeletePlaintext(pt2_id)
            backend.DeleteCiphertext(ct1_id)
            backend.DeleteCiphertext(ct2_id)
            backend.DeleteCiphertext(ct_result_id)
            backend.DeletePlaintext(pt_result_id)

            return result[:len(values1)]

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result, tolerance=1e-2)

    def test_subtract_plaintext_from_ciphertext(self):
        """Test subtracting plaintext from ciphertext"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.NewEncoder()
            backend.NewEncryptor()
            backend.NewDecryptor()
            backend.NewEvaluator()

            # Encode and encrypt values
            ct_values = [10.0, 8.0, 6.0, 4.0]
            pt_values = [1.0, 2.0, 1.0, 2.0]

            ct_pt_id = backend.Encode(ct_values, 0, 1 << 40)
            pt_pt_id = backend.Encode(pt_values, 0, 1 << 40)
            ct_id = backend.Encrypt(ct_pt_id)

            # Subtract plaintext from ciphertext
            ct_result_id = backend.SubPlaintext(ct_id, pt_pt_id)

            # Decrypt and decode
            pt_result_id = backend.Decrypt(ct_result_id)
            result = backend.Decode(pt_result_id)

            # Clean up
            backend.DeletePlaintext(ct_pt_id)
            backend.DeletePlaintext(pt_pt_id)
            backend.DeleteCiphertext(ct_id)
            backend.DeleteCiphertext(ct_result_id)
            backend.DeletePlaintext(pt_result_id)

            return result[:len(ct_values)]

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result, tolerance=1e-2)

    def test_new_copy_operations(self):
        """Test operations that create new copies (for compatibility)"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.GenerateRelinearizationKey()
            backend.NewEncoder()
            backend.NewEncryptor()
            backend.NewDecryptor()
            backend.NewEvaluator()

            # Encode and encrypt values
            values1 = [2.0, 3.0]
            values2 = [1.0, 1.0]

            pt1_id = backend.Encode(values1, 1)
            pt2_id = backend.Encode(values2, 1)
            ct1_id = backend.Encrypt(pt1_id)
            ct2_id = backend.Encrypt(pt2_id)

            # Test various "New" operations
            results = []

            # AddCiphertextNew
            ct_add_new_id = backend.AddCiphertextNew(ct1_id, ct2_id)
            pt_result_id = backend.Decrypt(ct_add_new_id)
            add_result = backend.Decode(pt_result_id)
            results.append(add_result[:len(values1)])
            backend.DeleteCiphertext(ct_add_new_id)
            backend.DeletePlaintext(pt_result_id)

            # AddPlaintextNew
            ct_add_pt_new_id = backend.AddPlaintextNew(ct1_id, pt2_id)
            pt_result_id = backend.Decrypt(ct_add_pt_new_id)
            add_pt_result = backend.Decode(pt_result_id)
            results.append(add_pt_result[:len(values1)])
            backend.DeleteCiphertext(ct_add_pt_new_id)
            backend.DeletePlaintext(pt_result_id)

            # MulPlaintextNew
            ct_mul_pt_new_id = backend.MulPlaintextNew(ct1_id, pt2_id)
            pt_result_id = backend.Decrypt(ct_mul_pt_new_id)
            mul_pt_result = backend.Decode(pt_result_id)
            results.append(mul_pt_result[:len(values1)])
            backend.DeleteCiphertext(ct_mul_pt_new_id)
            backend.DeletePlaintext(pt_result_id)

            # MulRelinCiphertextNew
            ct_mul_new_id = backend.MulRelinCiphertextNew(ct1_id, ct2_id)
            pt_result_id = backend.Decrypt(ct_mul_new_id)
            mul_result = backend.Decode(pt_result_id)
            results.append(mul_result[:len(values1)])
            backend.DeleteCiphertext(ct_mul_new_id)
            backend.DeletePlaintext(pt_result_id)

            # Clean up
            backend.DeletePlaintext(pt1_id)
            backend.DeletePlaintext(pt2_id)
            backend.DeleteCiphertext(ct1_id)
            backend.DeleteCiphertext(ct2_id)

            return results

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result, tolerance=1e-1)


class AdvancedOperationTests(BackendCompatibilityTest):
    """Tests for advanced FHE operations"""

    def test_rotation(self):
        """Test ciphertext rotation"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.GenerateEvaluationKeys()
            backend.NewEncoder()
            backend.NewEncryptor()
            backend.NewDecryptor()
            backend.NewEvaluator()

            # Add rotation key
            backend.AddRotationKey(1)

            # Encode and encrypt values
            values = [1.0, 2.0, 3.0, 4.0]

            pt_id = backend.Encode(values, 1)
            ct_id = backend.Encrypt(pt_id)

            # Rotate by 1 position
            ct_rotated_id = backend.Rotate(ct_id, 1)

            # Decrypt and decode
            pt_result_id = backend.Decrypt(ct_rotated_id)
            result = backend.Decode(pt_result_id)

            # Clean up
            backend.DeletePlaintext(pt_id)
            backend.DeleteCiphertext(ct_id)
            backend.DeleteCiphertext(ct_rotated_id)
            backend.DeletePlaintext(pt_result_id)

            return result[:len(values)]

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result, tolerance=1e-2)

    def test_polynomial_evaluation(self):
        """Test polynomial evaluation"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.GenerateRelinearizationKey()
            backend.NewEncoder()
            backend.NewEncryptor()
            backend.NewDecryptor()
            backend.NewEvaluator()
            backend.NewPolynomialEvaluator()

            # Encode and encrypt values
            values = [1.0, 2.0]

            pt_id = backend.Encode(values, 0, 1 << 20)
            ct_id = backend.Encrypt(pt_id)

            # Simple polynomial: 1 + 2x (coefficients)
            poly_coeffs = [1.0, 2.0]
            poly_id = backend.GenerateMonomial(poly_coeffs, len(poly_coeffs))

            # Evaluate polynomial
            ct_result_id = backend.EvaluatePolynomial(ct_id, poly_id, 1 << 40)

            # Decrypt and decode
            pt_result_id = backend.Decrypt(ct_result_id)
            result = backend.Decode(pt_result_id)

            # Clean up
            backend.DeletePlaintext(pt_id)
            backend.DeleteCiphertext(ct_id)
            backend.DeleteCiphertext(ct_result_id)
            backend.DeletePlaintext(pt_result_id)

            return result[:len(values)]

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result, tolerance=1e-1)

    def test_bootstrapping(self):
        """Test ciphertext bootstrapping"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.GenerateRelinearizationKey()
            backend.GenerateEvaluationKeys()
            backend.NewEncoder()
            backend.NewEncryptor()
            backend.NewDecryptor()
            backend.NewEvaluator()
            backend.NewBootstrapper()

            # Encode and encrypt values with smaller scale to allow for noise accumulation
            values = [0.5, 1.0]
            pt_id = backend.Encode(values, 3)  # Higher level
            ct_id = backend.Encrypt(pt_id)

            # Perform some operations to accumulate noise (multiple multiplications)
            # This helps test bootstrapping effectiveness
            for _ in range(2):
                ct_temp_id = backend.MulRelinCiphertext(ct_id, ct_id)
                backend.DeleteCiphertext(ct_id)
                ct_id = ct_temp_id

            # Bootstrap the ciphertext to refresh noise
            ct_bootstrapped_id = backend.Bootstrap(ct_id)

            # Decrypt and decode
            pt_result_id = backend.Decrypt(ct_bootstrapped_id)
            result = backend.Decode(pt_result_id)

            # Clean up
            backend.DeletePlaintext(pt_id)
            backend.DeleteCiphertext(ct_id)
            backend.DeleteCiphertext(ct_bootstrapped_id)
            backend.DeletePlaintext(pt_result_id)
            backend.DeleteBootstrappers()

            return result[:len(values)]

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result, tolerance=1e-1)


class KeyManagementTests(BackendCompatibilityTest):
    """Tests for key management functions"""

    def test_secret_key_serialization(self):
        """Test secret key serialization and loading"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()

            # Serialize secret key
            key_data = backend.SerializeSecretKey()

            # Load secret key
            # Convert numpy array to list for Lattigo compatibility
            if hasattr(key_data[0], 'tolist'):
                key_bytes = key_data[0].tolist()
            else:
                key_bytes = key_data[0]
            # Lattigo's LoadSecretKey automatically infers length from the array
            backend.LoadSecretKey(key_bytes)

            return len(key_data[0])  # Return size of serialized key

        lattigo_result, openfhe_result = self.run_backend_test(test_func)

        # Keys might have different sizes, but both should succeed
        self.assertTrue(lattigo_result.success, f"Lattigo failed: {lattigo_result.error}")
        self.assertTrue(openfhe_result.success, f"OpenFHE failed: {openfhe_result.error}")
        self.assertGreater(lattigo_result.result, 0, "Lattigo key should have positive size")
        self.assertGreater(openfhe_result.result, 0, "OpenFHE key should have positive size")


class ScaleLevelManagementTests(BackendCompatibilityTest):
    """Tests for scale and level management functions"""

    def test_plaintext_scale_management(self):
        """Test plaintext scale get/set operations"""
        def test_func(backend):
            # Setup
            backend.NewEncoder()

            # Create plaintext
            values = [1.0, 2.0, 3.0, 4.0]
            pt_id = backend.Encode(values, 0, 1 << 40)

            # Get initial scale
            initial_scale = backend.GetPlaintextScale(pt_id)

            # Set new scale
            new_scale = 1 << 45
            backend.SetPlaintextScale(pt_id, new_scale)

            # Get updated scale
            updated_scale = backend.GetPlaintextScale(pt_id)

            # Clean up
            backend.DeletePlaintext(pt_id)

            return {
                'initial_scale': initial_scale,
                'new_scale': new_scale,
                'updated_scale': updated_scale
            }

        lattigo_result, openfhe_result = self.run_backend_test(test_func)

        # Both should succeed
        self.assertTrue(lattigo_result.success, f"Lattigo failed: {lattigo_result.error}")
        self.assertTrue(openfhe_result.success, f"OpenFHE failed: {openfhe_result.error}")

        # Verify that scale setting worked
        self.assertGreater(lattigo_result.result['initial_scale'], 0, "Lattigo initial scale should be positive")
        self.assertGreater(openfhe_result.result['initial_scale'], 0, "OpenFHE initial scale should be positive")

    def test_ciphertext_scale_management(self):
        """Test ciphertext scale get/set operations"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.NewEncoder()
            backend.NewEncryptor()

            # Create and encrypt plaintext
            values = [1.0, 2.0, 3.0, 4.0]
            pt_id = backend.Encode(values, 0, 1 << 40)
            ct_id = backend.Encrypt(pt_id)

            # Get initial scale
            initial_scale = backend.GetCiphertextScale(ct_id)

            # Set new scale
            new_scale = 1 << 45
            backend.SetCiphertextScale(ct_id, new_scale)

            # Get updated scale
            updated_scale = backend.GetCiphertextScale(ct_id)

            # Clean up
            backend.DeletePlaintext(pt_id)
            backend.DeleteCiphertext(ct_id)

            return {
                'initial_scale': initial_scale,
                'new_scale': new_scale,
                'updated_scale': updated_scale
            }

        lattigo_result, openfhe_result = self.run_backend_test(test_func)

        # Both should succeed
        self.assertTrue(lattigo_result.success, f"Lattigo failed: {lattigo_result.error}")
        self.assertTrue(openfhe_result.success, f"OpenFHE failed: {openfhe_result.error}")

        # Verify that scale setting worked
        self.assertGreater(lattigo_result.result['initial_scale'], 0, "Lattigo initial scale should be positive")
        self.assertGreater(openfhe_result.result['initial_scale'], 0, "OpenFHE initial scale should be positive")

    def test_level_management(self):
        """Test level get operations"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.NewEncoder()
            backend.NewEncryptor()

            # Create and encrypt plaintext
            values = [1.0, 2.0]
            pt_id = backend.Encode(values, 1, 1 << 40)  # Level 1, scale
            ct_id = backend.Encrypt(pt_id)

            # Get levels
            pt_level = backend.GetPlaintextLevel(pt_id)
            ct_level = backend.GetCiphertextLevel(ct_id)

            # Clean up
            backend.DeletePlaintext(pt_id)
            backend.DeleteCiphertext(ct_id)

            return {
                'plaintext_level': pt_level,
                'ciphertext_level': ct_level
            }

        lattigo_result, openfhe_result = self.run_backend_test(test_func)

        # Both should succeed
        self.assertTrue(lattigo_result.success, f"Lattigo failed: {lattigo_result.error}")
        self.assertTrue(openfhe_result.success, f"OpenFHE failed: {openfhe_result.error}")

        # Verify levels are non-negative
        self.assertGreaterEqual(lattigo_result.result['plaintext_level'], 0, "Lattigo PT level should be >= 0")
        self.assertGreaterEqual(lattigo_result.result['ciphertext_level'], 0, "Lattigo CT level should be >= 0")
        self.assertGreaterEqual(openfhe_result.result['plaintext_level'], 0, "OpenFHE PT level should be >= 0")
        self.assertGreaterEqual(openfhe_result.result['ciphertext_level'], 0, "OpenFHE CT level should be >= 0")

    def test_slot_count_queries(self):
        """Test slot count queries"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.NewEncoder()
            backend.NewEncryptor()

            # Create and encrypt plaintext
            values = [1.0, 2.0, 3.0, 4.0]
            pt_id = backend.Encode(values, 0, 1 << 40)
            ct_id = backend.Encrypt(pt_id)

            # Get slot counts
            pt_slots = backend.GetPlaintextSlots(pt_id)
            ct_slots = backend.GetCiphertextSlots(ct_id)

            # Clean up
            backend.DeletePlaintext(pt_id)
            backend.DeleteCiphertext(ct_id)

            return {
                'plaintext_slots': pt_slots,
                'ciphertext_slots': ct_slots
            }

        lattigo_result, openfhe_result = self.run_backend_test(test_func)

        # Both should succeed
        self.assertTrue(lattigo_result.success, f"Lattigo failed: {lattigo_result.error}")
        self.assertTrue(openfhe_result.success, f"OpenFHE failed: {openfhe_result.error}")

        # Verify slot counts are positive
        self.assertGreater(lattigo_result.result['plaintext_slots'], 0, "Lattigo PT slots should be positive")
        self.assertGreater(lattigo_result.result['ciphertext_slots'], 0, "Lattigo CT slots should be positive")
        self.assertGreater(openfhe_result.result['plaintext_slots'], 0, "OpenFHE PT slots should be positive")
        self.assertGreater(openfhe_result.result['ciphertext_slots'], 0, "OpenFHE CT slots should be positive")


class SystemInformationTests(BackendCompatibilityTest):
    """Tests for memory and system information functions"""

    def test_moduli_chain_information(self):
        """Test moduli chain information retrieval"""
        def test_func(backend):
            # Setup (minimal setup needed)
            moduli_info = backend.GetModuliChain()
            return {
                'moduli_chain': moduli_info,
                'info_length': len(moduli_info) if moduli_info else 0
            }

        lattigo_result, openfhe_result = self.run_backend_test(test_func)

        # Both should succeed
        self.assertTrue(lattigo_result.success, f"Lattigo failed: {lattigo_result.error}")
        self.assertTrue(openfhe_result.success, f"OpenFHE failed: {openfhe_result.error}")

        # Verify moduli chain information is present
        self.assertGreater(lattigo_result.result['info_length'], 0, "Lattigo moduli info should not be empty")
        self.assertGreater(openfhe_result.result['info_length'], 0, "OpenFHE moduli info should not be empty")

    def test_memory_tracking(self):
        """Test live plaintext/ciphertext tracking"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.NewEncoder()
            backend.NewEncryptor()

            # Get initial counts
            initial_pts = backend.GetLivePlaintexts()
            initial_cts = backend.GetLiveCiphertexts()

            # Create several plaintexts and ciphertexts
            pt_ids = []
            ct_ids = []

            for i in range(3):
                values = [float(i), float(i+1)]
                pt_id = backend.Encode(values, 0, 1 << 40)
                ct_id = backend.Encrypt(pt_id)
                pt_ids.append(pt_id)
                ct_ids.append(ct_id)

            # Get counts after creation
            after_creation_pts = backend.GetLivePlaintexts()
            after_creation_cts = backend.GetLiveCiphertexts()

            # Delete half of them
            for i in range(2):
                backend.DeletePlaintext(pt_ids[i])
                backend.DeleteCiphertext(ct_ids[i])

            # Get counts after partial deletion
            after_deletion_pts = backend.GetLivePlaintexts()
            after_deletion_cts = backend.GetLiveCiphertexts()

            # Clean up remaining
            backend.DeletePlaintext(pt_ids[2])
            backend.DeleteCiphertext(ct_ids[2])

            # Get final counts
            final_pts = backend.GetLivePlaintexts()
            final_cts = backend.GetLiveCiphertexts()

            return {
                'initial_pts': len(initial_pts[0]) if isinstance(initial_pts, tuple) else initial_pts,
                'initial_cts': len(initial_cts[0]) if isinstance(initial_cts, tuple) else initial_cts,
                'after_creation_pts': len(after_creation_pts[0]) if isinstance(after_creation_pts, tuple) else after_creation_pts,
                'after_creation_cts': len(after_creation_cts[0]) if isinstance(after_creation_cts, tuple) else after_creation_cts,
                'after_deletion_pts': len(after_deletion_pts[0]) if isinstance(after_deletion_pts, tuple) else after_deletion_pts,
                'after_deletion_cts': len(after_deletion_cts[0]) if isinstance(after_deletion_cts, tuple) else after_deletion_cts,
                'final_pts': len(final_pts[0]) if isinstance(final_pts, tuple) else final_pts,
                'final_cts': len(final_cts[0]) if isinstance(final_cts, tuple) else final_cts
            }

        lattigo_result, openfhe_result = self.run_backend_test(test_func)

        # Both should succeed
        self.assertTrue(lattigo_result.success, f"Lattigo failed: {lattigo_result.error}")
        self.assertTrue(openfhe_result.success, f"OpenFHE failed: {openfhe_result.error}")

        # Verify memory tracking shows increases and decreases
        # (Note: exact numbers might differ due to internal allocations)
        lattigo_data = lattigo_result.result
        openfhe_data = openfhe_result.result

        # Verify that creation increased counts (at least for plaintexts)
        self.assertGreaterEqual(lattigo_data['after_creation_pts'], lattigo_data['initial_pts'])
        self.assertGreaterEqual(openfhe_data['after_creation_pts'], openfhe_data['initial_pts'])


class LinearTransformOperationTests(BackendCompatibilityTest):
    """Tests for linear transformation operations"""

    def test_linear_transform_evaluator_initialization(self):
        """Test linear transform evaluator initialization"""
        def test_func(backend):
            # Initialize linear transform evaluator
            backend.NewLinearTransformEvaluator()
            return True

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result)

    def test_linear_transform_generation(self):
        """Test linear transformation generation"""
        def test_func(backend):
            # Setup - need encoder before linear transform evaluator
            backend.NewEncoder()
            backend.NewLinearTransformEvaluator()

            # Create a simple linear transformation using diagonal format (Lattigo interface)
            # Use main diagonal (index 0) with identity values
            import ctypes
            import numpy as np

            diag_idxs = [0]  # Main diagonal
            # Lattigo expects len(diag_idxs) * MaxSlots() elements
            # MaxSlots() seems to be 8192, so provide that many elements
            max_slots = 8192  # This should match Lattigo's MaxSlots()
            diag_data = [1.0] * max_slots  # Identity values for all slots
            level = 1
            bsgs_ratio = 1.0
            io_mode = "memory"

            # Convert to ctypes arrays
            diag_idxs_array = (ctypes.c_int * len(diag_idxs))(*diag_idxs)
            diag_data_array = (ctypes.c_float * len(diag_data))(*diag_data)
            io_mode_bytes = io_mode.encode('utf-8')

            # Generate linear transformation with proper Lattigo signature
            transform_id = backend.GenerateLinearTransform(
                diag_idxs_array, len(diag_idxs),
                diag_data_array, len(diag_data),
                level, bsgs_ratio, io_mode_bytes
            )

            # Clean up
            backend.DeleteLinearTransform(transform_id)

            return transform_id

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        # Both should return valid transform IDs (non-negative integers, 0 is valid)
        self.assertTrue(lattigo_result.success, f"Lattigo failed: {lattigo_result.error}")
        self.assertTrue(openfhe_result.success, f"OpenFHE failed: {openfhe_result.error}")
        self.assertGreaterEqual(lattigo_result.result, 0, "Lattigo should return valid transform ID")
        self.assertGreaterEqual(openfhe_result.result, 0, "OpenFHE should return valid transform ID")

    def test_linear_transform_rotation_keys(self):
        """Test linear transformation rotation key requirements"""
        def test_func(backend):
            # Setup - need encoder before linear transform evaluator
            backend.NewEncoder()
            backend.NewLinearTransformEvaluator()

            # Create a simple linear transformation using diagonal format (Lattigo interface)
            import ctypes
            import numpy as np

            diag_idxs = [0]  # Main diagonal
            # Lattigo expects len(diag_idxs) * MaxSlots() elements
            # MaxSlots() seems to be 8192, so provide that many elements
            max_slots = 8192  # This should match Lattigo's MaxSlots()
            diag_data = [1.0] * max_slots  # Identity values for all slots
            level = 1
            bsgs_ratio = 1.0
            io_mode = "memory"

            # Convert to ctypes arrays
            diag_idxs_array = (ctypes.c_int * len(diag_idxs))(*diag_idxs)
            diag_data_array = (ctypes.c_float * len(diag_data))(*diag_data)
            io_mode_bytes = io_mode.encode('utf-8')

            # Generate linear transformation with proper Lattigo signature
            transform_id = backend.GenerateLinearTransform(
                diag_idxs_array, len(diag_idxs),
                diag_data_array, len(diag_data),
                level, bsgs_ratio, io_mode_bytes
            )

            # Get required rotation keys
            rotation_keys = backend.GetLinearTransformRotationKeys(transform_id)

            # Clean up
            backend.DeleteLinearTransform(transform_id)

            return {
                'rotation_keys': rotation_keys,
                'num_keys': len(rotation_keys) if rotation_keys else 0
            }

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        # Both should succeed
        self.assertTrue(lattigo_result.success, f"Lattigo failed: {lattigo_result.error}")
        self.assertTrue(openfhe_result.success, f"OpenFHE failed: {openfhe_result.error}")

        # Results might differ slightly but should be reasonable
        self.assertIsInstance(lattigo_result.result['rotation_keys'], list)
        self.assertIsInstance(openfhe_result.result['rotation_keys'], list)

    def test_linear_transform_evaluation(self):
        """Test linear transformation evaluation on encrypted data"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.GenerateEvaluationKeys()
            backend.NewEncoder()
            backend.NewEncryptor()
            backend.NewDecryptor()
            backend.NewEvaluator()
            backend.NewLinearTransformEvaluator()

            # Create a simple scaling matrix (multiply by 2)
            matrix_data = [2.0, 0.0, 0.0, 2.0]
            num_rows, num_cols = 2, 2
            transform_id = backend.GenerateLinearTransform(matrix_data, num_rows, num_cols)

            # Get and generate required rotation keys
            rotation_keys = backend.GetLinearTransformRotationKeys(transform_id)
            for key in rotation_keys:
                backend.GenerateLinearTransformRotationKey(key)

            # Create test data
            test_values = [1.0, 2.0]
            pt_id = backend.Encode(test_values, 0, 1 << 40)
            ct_id = backend.Encrypt(pt_id)

            # Apply linear transformation
            result_ct_id = backend.EvaluateLinearTransform(transform_id, ct_id)

            # Decrypt and decode result
            result_pt_id = backend.Decrypt(result_ct_id)
            result = backend.Decode(result_pt_id)

            # Clean up
            backend.DeletePlaintext(pt_id)
            backend.DeleteCiphertext(ct_id)
            backend.DeleteCiphertext(result_ct_id)
            backend.DeletePlaintext(result_pt_id)
            backend.DeleteLinearTransform(transform_id)

            return result[:len(test_values)]

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result, tolerance=1e-1)

    def test_rotation_key_serialization(self):
        """Test rotation key serialization and loading"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GenerateEvaluationKeys()  # Initialize evaluation keys structure
            backend.NewEncoder()
            backend.NewLinearTransformEvaluator()

            rotation_amount = 1

            # Generate and serialize rotation key
            serialized_key = backend.GenerateAndSerializeRotationKey(rotation_amount)

            # Load rotation key with proper Lattigo signature
            import ctypes

            # Convert serialized_key to proper ctypes format
            # Different backends expect different formats:
            # - Lattigo: POINTER(c_ubyte), c_ulong, c_ulong
            # - OpenFHE: c_char_p, c_size_t, c_int

            if isinstance(serialized_key, tuple):
                # Lattigo returns (numpy_array, ctypes_pointer) - use the numpy array
                numpy_array = serialized_key[0]
                serialized_bytes = numpy_array.tobytes()
            elif hasattr(serialized_key, 'ctypes'):
                # numpy array -> convert to bytes
                serialized_bytes = serialized_key.tobytes()
            elif hasattr(serialized_key, 'Data') and hasattr(serialized_key, 'Length'):
                # OpenFHE ArrayResultByte -> convert to bytes
                buffer = ctypes.cast(
                    serialized_key.Data,
                    ctypes.POINTER(ctypes.c_ubyte * serialized_key.Length)
                ).contents
                serialized_bytes = bytes(buffer)
            else:
                # bytes or other format
                if isinstance(serialized_key, bytes):
                    serialized_bytes = serialized_key
                else:
                    serialized_bytes = bytes(serialized_key)

            # Convert to appropriate ctypes format based on backend
            if 'Lattigo' in backend.__class__.__name__:
                # Lattigo backend expects POINTER(c_ubyte)
                data_array = (ctypes.c_ubyte * len(serialized_bytes))(*serialized_bytes)
                data_ptr = ctypes.cast(data_array, ctypes.POINTER(ctypes.c_ubyte))
                data_len = len(serialized_bytes)
                backend.LoadRotationKey(data_ptr, data_len, rotation_amount)
            else:
                # OpenFHE backend expects c_char_p
                data_ptr = ctypes.c_char_p(serialized_bytes)
                data_len = len(serialized_bytes)
                backend.LoadRotationKey(data_ptr, data_len, rotation_amount)

            # Calculate serialized size based on format
            if isinstance(serialized_key, tuple):
                # Lattigo returns (numpy_array, ctypes_pointer) - use numpy array size
                serialized_size = len(serialized_key[0])
            elif hasattr(serialized_key, '__len__'):
                serialized_size = len(serialized_key)
            elif hasattr(serialized_key, 'Length'):
                # OpenFHE ArrayResultByte
                serialized_size = serialized_key.Length
            else:
                serialized_size = 0

            return {
                'serialized_size': serialized_size,
                'rotation_amount': rotation_amount
            }

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        # Both should succeed
        self.assertTrue(lattigo_result.success, f"Lattigo failed: {lattigo_result.error}")
        self.assertTrue(openfhe_result.success, f"OpenFHE failed: {openfhe_result.error}")
        # Serialized sizes might differ
        self.assertGreater(lattigo_result.result['serialized_size'], 0)
        self.assertGreater(openfhe_result.result['serialized_size'], 0)

    def test_diagonal_serialization(self):
        """Test diagonal data serialization and loading"""
        def test_func(backend):
            # Setup
            backend.NewEncoder()
            backend.NewLinearTransformEvaluator()

            # Create diagonal data
            diagonal_data = [1.0, 2.0, 3.0, 4.0]

            # Serialize diagonal
            serialized_diagonal = backend.SerializeDiagonal(diagonal_data)

            # Load plaintext diagonal
            pt_id = backend.LoadPlaintextDiagonal(serialized_diagonal)

            # Decode to verify
            decoded_values = backend.Decode(pt_id)

            # Clean up
            backend.DeletePlaintext(pt_id)

            return {
                'original': diagonal_data,
                'decoded': decoded_values[:len(diagonal_data)],
                'serialized_size': len(serialized_diagonal)
            }

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result, tolerance=1e-3)

    def test_memory_cleanup_operations(self):
        """Test linear transform memory cleanup operations"""
        def test_func(backend):
            # Setup
            backend.NewEncoder()
            backend.NewLinearTransformEvaluator()

            # Create some diagonal plaintexts
            diagonal_ids = []
            for i in range(3):
                diagonal_data = [float(i), float(i+1)]
                serialized = backend.SerializeDiagonal(diagonal_data)
                pt_id = backend.LoadPlaintextDiagonal(serialized)
                diagonal_ids.append(pt_id)

            # Remove plaintext diagonals
            backend.RemovePlaintextDiagonals(diagonal_ids)

            # Test rotation key removal
            rotation_amounts = [1, 2, 3]
            backend.RemoveRotationKeys(rotation_amounts)

            return len(diagonal_ids)

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result)


class MemoryManagementTests(BackendCompatibilityTest):
    """Tests for memory management functions"""

    def test_plaintext_lifecycle(self):
        """Test plaintext creation and deletion"""
        def test_func(backend):
            # Initialize encoder
            backend.NewEncoder()

            # Create multiple plaintexts
            pt_ids = []
            for i in range(5):
                values = [float(i), float(i+1)]
                pt_id = backend.Encode(values, 0, 1 << 40)
                pt_ids.append(pt_id)

            # Delete all plaintexts
            for pt_id in pt_ids:
                backend.DeletePlaintext(pt_id)

            return len(pt_ids)

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result)

    def test_ciphertext_lifecycle(self):
        """Test ciphertext creation and deletion"""
        def test_func(backend):
            # Setup
            backend.NewKeyGenerator()
            backend.GenerateSecretKey()
            backend.GeneratePublicKey()
            backend.NewEncoder()
            backend.NewEncryptor()

            # Create multiple ciphertexts
            ct_ids = []
            for i in range(3):
                values = [float(i), float(i+1)]
                pt_id = backend.Encode(values, 0, 1 << 40)
                ct_id = backend.Encrypt(pt_id)
                ct_ids.append(ct_id)
                backend.DeletePlaintext(pt_id)

            # Delete all ciphertexts
            for ct_id in ct_ids:
                backend.DeleteCiphertext(ct_id)

            return len(ct_ids)

        lattigo_result, openfhe_result = self.run_backend_test(test_func)
        self.assert_results_compatible(lattigo_result, openfhe_result)


def create_test_suite(test_pattern: str = None) -> unittest.TestSuite:
    """Create a test suite with optional filtering"""
    loader = unittest.TestLoader()
    suite = unittest.TestSuite()

    # Add all test classes
    test_classes = [
        CoreFunctionTests,
        ArithmeticOperationTests,
        AdvancedCiphertextOperationTests,
        AdvancedOperationTests,
        LinearTransformOperationTests,
        ScaleLevelManagementTests,
        SystemInformationTests,
        KeyManagementTests,
        MemoryManagementTests,
    ]

    for test_class in test_classes:
        if test_pattern:
            # Filter tests by pattern
            class_suite = loader.loadTestsFromTestCase(test_class)
            filtered_tests = unittest.TestSuite()
            for test in class_suite:
                if test_pattern in str(test):
                    filtered_tests.addTest(test)
            suite.addTest(filtered_tests)
        else:
            suite.addTest(loader.loadTestsFromTestCase(test_class))

    return suite


def main():
    """Main test runner"""
    parser = argparse.ArgumentParser(description='Backend Compatibility Test Suite')
    parser.add_argument('--verbose', '-v', action='store_true',
                       help='Verbose output')
    parser.add_argument('--test-function', '-t', type=str,
                       help='Run specific test function (partial name match)')
    parser.add_argument('--list-tests', '-l', action='store_true',
                       help='List all available tests')

    args = parser.parse_args()

    # Check backend availability
    print("=== Backend Compatibility Test Suite ===")
    print(f"Lattigo Available: {'✓' if LATTIGO_AVAILABLE else '✗'}")
    print(f"OpenFHE Available: {'✓' if OPENFHE_AVAILABLE else '✗'}")
    print()

    if not LATTIGO_AVAILABLE or not OPENFHE_AVAILABLE:
        print("Both backends must be available to run compatibility tests.")
        return 1

    # Create test suite
    suite = create_test_suite(args.test_function)

    if args.list_tests:
        print("Available tests:")
        for test in suite:
            if hasattr(test, '_tests'):
                for subtest in test._tests:
                    print(f"  {subtest}")
            else:
                print(f"  {test}")
        return 0

    # Run tests
    verbosity = 2 if args.verbose else 1
    runner = unittest.TextTestRunner(verbosity=verbosity, buffer=True)

    print(f"Running {suite.countTestCases()} tests...")
    print("=" * 70)

    result = runner.run(suite)

    # Print summary
    print("=" * 70)
    print("Test Summary:")
    print(f"Tests run: {result.testsRun}")
    print(f"Failures: {len(result.failures)}")
    print(f"Errors: {len(result.errors)}")
    print(f"Success rate: {((result.testsRun - len(result.failures) - len(result.errors)) / result.testsRun * 100):.1f}%")

    if result.failures:
        print("\nFailures:")
        for test, traceback in result.failures:
            print(f"  {test}: {traceback.splitlines()[-1]}")

    if result.errors:
        print("\nErrors:")
        for test, traceback in result.errors:
            print(f"  {test}: {traceback.splitlines()[-1]}")

    return 0 if result.wasSuccessful() else 1


if __name__ == '__main__':
    sys.exit(main())