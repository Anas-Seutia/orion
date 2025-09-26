# orion/backend/openfhe/bindings.py
# Updated OpenFHE Python bindings for modular Orion backend

import ctypes
import numpy as np
from pathlib import Path
import os
import sys
from typing import List, Optional, Union

class OpenFHEFunction:
    """Wrapper for OpenFHE C++ functions with enhanced error handling"""
    
    FreeCArray = None  # Will be set during initialization
    
    def __init__(self, func, argtypes=None, restype=None, name=None):
        self.func = func
        self.name = name or getattr(func, '__name__', 'unknown')
        
        if argtypes is not None:
            self.func.argtypes = argtypes
        if restype is not None:
            self.func.restype = restype
    
    def __call__(self, *args):
        try:
            result = self.func(*args)
            return result
        except Exception as e:
            print(f"OpenFHE function '{self.name}' failed: {e}")
            print(f"Arguments: {args}")
            raise RuntimeError(f"OpenFHE function call failed: {e}")

class OpenFHEBackend:
    """Enhanced OpenFHE backend implementation for Orion"""
    
    def __init__(self, orion_params):
        """Initialize the OpenFHE backend with given parameters"""
        self.lib = None
        self.initialized = False
        self.load_library()
        self.setup_functions()
        self.initialize_scheme(orion_params)
        
    def load_library(self):
        """Load the OpenFHE shared library with improved path handling"""
        try:
            # Get the directory containing this Python file
            current_dir = Path(__file__).parent.absolute()

            # Try different possible library names and paths
            library_names = [
                'libopenfhe_orion.so',     # Linux
                'libopenfhe_orion.dylib',  # macOS
                'openfhe_orion.dll',       # Windows
                'openfhe_orion.so',        # Alternative Linux name
            ]

            lib_paths = [
                current_dir / 'build',          # Build directory
                current_dir,                    # Current directory
                current_dir / 'lib',           # lib subdirectory
                current_dir / '..' / 'build',  # Parent build directory
                Path('/usr/local/lib'),        # System lib directory
                Path.cwd(),                    # Current working directory
            ]

            # Try to load the library
            for path in lib_paths:
                for lib_name in library_names:
                    lib_path = path / lib_name
                    if lib_path.exists():
                        try:
                            self.lib = ctypes.CDLL(str(lib_path))
                            print(f"Successfully loaded OpenFHE library: {lib_path}")
                            return
                        except OSError as e:
                            print(f"Failed to load {lib_path}: {e}")
                            continue

            # If we get here, library wasn't found
            self._print_debug_info(lib_paths, library_names)
            raise FileNotFoundError(
                f"OpenFHE library not found. Searched for {library_names} in {len(lib_paths)} paths."
            )

        except Exception as e:
            raise RuntimeError(f"Failed to load OpenFHE library: {e}")

    def _print_debug_info(self, lib_paths, library_names):
        """Print debugging information about library search"""
        print("=== OpenFHE Library Search Debug Info ===")
        print(f"Searched for library names: {library_names}")
        print("Search paths and contents:")
        
        for path in lib_paths:
            print(f"  {path} - exists: {path.exists()}")
            if path.exists() and path.is_dir():
                try:
                    # Look for any files that might be relevant
                    openfhe_files = list(path.glob("*openfhe*"))
                    orion_files = list(path.glob("*orion*"))
                    so_files = list(path.glob("*.so"))
                    
                    if openfhe_files:
                        print(f"    OpenFHE files: {[f.name for f in openfhe_files]}")
                    if orion_files:
                        print(f"    Orion files: {[f.name for f in orion_files]}")
                    if so_files:
                        print(f"    .so files: {[f.name for f in so_files[:10]]}")  # Limit output
                        
                except Exception as e:
                    print(f"    Error listing directory: {e}")

    def setup_functions(self):
        """Setup all C function bindings"""
        self.setup_scheme_functions()
        self.setup_tensor_functions()
        self.setup_encoder_functions()
        self.setup_encryptor_functions()
        self.setup_evaluator_functions()
        self.setup_linear_transform_functions()
        self.setup_utility_functions()

    def setup_scheme_functions(self):
        """Setup scheme management functions"""
        self.NewScheme = OpenFHEFunction(
            self.lib.InitializeOrionBackend,
            argtypes=[
                ctypes.c_int,  # logN
                ctypes.POINTER(ctypes.c_int),  # logQ array
                ctypes.c_int,  # logQ size
                ctypes.POINTER(ctypes.c_int),  # logP array  
                ctypes.c_int,  # logP size
                ctypes.c_int,  # logScale
                ctypes.c_int,  # hammingWeight
                ctypes.c_char_p,  # ringType
                ctypes.c_char_p,  # keysPath
                ctypes.c_char_p,  # ioMode
            ],
            restype=None,
            name="InitializeOrionBackend"
        )

        self.DeleteScheme = OpenFHEFunction(
            self.lib.CleanupOrionBackend,
            argtypes=None,
            restype=None,
            name="CleanupOrionBackend"
        )

        self.IsSchemeInitialized = OpenFHEFunction(
            self.lib.IsSchemeInitialized,
            argtypes=[],
            restype=ctypes.c_int,
            name="IsSchemeInitialized"
        )

        self.AddRotationKey = OpenFHEFunction(
            self.lib.AddRotationKey,
            argtypes=[ctypes.c_int],
            restype=None,
            name="AddRotationKey"
        )

    def setup_tensor_functions(self):
        """Setup plaintext and ciphertext management functions"""
        # Plaintext operations
        self.CreatePlaintext = OpenFHEFunction(
            self.lib.CreatePlaintext,
            argtypes=[ctypes.POINTER(ctypes.c_double), ctypes.c_int],
            restype=ctypes.c_int,
            name="CreatePlaintext"
        )
        
        self.DeletePlaintext = OpenFHEFunction(
            self.lib.DeletePlaintextC,
            argtypes=[ctypes.c_int],
            restype=None,
            name="DeletePlaintextC"
        )

        self.GetPlaintextScale = OpenFHEFunction(
            self.lib.GetPlaintextScale,
            argtypes=[ctypes.c_int],
            restype=ctypes.c_double,
            name="GetPlaintextScale"
        )

        self.SetPlaintextScale = OpenFHEFunction(
            self.lib.SetPlaintextScale,
            argtypes=[ctypes.c_int, ctypes.c_double],
            restype=None,
            name="SetPlaintextScale"
        )

        self.GetPlaintextValues = OpenFHEFunction(
            self.lib.GetPlaintextValues,
            argtypes=[ctypes.c_int, ctypes.POINTER(ctypes.c_double), ctypes.c_int],
            restype=ctypes.c_int,
            name="GetPlaintextValues"
        )

        # Ciphertext operations
        self.DeleteCiphertext = OpenFHEFunction(
            self.lib.DeleteCiphertextC,
            argtypes=[ctypes.c_int],
            restype=None,
            name="DeleteCiphertextC"
        )

        self.GetCiphertextScale = OpenFHEFunction(
            self.lib.GetCiphertextScale,
            argtypes=[ctypes.c_int],
            restype=ctypes.c_double,
            name="GetCiphertextScale"
        )

        self.SetCiphertextScale = OpenFHEFunction(
            self.lib.SetCiphertextScale,
            argtypes=[ctypes.c_int, ctypes.c_double],
            restype=None,
            name="SetCiphertextScale"
        )

    def setup_encoder_functions(self):
        """Setup encoding and decoding functions"""
        self.NewEncoder = OpenFHEFunction(
            self.lib.NewEncoder,
            argtypes=[],
            restype=None,
            name="NewEncoder"
        )

        self.Encode = OpenFHEFunction(
            self.lib.Encode,
            argtypes=[ctypes.POINTER(ctypes.c_double), ctypes.c_int, ctypes.c_int, ctypes.c_ulong],
            restype=ctypes.c_int,
            name="Encode"
        )

        self.Decode = OpenFHEFunction(
            self.lib.Decode,
            argtypes=[ctypes.c_int, ctypes.POINTER(ctypes.c_double), ctypes.c_int],
            restype=ctypes.c_int,
            name="Decode"
        )

    def setup_encryptor_functions(self):
        """Setup encryption and decryption functions"""
        self.NewEncryptor = OpenFHEFunction(
            self.lib.NewEncryptor,
            argtypes=[],
            restype=None,
            name="NewEncryptor"
        )

        self.NewDecryptor = OpenFHEFunction(
            self.lib.NewDecryptor,
            argtypes=[],
            restype=None,
            name="NewDecryptor"
        )

        self.Encrypt = OpenFHEFunction(
            self.lib.Encrypt,
            argtypes=[ctypes.c_int],
            restype=ctypes.c_int,
            name="Encrypt"
        )

        self.Decrypt = OpenFHEFunction(
            self.lib.Decrypt,
            argtypes=[ctypes.c_int],
            restype=ctypes.c_int,
            name="Decrypt"
        )

    def setup_evaluator_functions(self):
        """Setup homomorphic evaluation functions"""
        self.Add = OpenFHEFunction(
            self.lib.Add,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="Add"
        )

        self.AddPlain = OpenFHEFunction(
            self.lib.AddPlain,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="AddPlain"
        )

        self.Multiply = OpenFHEFunction(
            self.lib.Multiply,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="Multiply"
        )

        self.MultiplyPlain = OpenFHEFunction(
            self.lib.MultiplyPlain,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="MultiplyPlain"
        )

        self.Rotate = OpenFHEFunction(
            self.lib.Rotate,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="Rotate"
        )

        self.Rescale = OpenFHEFunction(
            self.lib.Rescale,
            argtypes=[ctypes.c_int],
            restype=ctypes.c_int,
            name="Rescale"
        )

    def setup_linear_transform_functions(self):
        """Setup linear transformation functions"""
        self.CreateLinearTransform = OpenFHEFunction(
            self.lib.CreateLinearTransform,
            argtypes=[ctypes.POINTER(ctypes.c_double), ctypes.c_int],
            restype=ctypes.c_int,
            name="CreateLinearTransform"
        )

        self.DeleteLinearTransform = OpenFHEFunction(
            self.lib.DeleteLinearTransform,
            argtypes=[ctypes.c_int],
            restype=None,
            name="DeleteLinearTransform"
        )

        self.ApplyLinearTransform = OpenFHEFunction(
            self.lib.ApplyLinearTransform,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="ApplyLinearTransform"
        )

    def setup_utility_functions(self):
        """Setup utility functions"""
        self.FreeCArray = OpenFHEFunction(
            self.lib.FreeCArray,
            argtypes=[ctypes.c_void_p],
            restype=None,
            name="FreeCArray"
        )
        OpenFHEFunction.FreeCArray = self.FreeCArray

        self.GetMemoryUsage = OpenFHEFunction(
            self.lib.GetMemoryUsage,
            argtypes=[ctypes.POINTER(ctypes.c_int), ctypes.POINTER(ctypes.c_int)],
            restype=None,
            name="GetMemoryUsage"
        )

    def initialize_scheme(self, orion_params):
        """Initialize the cryptographic scheme with parameters"""
        try:
            # Extract parameters from orion_params
            logn = orion_params.get_logn()
            logq = orion_params.get_logq()
            logp = orion_params.get_logp()
            logscale = orion_params.get_logscale()
            h = orion_params.get_hamming_weight()
            ringtype = orion_params.get_ringtype()
            keys_path = orion_params.get_keys_path()
            io_mode = orion_params.get_io_mode()

            # Convert Python lists to C arrays
            logq_array = (ctypes.c_int * len(logq))(*logq) if logq else None
            logp_array = (ctypes.c_int * len(logp))(*logp) if logp else None
            
            # Initialize scheme
            self.NewScheme(
                logn, 
                logq_array, len(logq) if logq else 0, 
                logp_array, len(logp) if logp else 0,
                logscale, h, 
                ringtype.encode('utf-8') if ringtype else None,
                keys_path.encode('utf-8') if keys_path else None,
                io_mode.encode('utf-8') if io_mode else None
            )

            # Initialize components
            self.NewEncoder()
            self.NewEncryptor()
            self.NewDecryptor()

            # Check if initialization was successful
            if self.IsSchemeInitialized():
                self.initialized = True
                print("OpenFHE backend initialized successfully")
            else:
                raise RuntimeError("Scheme initialization failed")

        except Exception as e:
            print(f"Failed to initialize OpenFHE backend: {e}")
            raise

    # High-level helper functions matching Orion's interface
    
    def encode_and_encrypt(self, values: Union[List[float], np.ndarray]) -> int:
        """Encode values into plaintext and encrypt to ciphertext"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        if isinstance(values, (list, np.ndarray)):
            values = np.array(values, dtype=np.float64)
            values_ptr = values.ctypes.data_as(ctypes.POINTER(ctypes.c_double))
            
            # Create plaintext
            pt_id = self.CreatePlaintext(values_ptr, len(values))
            if pt_id < 0:
                raise RuntimeError("Failed to create plaintext")
            
            try:
                # Encrypt to ciphertext
                ct_id = self.Encrypt(pt_id)
                if ct_id < 0:
                    raise RuntimeError("Failed to encrypt plaintext")
                    
                return ct_id
            finally:
                # Clean up intermediate plaintext
                self.DeletePlaintext(pt_id)
        else:
            raise ValueError("Values must be a list or numpy array")

    def decrypt_and_decode(self, ct_id: int) -> List[float]:
        """Decrypt ciphertext and decode to values"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        # Decrypt to plaintext
        pt_id = self.Decrypt(ct_id)
        if pt_id < 0:
            raise RuntimeError("Failed to decrypt ciphertext")
        
        try:
            # Get values from plaintext
            max_size = 8192  # Reasonable default
            output_array = (ctypes.c_double * max_size)()
            actual_size = self.GetPlaintextValues(pt_id, output_array, max_size)
            
            if actual_size <= 0:
                raise RuntimeError("Failed to get plaintext values")
            
            # Convert to Python list
            values = [output_array[i] for i in range(actual_size)]
            return values
        finally:
            # Clean up plaintext
            self.DeletePlaintext(pt_id)

    def create_linear_transform_matrix(self, matrix: Union[List[float], np.ndarray]) -> int:
        """Create a linear transformation from matrix"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        if isinstance(matrix, (list, np.ndarray)):
            matrix = np.array(matrix, dtype=np.float64).flatten()
            matrix_ptr = matrix.ctypes.data_as(ctypes.POINTER(ctypes.c_double))
            
            transform_id = self.CreateLinearTransform(matrix_ptr, len(matrix))
            if transform_id < 0:
                raise RuntimeError("Failed to create linear transform")
            return transform_id
        else:
            raise ValueError("Matrix must be a list or numpy array")

    def apply_transform(self, ct_id: int, transform_id: int) -> int:
        """Apply linear transformation to ciphertext"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        result_id = self.ApplyLinearTransform(ct_id, transform_id)
        if result_id < 0:
            raise RuntimeError("Failed to apply linear transform")
        return result_id

    def get_memory_stats(self) -> dict:
        """Get memory usage statistics"""
        if not self.initialized:
            return {"plaintexts": 0, "ciphertexts": 0, "initialized": False}

        try:
            pt_count = ctypes.c_int()
            ct_count = ctypes.c_int()
            self.GetMemoryUsage(ctypes.byref(pt_count), ctypes.byref(ct_count))
            
            return {
                "plaintexts": pt_count.value,
                "ciphertexts": ct_count.value,
                "initialized": True
            }
        except Exception as e:
            print(f"Failed to get memory stats: {e}")
            return {"error": str(e), "initialized": self.initialized}

    def cleanup(self):
        """Clean up resources"""
        if self.initialized:
            try:
                self.DeleteScheme()
                self.initialized = False
                print("OpenFHE backend cleaned up successfully")
            except Exception as e:
                print(f"Error during cleanup: {e}")

    def __del__(self):
        """Destructor to ensure cleanup"""
        try:
            self.cleanup()
        except:
            pass  # Ignore errors during cleanup


class OpenFHEEvaluator:
    """High-level evaluator interface matching Orion's API"""
    
    def __init__(self, backend: OpenFHEBackend):
        self.backend = backend
        self.active_transforms = {}
    
    def linear_transformation(self, ct_tensor: Union[int, List[int]], transform_matrix: Union[List[float], np.ndarray]) -> Union[int, List[int]]:
        """Apply linear transformation to encrypted tensor"""
        if not self.backend.initialized:
            raise RuntimeError("Backend not initialized")

        # Create transform if not already cached
        transform_key = id(transform_matrix)
        if transform_key not in self.active_transforms:
            transform_id = self.backend.create_linear_transform_matrix(transform_matrix)
            self.active_transforms[transform_key] = transform_id
        else:
            transform_id = self.active_transforms[transform_key]
        
        # Apply transformation
        if isinstance(ct_tensor, list):
            result_ids = []
            for ct_id in ct_tensor:
                result_id = self.backend.apply_transform(ct_id, transform_id)
                result_ids.append(result_id)
            return result_ids
        else:
            return self.backend.apply_transform(ct_tensor, transform_id)
    
    def add(self, ct1: int, ct2: int) -> int:
        """Add two ciphertexts"""
        return self.backend.Add(ct1, ct2)
    
    def multiply(self, ct1: int, ct2: int) -> int:
        """Multiply two ciphertexts"""
        return self.backend.Multiply(ct1, ct2)
    
    def rotate(self, ct: int, steps: int) -> int:
        """Rotate ciphertext by steps"""
        return self.backend.Rotate(ct, steps)
    
    def rescale(self, ct: int) -> int:
        """Rescale ciphertext"""
        return self.backend.Rescale(ct)
    
    def delete_transforms(self):
        """Delete all cached linear transforms"""
        for transform_id in self.active_transforms.values():
            self.backend.DeleteLinearTransform(transform_id)
        self.active_transforms.clear()


# Example usage and testing
def test_openfhe_backend():
    """Test function for the modular OpenFHE backend"""
    
    class MockOrionParams:
        """Mock parameters for testing"""
        def get_logn(self): return 14
        def get_logq(self): return [60, 40, 40, 60]
        def get_logp(self): return [60]
        def get_logscale(self): return 40
        def get_hamming_weight(self): return 64
        def get_ringtype(self): return "standard"
        def get_keys_path(self): return "./keys"
        def get_io_mode(self): return "memory"
    
    try:
        print("=== Testing Modular OpenFHE Backend ===")
        
        params = MockOrionParams()
        backend = OpenFHEBackend(params)
        evaluator = OpenFHEEvaluator(backend)
        
        # Test basic encryption/decryption
        test_values1 = [1.0, 2.0, 3.0, 4.0]
        test_values2 = [5.0, 6.0, 7.0, 8.0]
        
        print(f"Original values 1: {test_values1}")
        print(f"Original values 2: {test_values2}")
        
        # Encrypt values
        ct_id1 = backend.encode_and_encrypt(test_values1)
        ct_id2 = backend.encode_and_encrypt(test_values2)
        print(f"Encrypted: ct1={ct_id1}, ct2={ct_id2}")

        # Test homomorphic multiplication
        ct_id3 = evaluator.multiply(ct_id1, ct_id2)
        print(f"Multiplication result: ct3={ct_id3}")

        # Test decryption
        decrypted_values = backend.decrypt_and_decode(ct_id3)
        print(f"Decrypted: {decrypted_values[:len(test_values1)]}")
        
        # Test linear transformation
        transform_matrix = [2.0, 1.0, 0.5, 0.25]
        transform_id = backend.create_linear_transform_matrix(transform_matrix)
        ct_id4 = backend.apply_transform(ct_id1, transform_id)
        
        transformed_values = backend.decrypt_and_decode(ct_id4)
        print(f"Transformed: {transformed_values[:len(test_values1)]}")
        
        # Test memory statistics
        stats = backend.get_memory_stats()
        print(f"Memory stats: {stats}")
        
        # Clean up
        backend.DeleteCiphertext(ct_id1)
        backend.DeleteCiphertext(ct_id2)
        backend.DeleteCiphertext(ct_id3)
        backend.DeleteCiphertext(ct_id4)
        backend.DeleteLinearTransform(transform_id)
        evaluator.delete_transforms()
        backend.cleanup()
        
        print("=== Test completed successfully! ===")
        
    except Exception as e:
        print(f"Test failed: {e}")
        import traceback
        traceback.print_exc()

if __name__ == "__main__":
    test_openfhe_backend()