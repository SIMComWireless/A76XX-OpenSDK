# demo_sm2

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_sm2.c

## Overview

This demo demonstrates the Chinese national standard SM2 elliptic curve public-key cryptographic operations on the ASR1603 module. SM2 is a Chinese government standard for public-key encryption, comparable to ECC. The demo covers key generation, key input/display, encryption, and decryption using 256-bit SM2 keys.

## Main Features

- SM2 key pair generation (public key X/Y coordinates and private key)
- Manual key input via UART (hex format)
- Display current key pair in hex format
- SM2 encryption using public key (X and Y coordinates)
- SM2 decryption using private key
- Support for both raw source data and hex-encoded input

## Key Header Dependencies

- `sm2.h` -- Provides SM2 cryptographic functions: sm2_keygen, encrypt, decrypt, sm2_sign, sm2_verify, sm2_keyagreement_a1_3, sm2_keyagreement_b1_9, sm2_keyagreement_a4_10, sm2_keyagreement_b10
- `sm3.h` -- Provides SM3 hash functions (used internally by SM2): SM3_Init, SM3_Update, SM3_Final
- `simcom_common.h` -- Provides common types
- `simcom_debug.h` -- Provides sAPI_Debug
- `uart_api.h` -- Provides UartReadValue, UartRead

## SDK APIs Observed

- `sm2_keygen(public_x, &xlen, public_y, &ylen, private_key, &privkeylen)` -- Generate an SM2 key pair. Outputs 32-byte public key X coordinate, 32-byte public key Y coordinate, and 32-byte private key.
- `encrypt(msg, msglen, wx, wxlen, wy, wylen, outmsg)` -- SM2 public-key encryption. Input plaintext, public key X/Y coordinates, output ciphertext. Ciphertext length = 64 + plaintext_len + 32 (C1 + C3 + C2 format).
- `decrypt(msg, msglen, privkey, privkeylen, outmsg)` -- SM2 private-key decryption. Input ciphertext, private key, output plaintext.

## Usage

The demo presents an interactive menu:

1. **keygen** (Option 1): Generates a new SM2 key pair and immediately displays the keys in hex format.
2. **input key** (Option 2): Prompts for 32-byte public key X (hex), 32-byte public key Y (hex), and 32-byte private key (hex) via UART. Stores them for subsequent encrypt/decrypt operations.
3. **show key** (Option 3): Displays the current key pair in hex format.
4. **encrypt** (Option 4): Prompts for input data (source value or hex value), data length, and the data itself. Encrypts using the stored public key X/Y coordinates and displays both source and encrypted data in hex.
5. **decrypt** (Option 5): Prompts for encrypted data (source value or hex value), data length, and the data. Decrypts using the stored private key and displays the decrypted data.
6. **back** (Option 99): Return to parent menu.

## Implementation Notes

- All keys are exactly 32 bytes (256 bits), which is the standard SM2 key size.
- The `hex2value` helper function converts a 2-character hex string to a single byte. The `hex2valueString` function converts a full hex string to a byte array.
- SM2 encryption output size is `64 + plaintext_len + 32` bytes (C1 = 64 bytes for the elliptic curve point, C3 = 32 bytes for the hash, C2 = plaintext_len bytes for the encrypted data).
- SM2 decryption input size is `64 + plaintext_len + 32`, and the plaintext length can be computed as `totalLen - 64 - 32`.
- The encrypt/decrypt functions accept raw byte arrays, not PEM-encoded keys. Keys are exchanged as 32-byte binary blobs.
- The demo uses `goto PRINT_KEY` to avoid code duplication between the keygen and show-key operations.
- `sm2_sign`, `sm2_verify`, and key agreement functions are declared in the header but not demonstrated in this file.

## Best Practices

- Always generate keys using `sm2_keygen` rather than hardcoding test keys in production.
- Store private keys securely. The demo displays them on UART, which is insecure for production use.
- Use the hex input mode when entering keys from a terminal to avoid encoding issues.
- Verify that the public key X and Y coordinates are each exactly 32 bytes before using them for encryption.
- The SM2 encryption output is significantly larger than the input (adds 96 bytes). Account for this in buffer allocation.
- For production applications, consider using SM2 key agreement (`sm2_keyagreement_*`) for secure session key establishment rather than direct encryption for large data.
- SM3 hash (used internally by SM2) is a 256-bit hash function specific to Chinese cryptographic standards.
