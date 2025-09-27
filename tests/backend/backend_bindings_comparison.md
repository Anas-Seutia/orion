# Backend Bindings Comparison: Lattigo vs OpenFHE (Updated)

This document provides a comprehensive comparison of the backend binding functions between Lattigo and OpenFHE implementations in the Orion framework.

## Analysis Overview

- **Lattigo Backend**: `/orion/backend/lattigo/bindings.py`
- **OpenFHE Backend**: `/orion/backend/openfhe/bindings.py` (Updated to match Lattigo interface)
- **Analysis Date**: 2025-09-27
- **Update Date**: 2025-09-27 (Linear Transform Operations Implementation Complete)

## Color-Coded Function Categories

### 🟢 Compatible Functions
Functions that now exist with identical names and similar functionality in both backends:

| Function Name | Description |
|---------------|-------------|
| `NewScheme` | Initialize cryptographic scheme |
| `DeleteScheme` | Clean up scheme resources |
| `NewEncoder` | Initialize encoder |
| `NewEncryptor` | Initialize encryptor |
| `NewDecryptor` | Initialize decryptor |
| `NewEvaluator` | Initialize evaluator |
| `NewKeyGenerator` | Initialize key generator |
| `GenerateSecretKey` | Generate secret key |
| `GeneratePublicKey` | Generate public key |
| `GenerateRelinearizationKey` | Generate relinearization key |
| `GenerateEvaluationKeys` | Generate evaluation keys |
| `Encrypt` | Encrypt plaintext to ciphertext |
| `Decrypt` | Decrypt ciphertext to plaintext |
| `Encode` | Create plaintext from values (renamed from `CreatePlaintext`) |
| `Decode` | Extract values from plaintext (renamed from `GetPlaintextValues`) |
| `AddCiphertext` | Add two ciphertexts (renamed from `Add`) |
| `AddPlaintext` | Add plaintext to ciphertext (renamed from `AddPlain`) |
| `MulRelinCiphertext` | Multiply ciphertexts with relinearization (renamed from `Multiply`) |
| `MulPlaintext` | Multiply ciphertext by plaintext (renamed from `MultiplyPlain`) |
| `Rotate` | Rotate ciphertext slots |
| `RotateNew` | Rotate and create new ciphertext (alias added) |
| `Rescale` | Rescale ciphertext |
| `RescaleNew` | Rescale and create new ciphertext (alias added) |
| `SerializeSecretKey` | Serialize secret key to bytes |
| `LoadSecretKey` | Load secret key from bytes |
| `Bootstrap` | Bootstrap ciphertext |
| `DeleteCiphertext` | Delete ciphertext from memory |
| `DeletePlaintext` | Delete plaintext from memory |
| `DeleteLinearTransform` | Delete linear transform |
| `NewBootstrapper` | Initialize bootstrapper |
| `DeleteBootstrappers` | Clean up bootstrappers |
| `NewPolynomialEvaluator` | Initialize polynomial evaluator |
| `GenerateMonomial` | Generate monomial polynomial |
| `GenerateChebyshev` | Generate Chebyshev polynomial |
| `EvaluatePolynomial` | Evaluate polynomial on ciphertext |
| `GenerateMinimaxSignCoeffs` | Generate minimax sign approximation coefficients |
| `FreeCArray` | Free C allocated memory |
| `AddRotationKey` | Add rotation key for evaluation |
| `Negate` | Negate ciphertext |
| `AddScalar` / `AddScalarNew` | Add scalar to ciphertext |
| `SubScalar` / `SubScalarNew` | Subtract scalar from ciphertext |
| `MulScalarInt` / `MulScalarIntNew` | Multiply by integer scalar |
| `MulScalarFloat` / `MulScalarFloatNew` | Multiply by float scalar |
| `SubCiphertext` / `SubCiphertextNew` | Subtract ciphertexts |
| `SubPlaintext` / `SubPlaintextNew` | Subtract plaintext from ciphertext |
| `AddCiphertextNew` | Add ciphertexts (new copy) |
| `AddPlaintextNew` | Add plaintext to ciphertext (new copy) |
| `MulPlaintextNew` | Multiply ciphertext by plaintext (new copy) |
| `MulRelinCiphertextNew` | Multiply ciphertexts with relinearization (new copy) |
| `GetPlaintextScale` / `SetPlaintextScale` | Plaintext scale management |
| `GetCiphertextScale` / `SetCiphertextScale` | Ciphertext scale management |
| `GetPlaintextLevel` / `GetCiphertextLevel` | Level management |
| `GetPlaintextSlots` / `GetCiphertextSlots` | Slot count queries |
| `GetModuliChain` | Get moduli chain information |
| `GetLivePlaintexts` / `GetLiveCiphertexts` | Memory tracking (now with improved error handling) |
| `NewLinearTransformEvaluator` | Initialize linear transform evaluator | 
| `GenerateLinearTransform` | Generate linear transformation from diagonal data
| `EvaluateLinearTransform` | Apply linear transformation to ciphertext | 
| `GetLinearTransformRotationKeys` | Get array of required rotation keys | 
| `GenerateLinearTransformRotationKey` | Generate specific rotation key | 
| `GenerateAndSerializeRotationKey` | Generate and serialize rotation key |
| `LoadRotationKey` | Load rotation key from serialized bytes |
| `SerializeDiagonal` | Serialize diagonal data for linear transforms |
| `LoadPlaintextDiagonal` | Load plaintext diagonal from serialized data | 
| `RemovePlaintextDiagonals` | Remove plaintext diagonals from memory |
| `RemoveRotationKeys` | Remove rotation keys from memory |
| `GetCiphertextDegree` | Ciphertext degree query |

**Total Compatible Functions: 70 functions**