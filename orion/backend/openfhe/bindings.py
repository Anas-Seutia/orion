# orion/backend/openfhe/bindings.py
# OpenFHE Python bindings for Orion compatibility

import ctypes
import numpy as np
from pathlib import Path
import os

class OpenFHEFunction:
    """Wrapper for OpenFHE C++ functions with error handling"""
    
    FreeCArray = None  # Will be set during initialization
    
    def __init__(self, func, argtypes=None, restype=None):
        self.func = func
        if argtypes is not None:
            self.func.argtypes = argtypes
        if restype is not None:
            self.func.restype = restype
    
    def __call__(self, *args):
        try:
            return self.func(*args)
        except Exception as e:
            print(f"OpenFHE function call failed: {e}")
            raise

class OpenFHEBackend:
    """OpenFHE backend implementation for Orion"""
    
    def __init__(self, orion_params):
        self.lib = None
        self.load_library()
        self.setup_scheme(orion_params)
        self.setup_tensor_binds()
        self.setup_encoder()
        self.setup_encryptor()
        self.setup_evaluator()
        self.setup_linear_transform()
        
    def load_library(self):
        """Load the OpenFHE shared library"""
        try:
            # Get the directory containing this Python file
            current_dir = Path(__file__).parent.absolute()

            # Try different possible library names and paths
            library_names = [
                'libopenfhe_orion.so',     # Linux
                'libopenfhe_orion.dylib',  # macOS
                'openfhe_orion.dll'        # Windows
            ]

            lib_paths = [
                current_dir / 'build',          # Our build directory
                current_dir,                    # Current directory
                current_dir / 'lib',           # lib subdirectory
                Path('/usr/local/lib'),        # System lib directory
            ]

            for path in lib_paths:
                for lib_name in library_names:
                    lib_path = path / lib_name
                    if lib_path.exists():
                        self.lib = ctypes.CDLL(str(lib_path))
                        print(f"Loaded OpenFHE library from: {lib_path}")
                        return

            # Print debugging information
            print("OpenFHE library search paths:")
            for path in lib_paths:
                print(f"  {path} - exists: {path.exists()}")
                if path.exists():
                    try:
                        files = list(path.glob("*openfhe*"))
                        print(f"    OpenFHE files found: {files}")
                    except:
                        pass

            raise FileNotFoundError(f"OpenFHE library not found. Searched: {library_names} in {lib_paths}")

        except Exception as e:
            raise RuntimeError(f"Failed to load OpenFHE library: {e}")

    def setup_scheme(self, orion_params):
        """Setup scheme initialization and management functions"""
        self.NewScheme = OpenFHEFunction(
            self.lib.NewScheme,
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
            restype=None
        )

        self.DeleteScheme = OpenFHEFunction(
            self.lib.DeleteScheme,
            argtypes=None,
            restype=None
        )

        self.FreeCArray = OpenFHEFunction(
            self.lib.FreeCArray,
            argtypes=[ctypes.c_void_p],
            restype=None
        )
        OpenFHEFunction.FreeCArray = self.FreeCArray

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
        logq_array = (ctypes.c_int * len(logq))(*logq)
        logp_array = (ctypes.c_int * len(logp))(*logp)
        
        # Initialize scheme
        self.NewScheme(
            logn, logq_array, len(logq), logp_array, len(logp),
            logscale, h, 
            ringtype.encode('utf-8') if ringtype else None,
            keys_path.encode('utf-8') if keys_path else None,
            io_mode.encode('utf-8') if io_mode else None
        )

    def setup_tensor_binds(self):
        """Setup plaintext and ciphertext management functions"""
        # Plaintext operations
        self.CreatePlaintext = OpenFHEFunction(
            self.lib.CreatePlaintext,
            argtypes=[ctypes.POINTER(ctypes.c_double), ctypes.c_int],
            restype=ctypes.c_int
        )
        
        self.DeletePlaintext = OpenFHEFunction(
            self.lib.DeletePlaintext,
            argtypes=[ctypes.c_int],
            restype=None
        )

        self.GetPlaintextScale = OpenFHEFunction(
            self.lib.GetPlaintextScale,
            argtypes=[ctypes.c_int],
            restype=ctypes.c_double
        )

        self.SetPlaintextScale = OpenFHEFunction(
            self.lib.SetPlaintextScale,
            argtypes=[ctypes.c_int, ctypes.c_double],
            restype=None
        )

        self.GetPlaintextValues = OpenFHEFunction(
            self.lib.GetPlaintextValues,
            argtypes=[ctypes.c_int, ctypes.POINTER(ctypes.c_double), ctypes.c_int],
            restype=ctypes.c_int
        )

        # Ciphertext operations
        self.DeleteCiphertext = OpenFHEFunction(
            self.lib.DeleteCiphertext,
            argtypes=[ctypes.c_int],
            restype=None
        )

        self.GetCiphertextScale = OpenFHEFunction(
            self.lib.GetCiphertextScale,
            argtypes=[ctypes.c_int],
            restype=ctypes.c_double
        )

        self.SetCiphertextScale = OpenFHEFunction(
            self.lib.SetCiphertextScale,
            argtypes=[ctypes.c_int, ctypes.c_double],
            restype=None
        )

    def setup_encoder(self):
        """Setup encoding functions"""
        pass  # Encoding is handled internally in OpenFHE

    def setup_encryptor(self):
        """Setup encryption and decryption functions"""
        self.Encrypt = OpenFHEFunction(
            self.lib.Encrypt,
            argtypes=[ctypes.c_int],
            restype=ctypes.c_int
        )

        self.Decrypt = OpenFHEFunction(
            self.lib.Decrypt,
            argtypes=[ctypes.c_int],
            restype=ctypes.c_int
        )

    def setup_evaluator(self):
        """Setup homomorphic evaluation functions"""
        self.Add = OpenFHEFunction(
            self.lib.Add,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int
        )

        self.AddPlain = OpenFHEFunction(
            self.lib.AddPlain,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int
        )

        self.Multiply = OpenFHEFunction(
            self.lib.Multiply,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int
        )

        self.MultiplyPlain = OpenFHEFunction(
            self.lib.MultiplyPlain,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int
        )

        self.Rotate = OpenFHEFunction(
            self.lib.Rotate,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int
        )

        self.Rescale = OpenFHEFunction(
            self.lib.Rescale,
            argtypes=[ctypes.c_int],
            restype=ctypes.c_int
        )

    def setup_linear_transform(self):
        """Setup linear transformation functions"""
        self.CreateLinearTransform = OpenFHEFunction(
            self.lib.CreateLinearTransform,
            argtypes=[ctypes.POINTER(ctypes.c_double), ctypes.c_int],
            restype=ctypes.c_int
        )

        self.DeleteLinearTransform = OpenFHEFunction(
            self.lib.DeleteLinearTransform,
            argtypes=[ctypes.c_int],
            restype=None
        )

        self.ApplyLinearTransform = OpenFHEFunction(
            self.lib.ApplyLinearTransform,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int
        )

    # High-level helper functions matching Orion's interface
    
    def encode_and_encrypt(self, values):
        """Encode values into plaintext and encrypt to ciphertext"""
        if isinstance(values, (list, np.ndarray)):
            values = np.array(values, dtype=np.float64)
            values_ptr = values.ctypes.data_as(ctypes.POINTER(ctypes.c_double))
            
            # Create plaintext
            pt_id = self.CreatePlaintext(values_ptr, len(values))
            if pt_id < 0:
                raise RuntimeError("Failed to create plaintext")
            
            # Encrypt to ciphertext
            ct_id = self.Encrypt(pt_id)
            if ct_id < 0:
                self.DeletePlaintext(pt_id)
                raise RuntimeError("Failed to encrypt plaintext")
                
            # Clean up intermediate plaintext
            self.DeletePlaintext(pt_id)
            return ct_id
        else:
            raise ValueError("Values must be a list or numpy array")

    def decrypt_and_decode(self, ct_id):
        """Decrypt ciphertext and decode to values"""
        # Decrypt to plaintext
        pt_id = self.Decrypt(ct_id)
        if pt_id < 0:
            raise RuntimeError("Failed to decrypt ciphertext")
        
        try:
            # Get values from plaintext
            max_size = 1024  # Reasonable default
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

    def create_linear_transform_matrix(self, matrix):
        """Create a linear transformation from matrix"""
        if isinstance(matrix, (list, np.ndarray)):
            matrix = np.array(matrix, dtype=np.float64).flatten()
            matrix_ptr = matrix.ctypes.data_as(ctypes.POINTER(ctypes.c_double))
            
            transform_id = self.CreateLinearTransform(matrix_ptr, len(matrix))
            if transform_id < 0:
                raise RuntimeError("Failed to create linear transform")
            return transform_id
        else:
            raise ValueError("Matrix must be a list or numpy array")

    def apply_transform(self, ct_id, transform_id):
        """Apply linear transformation to ciphertext"""
        result_id = self.ApplyLinearTransform(ct_id, transform_id)
        if result_id < 0:
            raise RuntimeError("Failed to apply linear transform")
        return result_id

    def cleanup(self):
        """Clean up resources"""
        if hasattr(self, 'DeleteScheme'):
            self.DeleteScheme()

    def __del__(self):
        """Destructor to ensure cleanup"""
        try:
            self.cleanup()
        except:
            pass  # Ignore errors during cleanup


class OpenFHEEvaluator:
    """High-level evaluator interface matching Orion's API"""
    
    def __init__(self, backend):
        self.backend = backend
        self.active_transforms = {}
    
    def linear_transformation(self, ct_tensor, transform_matrix):
        """Apply linear transformation to encrypted tensor"""
        # Create transform if not already cached
        transform_key = id(transform_matrix)
        if transform_key not in self.active_transforms:
            transform_id = self.backend.create_linear_transform_matrix(transform_matrix)
            self.active_transforms[transform_key] = transform_id
        else:
            transform_id = self.active_transforms[transform_key]
        
        # Apply transformation
        result_ids = []
        if isinstance(ct_tensor, list):
            for ct_id in ct_tensor:
                result_id = self.backend.apply_transform(ct_id, transform_id)
                result_ids.append(result_id)
        else:
            result_id = self.backend.apply_transform(ct_tensor, transform_id)
            result_ids = [result_id]
        
        return result_ids if len(result_ids) > 1 else result_ids[0]
    
    def add(self, ct1, ct2):
        """Add two ciphertexts"""
        return self.backend.Add(ct1, ct2)
    
    def multiply(self, ct1, ct2):
        """Multiply two ciphertexts"""
        return self.backend.Multiply(ct1, ct2)
    
    def rotate(self, ct, steps):
        """Rotate ciphertext by steps"""
        return self.backend.Rotate(ct, steps)
    
    def rescale(self, ct):
        """Rescale ciphertext"""
        return self.backend.Rescale(ct)
    
    def delete_transforms(self, transform_ids):
        """Delete linear transforms"""
        for key, transform_id in transform_ids.items():
            self.backend.DeleteLinearTransform(transform_id)
            if key in self.active_transforms:
                del self.active_transforms[key]


# Example usage and testing
def test_openfhe_backend():
    """Test function for the OpenFHE backend"""
    
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
        params = MockOrionParams()
        backend = OpenFHEBackend(params)
        evaluator = OpenFHEEvaluator(backend)
        
        # Test basic encryption/decryption
        test_values = [1.0, 2.0, 3.0, 4.0]
        print(f"Original: {test_values}")
        ct_id1 = backend.encode_and_encrypt(test_values)
        
        ct_id2 = backend.encode_and_encrypt([5.0, 6.0, 7.0, 8.0])

        ct_id3 = evaluator.multiply(ct_id1, ct_id2)

        # Test decryption
        decrypted_values = backend.decrypt_and_decode(ct_id3)
        print(f"Decrypted: {decrypted_values[:len(test_values)]}")
        
        # Clean up
        backend.DeleteCiphertext(ct_id1)
        backend.DeleteCiphertext(ct_id2)
        backend.DeleteCiphertext(ct_id3)
        backend.cleanup()
        
        print("OpenFHE backend test completed successfully!")
        
    except Exception as e:
        print(f"Test failed: {e}")

if __name__ == "__main__":
    test_openfhe_backend()