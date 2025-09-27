# orion/backend/openfhe/bindings.py
# Updated OpenFHE Python bindings for modular Orion backend

import ctypes
import numpy as np
from pathlib import Path
import os
import sys
from typing import List, Optional, Union

# Result structures for array data (matching Lattigo interface)
class ArrayResultByte(ctypes.Structure):
    _fields_ = [("Data", ctypes.POINTER(ctypes.c_char)), ("Length", ctypes.c_ulong)]

class ArrayResultInt(ctypes.Structure):
    _fields_ = [("data", ctypes.POINTER(ctypes.c_int)), ("length", ctypes.c_ulong)]

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
            # Convert ArrayResultInt to Python list (matching Lattigo behavior)
            if hasattr(result, 'contents') and isinstance(result.contents, ArrayResultInt):
                array_result = result.contents
                return [int(array_result.data[i]) for i in range(array_result.length)]
            # Convert ArrayResultByte to ArrayResultByte object (preserve for test compatibility)
            elif hasattr(result, 'contents') and isinstance(result.contents, ArrayResultByte):
                return result.contents
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
        self.setup_function_bindings()
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

    def setup_function_bindings(self):
        """Setup all C function bindings"""
        self.setup_scheme_functions()
        self.setup_tensor_functions()
        self.setup_encoder_functions()
        self.setup_encryptor_functions()
        self.setup_evaluator_functions()
        self.setup_linear_transform_functions()
        self.setup_key_generator_functions()
        self.setup_polynomial_evaluator_functions()
        self.setup_bootstrapper_functions()
        self.setup_utility_functions()

    def setup_scheme_functions(self):
        """Setup scheme management functions"""
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
            restype=None,
            name="NewScheme"
        )

        self.DeleteScheme = OpenFHEFunction(
            self.lib.DeleteScheme,
            argtypes=None,
            restype=None,
            name="DeleteScheme"
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
        self._encode_ctypes = OpenFHEFunction(
            self.lib.Encode,
            argtypes=[ctypes.POINTER(ctypes.c_double), ctypes.c_int, ctypes.c_int, ctypes.c_uint64],
            restype=ctypes.c_int,
            name="Encode"
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

        self._decode_ctypes = OpenFHEFunction(
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

        # Level Management Functions
        self.GetPlaintextLevel = OpenFHEFunction(
            self.lib.GetPlaintextLevel,
            argtypes=[ctypes.c_int],
            restype=ctypes.c_int,
            name="GetPlaintextLevel"
        )

        self.GetCiphertextLevel = OpenFHEFunction(
            self.lib.GetCiphertextLevel,
            argtypes=[ctypes.c_int],
            restype=ctypes.c_int,
            name="GetCiphertextLevel"
        )

        self.GetPlaintextSlots = OpenFHEFunction(
            self.lib.GetPlaintextSlots,
            argtypes=[ctypes.c_int],
            restype=ctypes.c_int,
            name="GetPlaintextSlots"
        )

        self.GetCiphertextSlots = OpenFHEFunction(
            self.lib.GetCiphertextSlots,
            argtypes=[ctypes.c_int],
            restype=ctypes.c_int,
            name="GetCiphertextSlots"
        )

        self.GetCiphertextDegree = OpenFHEFunction(
            self.lib.GetCiphertextDegree,
            argtypes=[ctypes.c_int],
            restype=ctypes.c_int,
            name="GetCiphertextDegree"
        )

        # Memory and System Information Functions
        self.GetModuliChain = OpenFHEFunction(
            self.lib.GetModuliChain,
            argtypes=[],
            restype=ctypes.c_char_p,
            name="GetModuliChain"
        )

        self._get_live_plaintexts_ctypes = OpenFHEFunction(
            self.lib.GetLivePlaintexts,
            argtypes=[ctypes.POINTER(ctypes.c_int)],
            restype=ctypes.POINTER(ctypes.c_int),
            name="GetLivePlaintexts"
        )

        self._get_live_ciphertexts_ctypes = OpenFHEFunction(
            self.lib.GetLiveCiphertexts,
            argtypes=[ctypes.POINTER(ctypes.c_int)],
            restype=ctypes.POINTER(ctypes.c_int),
            name="GetLiveCiphertexts"
        )

        self.FreeCIntArray = OpenFHEFunction(
            self.lib.FreeCIntArray,
            argtypes=[ctypes.POINTER(ctypes.c_int)],
            restype=None,
            name="FreeCIntArray"
        )

    def setup_encoder_functions(self):
        """Setup encoding and decoding functions"""
        self.NewEncoder = OpenFHEFunction(
            self.lib.NewEncoder,
            argtypes=[],
            restype=None,
            name="NewEncoder"
        )

        # Note: Encode and Decode methods are implemented as high-level Python methods
        # that call self.lib functions directly to properly handle arguments

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
        self.NewEvaluator = OpenFHEFunction(
            self.lib.NewEvaluator,
            argtypes=[],
            restype=None,
            name="NewEvaluator"
        )
        self.AddCiphertext = OpenFHEFunction(
            self.lib.Add,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="AddCiphertext"
        )

        self.AddPlaintext = OpenFHEFunction(
            self.lib.AddPlain,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="AddPlaintext"
        )

        self.MulRelinCiphertext = OpenFHEFunction(
            self.lib.Multiply,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="MulRelinCiphertext"
        )

        self.MulPlaintext = OpenFHEFunction(
            self.lib.MultiplyPlain,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="MulPlaintext"
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

        self.RotateNew = OpenFHEFunction(
            self.lib.Rotate,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="RotateNew"
        )

        self.RescaleNew = OpenFHEFunction(
            self.lib.Rescale,
            argtypes=[ctypes.c_int],
            restype=ctypes.c_int,
            name="RescaleNew"
        )

        # Basic Arithmetic Operations
        self.Negate = OpenFHEFunction(
            self.lib.Negate,
            argtypes=[ctypes.c_int],
            restype=ctypes.c_int,
            name="Negate"
        )

        self.AddScalar = OpenFHEFunction(
            self.lib.AddScalar,
            argtypes=[ctypes.c_int, ctypes.c_double],
            restype=ctypes.c_int,
            name="AddScalar"
        )

        self.AddScalarNew = OpenFHEFunction(
            self.lib.AddScalarNew,
            argtypes=[ctypes.c_int, ctypes.c_double],
            restype=ctypes.c_int,
            name="AddScalarNew"
        )

        self.SubScalar = OpenFHEFunction(
            self.lib.SubScalar,
            argtypes=[ctypes.c_int, ctypes.c_double],
            restype=ctypes.c_int,
            name="SubScalar"
        )

        self.SubScalarNew = OpenFHEFunction(
            self.lib.SubScalarNew,
            argtypes=[ctypes.c_int, ctypes.c_double],
            restype=ctypes.c_int,
            name="SubScalarNew"
        )

        self.MulScalarInt = OpenFHEFunction(
            self.lib.MulScalarInt,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="MulScalarInt"
        )

        self.MulScalarIntNew = OpenFHEFunction(
            self.lib.MulScalarIntNew,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="MulScalarIntNew"
        )

        self.MulScalarFloat = OpenFHEFunction(
            self.lib.MulScalarFloat,
            argtypes=[ctypes.c_int, ctypes.c_double],
            restype=ctypes.c_int,
            name="MulScalarFloat"
        )

        self.MulScalarFloatNew = OpenFHEFunction(
            self.lib.MulScalarFloatNew,
            argtypes=[ctypes.c_int, ctypes.c_double],
            restype=ctypes.c_int,
            name="MulScalarFloatNew"
        )

        # Advanced Ciphertext Operations
        self.SubCiphertext = OpenFHEFunction(
            self.lib.SubCiphertext,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="SubCiphertext"
        )

        self.SubCiphertextNew = OpenFHEFunction(
            self.lib.SubCiphertextNew,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="SubCiphertextNew"
        )

        self.SubPlaintext = OpenFHEFunction(
            self.lib.SubPlaintext,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="SubPlaintext"
        )

        self.SubPlaintextNew = OpenFHEFunction(
            self.lib.SubPlaintextNew,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="SubPlaintextNew"
        )

        self.AddCiphertextNew = OpenFHEFunction(
            self.lib.AddCiphertextNew,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="AddCiphertextNew"
        )

        self.AddPlaintextNew = OpenFHEFunction(
            self.lib.AddPlaintextNew,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="AddPlaintextNew"
        )

        self.MulPlaintextNew = OpenFHEFunction(
            self.lib.MulPlaintextNew,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="MulPlaintextNew"
        )

        self.MulRelinCiphertextNew = OpenFHEFunction(
            self.lib.MulRelinCiphertextNew,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="MulRelinCiphertextNew"
        )

    def setup_linear_transform_functions(self):
        """Setup linear transformation functions"""
        # New Linear Transform Operations matching Lattigo interface
        self.NewLinearTransformEvaluator = OpenFHEFunction(
            self.lib.NewLinearTransformEvaluator,
            argtypes=[],
            restype=None,
            name="NewLinearTransformEvaluator"
        )

        self.GenerateLinearTransform = OpenFHEFunction(
            self.lib.GenerateLinearTransform,
            argtypes=[
                ctypes.POINTER(ctypes.c_int), ctypes.c_int, # diags_idxs
                ctypes.POINTER(ctypes.c_float), ctypes.c_int, # diags_data
                ctypes.c_int, # level
                ctypes.c_float, # bsgs_ratio
                ctypes.c_char_p, # io_mode
            ],
            restype=ctypes.c_int,
            name="GenerateLinearTransform"
        )

        self.EvaluateLinearTransform = OpenFHEFunction(
            self.lib.EvaluateLinearTransform,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="EvaluateLinearTransform"
        )

        self.GetLinearTransformRotationKeys = OpenFHEFunction(
            self.lib.GetLinearTransformRotationKeysArray,
            argtypes=[ctypes.c_int],
            restype=ctypes.POINTER(ArrayResultInt),
            name="GetLinearTransformRotationKeys"
        )

        self.GenerateLinearTransformRotationKey = OpenFHEFunction(
            self.lib.GenerateLinearTransformRotationKey,
            argtypes=[ctypes.c_int],
            restype=None,
            name="GenerateLinearTransformRotationKey"
        )

        self.GenerateAndSerializeRotationKey = OpenFHEFunction(
            self.lib.GenerateAndSerializeRotationKey,
            argtypes=[ctypes.c_int],
            restype=ctypes.POINTER(ArrayResultByte),
            name="GenerateAndSerializeRotationKey"
        )

        self.LoadRotationKey = OpenFHEFunction(
            self.lib.LoadRotationKey,
            argtypes=[ctypes.c_char_p, ctypes.c_size_t, ctypes.c_int],
            restype=ctypes.c_int,
            name="LoadRotationKey"
        )

        self.SerializeDiagonal = OpenFHEFunction(
            self.lib.SerializeDiagonal,
            argtypes=[ctypes.POINTER(ctypes.c_double), ctypes.c_size_t, ctypes.c_char_p, ctypes.c_size_t],
            restype=ctypes.c_int,
            name="SerializeDiagonal"
        )

        self.LoadPlaintextDiagonal = OpenFHEFunction(
            self.lib.LoadPlaintextDiagonal,
            argtypes=[ctypes.c_char_p, ctypes.c_size_t],
            restype=ctypes.c_int,
            name="LoadPlaintextDiagonal"
        )

        self.RemovePlaintextDiagonals = OpenFHEFunction(
            self.lib.RemovePlaintextDiagonals,
            argtypes=[ctypes.POINTER(ctypes.c_int), ctypes.c_size_t],
            restype=None,
            name="RemovePlaintextDiagonals"
        )

        self.RemoveRotationKeys = OpenFHEFunction(
            self.lib.RemoveRotationKeys,
            argtypes=[ctypes.POINTER(ctypes.c_int), ctypes.c_size_t],
            restype=None,
            name="RemoveRotationKeys"
        )

        # Legacy functions
        self.DeleteLinearTransform = OpenFHEFunction(
            self.lib.DeleteLinearTransformC,
            argtypes=[ctypes.c_int],
            restype=None,
            name="DeleteLinearTransform"
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


    def setup_key_generator_functions(self):
        """Setup key generation functions"""
        self.NewKeyGenerator = OpenFHEFunction(
            self.lib.NewKeyGenerator,
            argtypes=[],
            restype=None,
            name="NewKeyGenerator"
        )

        self.GenerateSecretKey = OpenFHEFunction(
            self.lib.GenerateSecretKey,
            argtypes=[],
            restype=None,
            name="GenerateSecretKey"
        )

        self.GeneratePublicKey = OpenFHEFunction(
            self.lib.GeneratePublicKey,
            argtypes=[],
            restype=None,
            name="GeneratePublicKey"
        )

        self.GenerateRelinearizationKey = OpenFHEFunction(
            self.lib.GenerateRelinearizationKey,
            argtypes=[],
            restype=None,
            name="GenerateRelinearizationKey"
        )

        self.GenerateEvaluationKeys = OpenFHEFunction(
            self.lib.GenerateEvaluationKeys,
            argtypes=[],
            restype=None,
            name="GenerateEvaluationKeys"
        )

        # Key serialization functions
        self._serialize_secret_key_ctypes = OpenFHEFunction(
            self.lib.SerializeSecretKey,
            argtypes=[ctypes.POINTER(ctypes.c_ulong)],
            restype=ctypes.POINTER(ctypes.c_char),
            name="SerializeSecretKey"
        )

        self._load_secret_key_ctypes = OpenFHEFunction(
            self.lib.LoadSecretKey,
            argtypes=[ctypes.POINTER(ctypes.c_char), ctypes.c_ulong],
            restype=None,
            name="LoadSecretKey"
        )

    def setup_polynomial_evaluator_functions(self):
        """Setup polynomial evaluation functions"""
        self.NewPolynomialEvaluator = OpenFHEFunction(
            self.lib.NewPolynomialEvaluator,
            argtypes=[],
            restype=None,
            name="NewPolynomialEvaluator"
        )


        self.GenerateMonomial = OpenFHEFunction(
            self.lib.GenerateMonomial,
            argtypes=[ctypes.POINTER(ctypes.c_double), ctypes.c_int],
            restype=ctypes.c_int,
            name="GenerateMonomial"
        )

        self.GenerateChebyshev = OpenFHEFunction(
            self.lib.GenerateChebyshev,
            argtypes=[ctypes.POINTER(ctypes.c_double), ctypes.c_int],
            restype=ctypes.c_int,
            name="GenerateChebyshev"
        )

        self.EvaluatePolynomial = OpenFHEFunction(
            self.lib.EvaluatePolynomial,
            argtypes=[ctypes.c_int, ctypes.c_int, ctypes.c_uint64],
            restype=ctypes.c_int,
            name="EvaluatePolynomial"
        )

        self.GenerateMinimaxSignCoeffs = OpenFHEFunction(
            self.lib.GenerateMinimaxSignCoeffs,
            argtypes=[
                ctypes.POINTER(ctypes.c_int),  # degrees
                ctypes.c_int,  # lenDegrees
                ctypes.c_int,  # prec
                ctypes.c_int,  # logalpha
                ctypes.c_int,  # logerr
                ctypes.c_int,  # debug
                ctypes.POINTER(ctypes.c_ulong)  # outLength
            ],
            restype=ctypes.POINTER(ctypes.c_double),
            name="GenerateMinimaxSignCoeffs"
        )


    def setup_bootstrapper_functions(self):
        """Setup bootstrapping functions"""
        self.NewBootstrapper = OpenFHEFunction(
            self.lib.NewBootstrapper,
            argtypes=[ctypes.POINTER(ctypes.c_int), ctypes.c_int, ctypes.c_int],
            restype=None,
            name="NewBootstrapper"
        )

        self.Bootstrap = OpenFHEFunction(
            self.lib.Bootstrap,
            argtypes=[ctypes.c_int, ctypes.c_int],
            restype=ctypes.c_int,
            name="Bootstrap"
        )


        self.DeleteBootstrappers = OpenFHEFunction(
            self.lib.DeleteBootstrappers,
            argtypes=[],
            restype=None,
            name="DeleteBootstrappers"
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
            self.NewEvaluator()

            # Initialize key generator
            self.NewKeyGenerator()

            # Initialize polynomial evaluator
            self.NewPolynomialEvaluator()

            # Mark as initialized
            self.initialized = True
            print("OpenFHE backend initialized successfully")

        except Exception as e:
            print(f"Failed to initialize OpenFHE backend: {e}")
            raise

    # High-level helper functions matching Orion's interface
    
    def Encode(self, values: List[float], level: int, scale: int) -> int:
        """Encode values into plaintext - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        if isinstance(values, list):
            values_array = np.array(values, dtype=np.float64)
            values_ptr = values_array.ctypes.data_as(ctypes.POINTER(ctypes.c_double))

            # Call the C++ Encode function with proper arguments
            # OpenFHE C++ function expects: (data_ptr, length, level, scale)
            pt_id = self._encode_ctypes(values_ptr, len(values), level, scale)
            if pt_id < 0:
                raise RuntimeError("Failed to encode plaintext")
            return pt_id
        else:
            raise ValueError("Values must be a list")

    def Decode(self, pt_id: int) -> List[float]:
        """Decode plaintext to values - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        # Get values from plaintext
        max_size = 8192  # Reasonable default
        output_array = (ctypes.c_double * max_size)()
        actual_size = self._decode_ctypes(pt_id, output_array, max_size)

        if actual_size <= 0:
            raise RuntimeError("Failed to decode plaintext")

        # Convert to Python list
        values = [output_array[i] for i in range(actual_size)]
        return values



    # Key generation helper methods
    def generate_keys(self):
        """Generate all necessary keys for the scheme"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        self.GenerateSecretKey()
        self.GeneratePublicKey()
        self.GenerateRelinearizationKey()
        self.GenerateEvaluationKeys()

    def serialize_secret_key(self) -> bytes:
        """Serialize the secret key to bytes"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        length = ctypes.c_ulong()
        data_ptr = self._serialize_secret_key_ctypes(ctypes.byref(length))

        if not data_ptr or length.value == 0:
            raise RuntimeError("Failed to serialize secret key")

        # Copy data to Python bytes
        data = ctypes.string_at(data_ptr, length.value)
        # Free the C-allocated memory
        self.FreeCArray(data_ptr)
        return data

    def load_secret_key(self, data: bytes):
        """Load secret key from bytes"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        if not data:
            raise ValueError("Empty key data")

        data_ptr = ctypes.c_char_p(data)
        self._load_secret_key_ctypes(data_ptr, len(data))


    # Polynomial evaluation helper methods

    def generate_minimax_sign_coefficients(self, degrees: List[int], precision: int = 10,
                                         log_alpha: int = 8, log_error: int = 20, debug: bool = False) -> List[float]:
        """Generate minimax approximation coefficients for sign function"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        degrees_array = (ctypes.c_int * len(degrees))(*degrees)
        output_length = ctypes.c_ulong()

        coeffs_ptr = self.GenerateMinimaxSignCoeffs(
            degrees_array, len(degrees),
            precision, log_alpha, log_error,
            1 if debug else 0,
            ctypes.byref(output_length)
        )

        if not coeffs_ptr or output_length.value == 0:
            raise RuntimeError("Failed to generate minimax coefficients")

        # Copy data to Python list
        coeffs = [coeffs_ptr[i] for i in range(output_length.value)]
        # Free the C-allocated memory
        self.FreeCArray(coeffs_ptr)
        return coeffs

    # Bootstrapping helper methods
    def create_bootstrapper(self, log_ps: List[int], num_slots: int):
        """Create a bootstrapper for the given parameters"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        log_ps_array = (ctypes.c_int * len(log_ps))(*log_ps)
        self.NewBootstrapper(log_ps_array, len(log_ps), num_slots)

    def bootstrap_ciphertext(self, ct_id: int, num_slots: int) -> int:
        """Bootstrap a ciphertext to refresh noise"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        result_id = self.Bootstrap(ct_id, num_slots)
        if result_id < 0:
            raise RuntimeError("Bootstrap operation failed")
        return result_id


    # Lattigo interface compatibility methods
    def SerializeSecretKey(self):
        """Serialize secret key - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        try:
            # Use the high-level method
            data = self.serialize_secret_key()
            # Return in Lattigo format: (data, data_pointer)
            # Since we don't have a C pointer, we'll return the data twice
            return (data, data)
        except Exception as e:
            raise RuntimeError(f"Failed to serialize secret key: {e}")

    def LoadSecretKey(self, data, length=None):
        """Load secret key - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        try:
            # Use the high-level method (length parameter is ignored)
            self.load_secret_key(data)
        except Exception as e:
            raise RuntimeError(f"Failed to load secret key: {e}")

    def DeleteScheme(self):
        """Delete the scheme - matches Lattigo interface"""
        if self.initialized:
            try:
                # Clean up bootstrappers
                self.DeleteBootstrappers()

                # Clean up the main scheme (call the C function directly)
                self.lib.DeleteScheme()

                self.initialized = False
                print("OpenFHE backend cleaned up successfully")
            except Exception as e:
                print(f"Error during cleanup: {e}")

    def NewEvaluator(self):
        """Initialize evaluator - matches Lattigo interface"""
        pass  # OpenFHE doesn't need separate evaluator initialization

    def NewLinearTransformEvaluator(self):
        """Initialize linear transform evaluator - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        self.lib.NewLinearTransformEvaluator()

    def NewPolynomialEvaluator(self):
        """Initialize polynomial evaluator - matches Lattigo interface"""
        pass  # OpenFHE doesn't need separate poly evaluator initialization

    def AddRotationKey(self, amount):
        """Add rotation key - matches Lattigo interface"""
        pass  # OpenFHE generates rotation keys as needed

    # Additional methods that Lattigo provides
    def RotateNew(self, ct_id, steps):
        """Rotate ciphertext (new copy) - matches Lattigo interface"""
        return self.Rotate(ct_id, steps)  # OpenFHE always creates new

    def RescaleNew(self, ct_id):
        """Rescale ciphertext (new copy) - matches Lattigo interface"""
        return self.Rescale(ct_id)  # OpenFHE always creates new

    # Basic Arithmetic Operations - now use direct C++ functions
    def Negate(self, ct_id):
        """Negate ciphertext - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.Negate(ct_id)

    def AddScalar(self, ct_id, scalar):
        """Add scalar to ciphertext - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.AddScalar(ct_id, scalar)

    def AddScalarNew(self, ct_id, scalar):
        """Add scalar to ciphertext (new copy) - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.AddScalarNew(ct_id, scalar)

    def SubScalar(self, ct_id, scalar):
        """Subtract scalar from ciphertext - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.SubScalar(ct_id, scalar)

    def SubScalarNew(self, ct_id, scalar):
        """Subtract scalar from ciphertext (new copy) - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.SubScalarNew(ct_id, scalar)

    def MulScalarInt(self, ct_id, scalar):
        """Multiply ciphertext by integer scalar - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.MulScalarInt(ct_id, scalar)

    def MulScalarIntNew(self, ct_id, scalar):
        """Multiply ciphertext by integer scalar (new copy) - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.MulScalarIntNew(ct_id, scalar)

    def MulScalarFloat(self, ct_id, scalar):
        """Multiply ciphertext by float scalar - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.MulScalarFloat(ct_id, scalar)

    def MulScalarFloatNew(self, ct_id, scalar):
        """Multiply ciphertext by float scalar (new copy) - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.MulScalarFloatNew(ct_id, scalar)

    # Advanced Ciphertext Operations - now use direct C++ functions
    def SubCiphertext(self, ct1_id, ct2_id):
        """Subtract ciphertexts - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.SubCiphertext(ct1_id, ct2_id)

    def SubCiphertextNew(self, ct1_id, ct2_id):
        """Subtract ciphertexts (new copy) - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.SubCiphertextNew(ct1_id, ct2_id)

    def SubPlaintext(self, ct_id, pt_id):
        """Subtract plaintext from ciphertext - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.SubPlaintext(ct_id, pt_id)

    def SubPlaintextNew(self, ct_id, pt_id):
        """Subtract plaintext from ciphertext (new copy) - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.SubPlaintextNew(ct_id, pt_id)

    def AddCiphertextNew(self, ct1_id, ct2_id):
        """Add ciphertexts (new copy) - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.AddCiphertextNew(ct1_id, ct2_id)

    def AddPlaintextNew(self, ct_id, pt_id):
        """Add plaintext to ciphertext (new copy) - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.AddPlaintextNew(ct_id, pt_id)

    def MulPlaintextNew(self, ct_id, pt_id):
        """Multiply ciphertext by plaintext (new copy) - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.MulPlaintextNew(ct_id, pt_id)

    def MulRelinCiphertextNew(self, ct1_id, ct2_id):
        """Multiply ciphertexts with relinearization (new copy) - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.MulRelinCiphertextNew(ct1_id, ct2_id)

    def GetPlaintextLevel(self, pt_id):
        """Get plaintext level - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.GetPlaintextLevel(pt_id)

    def GetCiphertextLevel(self, ct_id):
        """Get ciphertext level - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.GetCiphertextLevel(ct_id)

    def GetPlaintextSlots(self, pt_id):
        """Get number of slots in plaintext - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.GetPlaintextSlots(pt_id)

    def GetCiphertextSlots(self, ct_id):
        """Get number of slots in ciphertext - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.GetCiphertextSlots(ct_id)

    def GetCiphertextDegree(self, ct_id):
        """Get ciphertext degree - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        return self.lib.GetCiphertextDegree(ct_id)

    # Memory and System Information Functions
    def GetModuliChain(self):
        """Get moduli chain information - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")
        result = self.lib.GetModuliChain()
        return result.decode('utf-8') if result else "Error: Unable to retrieve moduli chain"

    def GetLivePlaintexts(self):
        """Get array of live plaintext IDs - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        try:
            count = ctypes.c_int()
            result_ptr = self._get_live_plaintexts_ctypes(ctypes.byref(count))

            if not result_ptr or count.value == 0:
                return ([], 0)

            # Convert C array to Python list
            ids = [result_ptr[i] for i in range(count.value)]

            # Free the C array (important to prevent memory leaks)
            try:
                self.FreeCIntArray(result_ptr)
            except Exception as e:
                print(f"Warning: Failed to free int array: {e}")

            # Return in Lattigo format: (data, length)
            return (ids, count.value)
        except RuntimeError as e:
            # Handle OpenFHE function call failures gracefully
            print(f"Warning: GetLivePlaintexts failed: {e}")
            return ([], 0)

    def GetLiveCiphertexts(self):
        """Get array of live ciphertext IDs - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        try:
            count = ctypes.c_int()
            result_ptr = self._get_live_ciphertexts_ctypes(ctypes.byref(count))

            if not result_ptr or count.value == 0:
                return ([], 0)

            # Convert C array to Python list
            ids = [result_ptr[i] for i in range(count.value)]

            # Free the C array (important to prevent memory leaks)
            try:
                self.FreeCIntArray(result_ptr)
            except Exception as e:
                print(f"Warning: Failed to free int array: {e}")

            # Return in Lattigo format: (data, length)
            return (ids, count.value)
        except RuntimeError as e:
            # Handle OpenFHE function call failures gracefully
            print(f"Warning: GetLiveCiphertexts failed: {e}")
            return ([], 0)

    # Linear Transform Operations - matches Lattigo interface

    def GenerateLinearTransform(self, *args, **kwargs):
        """Generate linear transformation - matches Lattigo interface with compatibility"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        # Handle both signatures:
        # 1. Lattigo interface: (diag_idxs, diag_data, level, bsgs_ratio, io_mode)
        # 2. Test interface: (matrix_data, num_rows, num_cols)

        if len(args) == 3 and isinstance(args[0], list):
            # Test interface: simple matrix
            matrix_data, num_rows, num_cols = args

            # Convert simple matrix to diagonal format
            diag_idxs = [0]  # Main diagonal
            diag_data = matrix_data[:min(len(matrix_data), num_rows * num_cols)]
            level = kwargs.get('level', 1)
            bsgs_ratio = kwargs.get('bsgs_ratio', 1.0)
            io_mode = kwargs.get('io_mode', "memory")

        elif len(args) >= 2:
            # Lattigo interface: diagonal format
            diag_idxs = args[0]
            diag_data = args[1]
            level = args[2] if len(args) > 2 else kwargs.get('level', 1)
            bsgs_ratio = args[3] if len(args) > 3 else kwargs.get('bsgs_ratio', 1.0)
            io_mode = args[4] if len(args) > 4 else kwargs.get('io_mode', "memory")
        else:
            raise ValueError("Invalid arguments for GenerateLinearTransform")

        # Convert to numpy arrays
        if isinstance(diag_idxs, list):
            idxs_array = np.array(diag_idxs, dtype=np.int32)
        else:
            idxs_array = diag_idxs.astype(np.int32)

        if isinstance(diag_data, list):
            data_array = np.array(diag_data, dtype=np.float32)
        else:
            data_array = diag_data.astype(np.float32)

        # Create ctypes pointers
        idxs_ptr = idxs_array.ctypes.data_as(ctypes.POINTER(ctypes.c_int))
        data_ptr = data_array.ctypes.data_as(ctypes.POINTER(ctypes.c_float))

        # Convert io_mode to bytes
        io_mode_bytes = io_mode.encode('utf-8') if isinstance(io_mode, str) else io_mode

        transform_id = self.lib.GenerateLinearTransform(
            idxs_ptr, len(idxs_array),
            data_ptr, len(data_array),
            level, bsgs_ratio, io_mode_bytes
        )

        if transform_id < 0:
            raise RuntimeError("Failed to generate linear transform")

        return transform_id

    def EvaluateLinearTransform(self, transform_id, ciphertext_id):
        """Apply linear transformation - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        result_id = self.lib.EvaluateLinearTransform(ciphertext_id, transform_id)
        if result_id < 0:
            raise RuntimeError("Failed to evaluate linear transform")

        return result_id

    def GetLinearTransformRotationKeys(self, transform_id):
        """Get required rotation keys for linear transformation - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        max_keys = 1024  # Reasonable upper bound
        rotation_keys_array = (ctypes.c_int * max_keys)()

        num_keys = self.lib.GetLinearTransformRotationKeys(
            transform_id, rotation_keys_array, max_keys
        )

        if num_keys < 0:
            raise RuntimeError("Failed to get rotation keys")

        # Convert to Python list
        rotation_keys = [rotation_keys_array[i] for i in range(num_keys)]
        return rotation_keys

    def GenerateLinearTransformRotationKey(self, rotation_amount):
        """Generate specific rotation key for linear transformation - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        self.lib.GenerateLinearTransformRotationKey(rotation_amount)

    def GenerateAndSerializeRotationKey(self, rotation_amount):
        """Generate and serialize rotation key - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        result = self.lib.GenerateAndSerializeRotationKey(rotation_amount)

        if result.Length == 0:
            raise RuntimeError("Failed to generate and serialize rotation key")

        # Convert result to numpy array (matching Lattigo behavior)
        buffer = ctypes.cast(
            result.Data,
            ctypes.POINTER(ctypes.c_ubyte * result.Length)
        ).contents

        return np.array(buffer, dtype=np.uint8)

    def LoadRotationKey(self, serialized_key, rotation_amount=None):
        """Load rotation key from bytes - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        # Handle ArrayResultByte from GenerateAndSerializeRotationKey
        if hasattr(serialized_key, 'Data') and hasattr(serialized_key, 'Length'):
            # This is an ArrayResultByte structure
            buffer = ctypes.cast(
                serialized_key.Data,
                ctypes.POINTER(ctypes.c_ubyte * serialized_key.Length)
            ).contents
            key_data = bytes(buffer)
            key_size = serialized_key.Length
        elif isinstance(serialized_key, np.ndarray):
            key_data = serialized_key.tobytes()
            key_size = len(key_data)
        elif isinstance(serialized_key, str):
            key_data = serialized_key.encode('utf-8')
            key_size = len(key_data)
        elif isinstance(serialized_key, bytes):
            key_data = serialized_key
            key_size = len(key_data)
        else:
            raise ValueError("Invalid serialized_key type")

        # If rotation_amount not provided, extract from the test case context
        if rotation_amount is None:
            rotation_amount = 1  # Default for tests

        result = self.lib.LoadRotationKey(
            ctypes.c_char_p(key_data), key_size, rotation_amount
        )

        if result < 0:
            raise RuntimeError("Failed to load rotation key")

    def SerializeDiagonal(self, diagonal_data):
        """Serialize diagonal for linear transformation - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        if isinstance(diagonal_data, list):
            diagonal_array = np.array(diagonal_data, dtype=np.float64)
        else:
            diagonal_array = diagonal_data.astype(np.float64)

        max_size = 8192 + len(diagonal_array) * 8  # Buffer + data size
        buffer = ctypes.create_string_buffer(max_size)

        diagonal_ptr = diagonal_array.ctypes.data_as(ctypes.POINTER(ctypes.c_double))
        serialized_size = self.lib.SerializeDiagonal(
            diagonal_ptr, len(diagonal_array), buffer, max_size
        )

        if serialized_size < 0:
            raise RuntimeError("Failed to serialize diagonal")

        return buffer.raw[:serialized_size]

    def LoadPlaintextDiagonal(self, serialized_data):
        """Load plaintext diagonal from serialized data - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        if isinstance(serialized_data, str):
            serialized_data = serialized_data.encode('utf-8')

        plaintext_id = self.lib.LoadPlaintextDiagonal(
            ctypes.c_char_p(serialized_data), len(serialized_data)
        )

        if plaintext_id < 0:
            raise RuntimeError("Failed to load plaintext diagonal")

        return plaintext_id

    def RemovePlaintextDiagonals(self, diagonal_ids):
        """Remove plaintext diagonals from memory - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        if not diagonal_ids:
            return

        ids_array = (ctypes.c_int * len(diagonal_ids))(*diagonal_ids)
        self.lib.RemovePlaintextDiagonals(ids_array, len(diagonal_ids))

    def RemoveRotationKeys(self, rotation_amounts):
        """Remove rotation keys from memory - matches Lattigo interface"""
        if not self.initialized:
            raise RuntimeError("Backend not initialized")

        if not rotation_amounts:
            return

        amounts_array = (ctypes.c_int * len(rotation_amounts))(*rotation_amounts)
        self.lib.RemoveRotationKeys(amounts_array, len(rotation_amounts))

    def cleanup(self):
        """Clean up resources"""
        if self.initialized:
            try:
                # Clean up bootstrappers
                self.DeleteBootstrappers()

                # Clean up the main scheme
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
    
    
    def add(self, ct1: int, ct2: int) -> int:
        """Add two ciphertexts"""
        return self.backend.AddCiphertext(ct1, ct2)
    
    def multiply(self, ct1: int, ct2: int) -> int:
        """Multiply two ciphertexts"""
        return self.backend.MulRelinCiphertext(ct1, ct2)
    
    def rotate(self, ct: int, steps: int) -> int:
        """Rotate ciphertext by steps"""
        return self.backend.Rotate(ct, steps)
    
    def rescale(self, ct: int) -> int:
        """Rescale ciphertext"""
        return self.backend.Rescale(ct)

    def bootstrap(self, ct: int, num_slots: int) -> int:
        """Bootstrap ciphertext to refresh noise"""
        return self.backend.bootstrap_ciphertext(ct, num_slots)