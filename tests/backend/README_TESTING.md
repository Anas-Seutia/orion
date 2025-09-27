# Backend Compatibility Testing Suite

This directory contains a comprehensive testing suite to verify that the OpenFHE backend functions behave identically to the Lattigo backend functions, ensuring complete compatibility between the two FHE backends in the Orion framework.

## Overview

The testing suite validates that:
1. **Function Signatures Match**: Both backends accept the same parameters
2. **Return Values are Compatible**: Results are numerically equivalent within acceptable tolerances
3. **Error Handling is Consistent**: Both backends fail gracefully in the same scenarios
4. **Memory Management Works**: Proper resource allocation and cleanup
5. **Performance is Reasonable**: Operations complete within expected time bounds

## Files

- `test_backend_compatibility.py` - Main test suite with comprehensive compatibility tests
- `test_config.py` - Configuration file with test parameters, tolerances, and settings
- `README_TESTING.md` - This documentation file

## Quick Start

### Prerequisites

1. Both Lattigo and OpenFHE backends must be properly installed and accessible
2. Required dependencies: `numpy`, `unittest` (standard library)

### Running Tests

```bash
# Run all compatibility tests
python test_backend_compatibility.py

# Run with verbose output
python test_backend_compatibility.py --verbose

# Run specific test function
python test_backend_compatibility.py --test-function=test_basic_encryption

# List all available tests
python test_backend_compatibility.py --list-tests

# Validate test configuration
python test_config.py
```

## Test Categories

### 🔵 Core Function Tests (`CoreFunctionTests`)
Tests fundamental FHE operations that must work identically:
- **Scheme Initialization**: Backend setup and parameter validation
- **Key Generation**: Secret, public, relinearization, and evaluation keys
- **Basic Encoding/Decoding**: Converting between plaintext values and FHE plaintexts
- **Basic Encryption/Decryption**: Core encrypt/decrypt operations

### 🟢 Arithmetic Operation Tests (`ArithmeticOperationTests`)
Tests homomorphic arithmetic operations:
- **Ciphertext Addition**: Adding two encrypted values
- **Plaintext Addition**: Adding plaintext to encrypted value
- **Ciphertext Multiplication**: Multiplying encrypted values with relinearization

### 🟡 Advanced Operation Tests (`AdvancedOperationTests`)
Tests sophisticated FHE operations:
- **Rotation**: Cyclically shifting encrypted vector elements
- **Polynomial Evaluation**: Evaluating polynomials on encrypted data

### 🔴 Key Management Tests (`KeyManagementTests`)
Tests cryptographic key operations:
- **Secret Key Serialization**: Saving and loading secret keys

### 🟣 Memory Management Tests (`MemoryManagementTests`)
Tests resource management:
- **Plaintext Lifecycle**: Creating and deleting plaintext objects
- **Ciphertext Lifecycle**: Creating and deleting ciphertext objects

## Test Configuration

### Tolerance Levels
Different operations have different numerical precision requirements:

```python
TOLERANCES = {
    'encoding': 1e-6,        # Encode/decode operations
    'basic_arithmetic': 1e-3, # Addition, subtraction
    'multiplication': 1e-2,   # Multiplication operations
    'polynomial': 1e-1,       # Polynomial evaluation
    'rotation': 1e-3,         # Rotation operations
    'bootstrap': 1e-1,        # Bootstrapping operations
}
```

### Scheme Parameters
Tests can be run with different parameter sets:

- **Minimal**: Fast tests with small parameters (`logn=12`)
- **Standard**: Typical production parameters (`logn=14`)
- **Large**: High-security parameters (`logn=15`)

### Test Data
Various test data sets are available:

- **Simple**: `[1.0, 2.0]` - Basic functionality
- **Small**: `[1.0, 2.0, 3.0, 4.0]` - Small vectors
- **Mixed**: Positive, negative, and zero values
- **Edge Cases**: Boundary values and special cases

## Understanding Test Results

### Success Criteria
A test passes when:
1. Both backends execute without errors
2. Results are numerically equivalent within tolerance
3. Execution completes within time limits

### Failure Analysis
Common failure modes:

#### ❌ **Function Not Implemented**
```
OpenFHE backend not available: No module named 'openfhe_bindings'
```
**Solution**: Ensure OpenFHE backend is properly compiled and installed

#### ❌ **Numerical Mismatch**
```
AssertionError: Numeric results differ: 3.0001 vs 2.9999
```
**Solution**: Check if tolerance needs adjustment or if there's a real compatibility issue

#### ❌ **Different Error Behavior**
```
Backend behavior mismatch:
Lattigo: SUCCESS
OpenFHE: FAILED
```
**Solution**: Investigate why one backend succeeds while the other fails

#### ❌ **Memory Issues**
```
RuntimeError: Failed to delete ciphertext
```
**Solution**: Check resource cleanup and memory management

## Advanced Usage

### Custom Test Parameters

You can modify `test_config.py` to adjust test parameters:

```python
# Use smaller parameters for faster tests
SCHEME_PARAMS['fast'] = {
    'logn': 10,
    'logq': [30, 20],
    'logp': [30],
    'logscale': 20,
    # ...
}
```

### Adding New Tests

To add a new compatibility test:

1. Create a new test method in the appropriate test class
2. Use the `run_backend_test()` helper method
3. Assert compatibility with `assert_results_compatible()`

```python
def test_my_new_operation(self):
    """Test my new FHE operation"""
    def test_func(backend):
        # Your test logic here
        result = backend.MyNewOperation(params)
        return result

    lattigo_result, openfhe_result = self.run_backend_test(test_func)
    self.assert_results_compatible(lattigo_result, openfhe_result)
```

### Integration with CI/CD

The test suite is designed to work in automated environments:

```bash
# Set environment variables for CI
export CI=true
export MAX_THREADS=2

# Run with minimal parameters for speed
python test_backend_compatibility.py --test-function=test_basic
```

## Performance Monitoring

Tests automatically measure execution time for performance comparison:

```
Test: test_basic_encryption
Lattigo: 0.045s
OpenFHE: 0.052s
Difference: +15.6%
```

## Troubleshooting

### Common Issues

1. **Backend Not Available**
   - Verify backend libraries are installed
   - Check Python path includes backend modules
   - Ensure shared libraries (.so/.dll files) are accessible

2. **Parameter Mismatch**
   - Backends may have different parameter requirements
   - Adjust scheme parameters in `test_config.py`
   - Check backend documentation for valid parameter ranges

3. **Numerical Precision**
   - FHE operations introduce noise and approximation errors
   - Adjust tolerance levels if needed
   - Consider if precision differences are acceptable

4. **Memory Issues**
   - Ensure proper cleanup in test teardown
   - Monitor system memory usage
   - Reduce test data size if needed

### Getting Help

1. Check the main documentation in `backend_bindings_comparison.md`
2. Review backend-specific documentation
3. Run individual tests to isolate issues
4. Enable verbose output for detailed error messages

## Contributing

When adding new tests:

1. Follow the existing test structure
2. Add appropriate documentation
3. Update `test_config.py` if new parameters are needed
4. Ensure tests clean up resources properly
5. Add the test to the appropriate category in the summary

## Example Test Run

```bash
$ python test_backend_compatibility.py --verbose

=== Backend Compatibility Test Suite ===
Lattigo Available: ✓
OpenFHE Available: ✓

✓ Lattigo backend initialized successfully
✓ OpenFHE backend initialized successfully

Running 15 tests...
======================================================================
test_basic_encryption_decryption (CoreFunctionTests) ... ok (0.087s)
test_ciphertext_addition (ArithmeticOperationTests) ... ok (0.134s)
test_plaintext_addition (ArithmeticOperationTests) ... ok (0.098s)
test_rotation (AdvancedOperationTests) ... ok (0.156s)
test_secret_key_serialization (KeyManagementTests) ... ok (0.045s)
...

======================================================================
Test Summary:
Tests run: 15
Failures: 0
Errors: 0
Success rate: 100.0%
```

---

*This testing suite ensures robust compatibility between Lattigo and OpenFHE backends, providing confidence that applications can switch between backends without functional differences.*