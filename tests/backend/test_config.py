"""
Configuration file for backend compatibility tests

This file contains test parameters and configurations that can be
adjusted based on the testing environment and requirements.
"""

import os
from typing import Dict, List, Any

class TestConfig:
    """Configuration class for backend compatibility tests"""

    # Test data configurations
    TEST_VALUES = {
        'simple': [1.0, 2.0],
        'small': [1.0, 2.0, 3.0, 4.0],
        'medium': [1.0, 2.0, 3.0, 4.0, 0.5, -1.0, 0.0, 10.0],
        'large': [float(i) for i in range(16)],
        'negative': [-1.0, -2.0, -3.0, -4.0],
        'mixed': [1.0, -2.0, 3.5, -4.1, 0.0, 100.0, -0.001, 42.0]
    }

    # Scheme parameters for different test scenarios
    SCHEME_PARAMS = {
        'minimal': {
            'logn': 12,
            'logq': [40, 30],
            'logp': [40],
            'logscale': 30,
            'hamming_weight': 4096,
            'ringtype': 'standard',
            'keys_path': './test_keys_minimal',
            'io_mode': 'memory'
        },
        'standard': {
            'logn': 14,
            'logq': [60, 40, 40, 60],
            'logp': [60],
            'logscale': 40,
            'hamming_weight': 8192,
            'ringtype': 'standard',
            'keys_path': './test_keys_standard',
            'io_mode': 'memory'
        },
        'large': {
            'logn': 15,
            'logq': [60, 50, 50, 50, 60],
            'logp': [60, 60],
            'logscale': 50,
            'hamming_weight': 8192,
            'ringtype': 'standard',
            'keys_path': './test_keys_large',
            'io_mode': 'memory'
        }
    }

    # Tolerance levels for different types of operations
    TOLERANCES = {
        'encoding': 1e-6,        # Encode/decode operations
        'basic_arithmetic': 1e-3, # Addition, subtraction
        'multiplication': 1e-2,   # Multiplication operations
        'polynomial': 1e-1,       # Polynomial evaluation
        'rotation': 1e-3,         # Rotation operations
        'bootstrap': 1e-1,        # Bootstrapping operations
        'approximate': 1e0        # Very approximate operations
    }

    # Test execution settings
    EXECUTION = {
        'timeout_seconds': 60,    # Maximum time per test
        'max_retries': 3,         # Number of retries for flaky tests
        'cleanup_on_failure': True, # Clean up resources even if test fails
        'parallel_backends': False,  # Whether to run backends in parallel
        'save_intermediate_results': False,  # Save intermediate computation results
    }

    # Memory management settings
    MEMORY = {
        'max_plaintexts': 100,    # Maximum number of plaintexts to create in memory tests
        'max_ciphertexts': 50,    # Maximum number of ciphertexts to create in memory tests
        'cleanup_frequency': 10,  # Clean up every N operations in stress tests
        'track_memory_usage': True, # Whether to track memory usage during tests
    }

    # Polynomial test configurations
    POLYNOMIALS = {
        'linear': [1.0, 2.0],                    # 1 + 2x
        'quadratic': [1.0, 0.5, 0.25],          # 1 + 0.5x + 0.25x²
        'cubic': [1.0, -1.0, 0.5, -0.1],        # 1 - x + 0.5x² - 0.1x³
        'chebyshev_3': [0.0, 1.0, 0.0, -0.25],  # Chebyshev polynomial T₃(x) = 4x³ - 3x
        'sign_approx': [0.0, 1.27324, 0.0, -0.42441, 0.0, 0.08478],  # Sign function approximation
    }

    # Rotation test configurations
    ROTATIONS = {
        'simple': [1, -1, 2, -2],
        'power_of_two': [1, 2, 4, 8, 16],
        'large': [100, 256, 512, 1024],
    }

    # Test categories and their priorities
    TEST_CATEGORIES = {
        'core': ['scheme_initialization', 'key_generation', 'basic_encoding_decoding', 'basic_encryption_decryption'],
        'arithmetic': ['ciphertext_addition', 'plaintext_addition', 'ciphertext_multiplication'],
        'advanced': ['rotation', 'polynomial_evaluation', 'bootstrap'],
        'memory': ['plaintext_lifecycle', 'ciphertext_lifecycle'],
        'keys': ['secret_key_serialization'],
        'stress': ['multiple_operations', 'large_computations', 'memory_pressure'],
    }

    # Expected behavior for edge cases
    EDGE_CASES = {
        'zero_values': [0.0, 0.0, 0.0, 0.0],
        'small_values': [1e-10, 1e-9, 1e-8, 1e-7],
        'large_values': [1e10, 1e9, 1e8, 1e7],
        'boundary_values': [-1.0, -0.5, 0.0, 0.5, 1.0],
    }

    @classmethod
    def get_test_values(cls, category: str = 'simple') -> List[float]:
        """Get test values for a specific category"""
        return cls.TEST_VALUES.get(category, cls.TEST_VALUES['simple'])

    @classmethod
    def get_scheme_params(cls, category: str = 'standard') -> Dict[str, Any]:
        """Get scheme parameters for a specific category"""
        return cls.SCHEME_PARAMS.get(category, cls.SCHEME_PARAMS['standard'])

    @classmethod
    def get_tolerance(cls, operation: str = 'basic_arithmetic') -> float:
        """Get tolerance for a specific operation"""
        return cls.TOLERANCES.get(operation, cls.TOLERANCES['basic_arithmetic'])

    @classmethod
    def get_polynomial(cls, poly_type: str = 'linear') -> List[float]:
        """Get polynomial coefficients for a specific type"""
        return cls.POLYNOMIALS.get(poly_type, cls.POLYNOMIALS['linear'])

    @classmethod
    def should_run_category(cls, category: str, test_level: str = 'standard') -> bool:
        """Determine if a test category should be run based on test level"""
        if test_level == 'minimal':
            return category in ['core']
        elif test_level == 'standard':
            return category in ['core', 'arithmetic', 'memory', 'keys']
        elif test_level == 'comprehensive':
            return category in ['core', 'arithmetic', 'advanced', 'memory', 'keys']
        elif test_level == 'stress':
            return True  # Run all categories including stress tests
        else:
            return category in ['core', 'arithmetic']

    @classmethod
    def create_test_environment(cls, clean_start: bool = True) -> None:
        """Create necessary directories and clean up from previous runs"""
        if clean_start:
            # Clean up any existing test directories
            import shutil
            for params in cls.SCHEME_PARAMS.values():
                keys_path = params['keys_path']
                if os.path.exists(keys_path):
                    shutil.rmtree(keys_path, ignore_errors=True)

        # Create test directories
        for params in cls.SCHEME_PARAMS.values():
            keys_path = params['keys_path']
            os.makedirs(keys_path, exist_ok=True)

    @classmethod
    def cleanup_test_environment(cls) -> None:
        """Clean up test environment after tests complete"""
        import shutil
        for params in cls.SCHEME_PARAMS.values():
            keys_path = params['keys_path']
            if os.path.exists(keys_path):
                shutil.rmtree(keys_path, ignore_errors=True)


# Environment-specific configurations
class EnvironmentConfig:
    """Environment-specific configuration"""

    @staticmethod
    def is_ci_environment() -> bool:
        """Check if running in CI environment"""
        return os.getenv('CI', '').lower() in ['true', '1', 'yes']

    @staticmethod
    def is_gpu_available() -> bool:
        """Check if GPU acceleration is available"""
        # This could be extended to check for actual GPU availability
        return os.getenv('CUDA_VISIBLE_DEVICES') is not None

    @staticmethod
    def get_max_threads() -> int:
        """Get maximum number of threads to use"""
        import multiprocessing
        default_threads = multiprocessing.cpu_count()
        return int(os.getenv('MAX_THREADS', default_threads))

    @classmethod
    def adjust_config_for_environment(cls, config: TestConfig) -> TestConfig:
        """Adjust configuration based on environment"""
        if cls.is_ci_environment():
            # Use smaller parameters for CI to speed up tests
            config.EXECUTION['timeout_seconds'] = 30
            config.MEMORY['max_plaintexts'] = 20
            config.MEMORY['max_ciphertexts'] = 10

        if not cls.is_gpu_available():
            # Disable GPU-specific tests if no GPU available
            pass

        return config


# Validation functions
def validate_config():
    """Validate the test configuration"""
    errors = []

    # Check that all test categories have valid entries
    for category, tests in TestConfig.TEST_CATEGORIES.items():
        if not tests:
            errors.append(f"Category '{category}' has no tests")

    # Check that polynomial coefficients are valid
    for poly_name, coeffs in TestConfig.POLYNOMIALS.items():
        if not coeffs or not all(isinstance(c, (int, float)) for c in coeffs):
            errors.append(f"Polynomial '{poly_name}' has invalid coefficients")

    # Check that tolerances are positive
    for op, tolerance in TestConfig.TOLERANCES.items():
        if tolerance <= 0:
            errors.append(f"Tolerance for '{op}' must be positive")

    if errors:
        raise ValueError("Configuration validation failed:\n" + "\n".join(errors))

    return True


if __name__ == '__main__':
    # Validate configuration when run directly
    try:
        validate_config()
        print("✓ Configuration validation passed")

        # Print configuration summary
        print("\nConfiguration Summary:")
        print(f"Test value categories: {list(TestConfig.TEST_VALUES.keys())}")
        print(f"Scheme parameter sets: {list(TestConfig.SCHEME_PARAMS.keys())}")
        print(f"Test categories: {list(TestConfig.TEST_CATEGORIES.keys())}")
        print(f"Polynomial types: {list(TestConfig.POLYNOMIALS.keys())}")

        # Check environment
        env = EnvironmentConfig()
        print(f"\nEnvironment:")
        print(f"CI Environment: {env.is_ci_environment()}")
        print(f"GPU Available: {env.is_gpu_available()}")
        print(f"Max Threads: {env.get_max_threads()}")

    except Exception as e:
        print(f"✗ Configuration validation failed: {e}")
        exit(1)