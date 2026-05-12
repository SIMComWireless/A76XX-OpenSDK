# demo_crypto

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_crypto.c

## Overview

This demo demonstrates the SIMCom platform crypto library (`pl_crypto`) for performing public-key encryption (RSA/ECC), symmetric cipher encryption/decryption (AES, DES, 3DES, etc.), message digest/hash computation (MD5, SHA1, SHA256, etc.), and random number generation on the ASR1603 module.

## Main Features

- Public-key (PK) encryption and decryption using RSA 2048 with PEM keys
- Symmetric cipher operations (AES, DES, 3DES, CAMELLIA, ARIA, CHACHA20) with multiple modes (ECB, CBC, CFB, OFB, CTR, GCM, CCM, XTS, CHACHAPOLY) and padding options
- Both one-shot (PL_MODE_ONCE) and streaming (PL_MODE_START/UPDATE/FINISH) cipher operations
- Message digest (hash) computation (MD5, SHA1, SHA224, SHA256, SHA384, SHA512, RIPEMD160)
- HMAC message digest computation
- Random data generation with customization string

## Key Header Dependencies

- `pl_crypto.h` -- Provides the platform crypto abstraction layer: pl_crypto_create, pl_crypto_cfg, pl_crypto, pl_crypto_delete, pl_crypto_random, and associated types/enums (pl_crypto_handle, pl_crypto_buf_t, PL_CFG_TYPE, PL_CFG_CIPHER_MODE, PL_CFG_CIPHER_KEY, PL_CFG_CIPHER_IV, PL_CFG_CIPHER_PADDING, PL_CFG_PK_PUBLIC_KEY, PL_CFG_PK_PRIVATE_KEY, PL_CFG_MD_KEY, PL_PK_ENCRYPT, PL_PK_DECRYPT, PL_CIPHER_ENCRYPT, PL_CIPHER_DECRYPT, PL_MD_HASH, PL_MD_HASH_HMAC, PL_MODE_ONCE, PL_MODE_START, PL_MODE_UPDATE, PL_MODE_FINISH)
- `simcom_os.h` -- Provides sAPI_Malloc, sAPI_Free, sAPI_Debug
- `simcom_debug.h` -- Provides sAPI_Debug
- `uart_api.h` -- Provides UartReadValue for menu input

## SDK APIs Observed

- `pl_crypto_create(&hd)` -- Create a crypto handle. Must be called before any configuration or operation.
- `pl_crypto_cfg(hd, cfg_type, &value)` -- Configure the crypto handle. Configuration types include PL_CFG_TYPE (algorithm type), PL_CFG_CIPHER_MODE, PL_CFG_CIPHER_KEY, PL_CFG_CIPHER_IV, PL_CFG_CIPHER_PADDING, PL_CFG_PK_PUBLIC_KEY, PL_CFG_PK_PRIVATE_KEY, PL_CFG_MD_KEY.
- `pl_crypto(hd, operation, mode, input, inLen, output, &outLen, bufLen)` -- Perform the crypto operation. Operations include PL_PK_ENCRYPT, PL_PK_DECRYPT, PL_CIPHER_ENCRYPT, PL_CIPHER_DECRYPT, PL_MD_HASH, PL_MD_HASH_HMAC. Modes: PL_MODE_ONCE, PL_MODE_START, PL_MODE_UPDATE, PL_MODE_FINISH.
- `pl_crypto_delete(&hd)` -- Delete/free the crypto handle.
- `pl_crypto_random(output, len, pers, persLen)` -- Generate random bytes with a personalization string.

## Usage

The demo presents a top-level menu:

1. **TEST PK** (Option 0): Shows sub-menu for PK types (RSA, ECKEY, ECDSA, etc.). After selecting a type and entering data length, the demo encrypts with the embedded RSA-2048 PEM public key and decrypts with the corresponding private key.
2. **TEST CIPHER** (Option 1): Shows sub-menus for cipher mode (ECB, CBC, CFB, etc.), padding (PKCS7, etc.), and key size (64/128/192/256 bits). Encrypts test data, decrypts, and compares. For data > 50 bytes, also demonstrates streaming mode encryption/decryption.
3. **TEST MD** (Option 2): Shows sub-menu for hash types (MD5, SHA1, SHA256, etc.). Computes both one-shot and streaming hash, plus HMAC variants.
4. **TEST RANDOM** (Option 3): Generates random data of user-specified length with a "Customization" personalization string.
5. **Back** (Option 99): Return to parent menu.

## Implementation Notes

- The `demo_crypto()` function handles all operations. It allocates buffers dynamically based on the test data length and algorithm requirements.
- RSA encryption buffer size: `rsa_size = ((test_len - 1) / (256 - 11) + 1) * 256` (RSA-2048 with PKCS1 padding, block size = 256-11 = 245 bytes plaintext per block).
- ECC encryption buffer: approximately `test_len * 2`.
- Symmetric cipher buffer: `test_len + 16` (one extra block for padding).
- The maximum buffer size is computed as the max of RSA, ECC, and symmetric sizes.
- The IV is always configured with 16 bytes; the library internally uses the required portion.
- Streaming mode (PL_MODE_START/UPDATE/FINISH) splits data into 20-byte chunks for demonstration. The output buffer and length automatically accumulate across calls.
- Pre-defined test keys are embedded: 8-byte, 16-byte, 24-byte, and 32-byte symmetric keys, and 2048-bit RSA PEM key pair.
- Hash streaming also demonstrates START/UPDATE/FINISH with different chunk sizes.

## Best Practices

- Always call `pl_crypto_create` before operations and `pl_crypto_delete` when done to avoid memory leaks.
- Use `PL_MODE_ONCE` for small data that fits in memory; use START/UPDATE/FINISH for large or streaming data to reduce peak memory usage.
- Choose key sizes appropriate for your security requirements (AES-256 for highest symmetric security, RSA-2048 minimum for asymmetric).
- For CBC mode, always use a random or unique IV for each encryption operation to prevent replay attacks.
- Use HMAC (PL_MD_HASH_HMAC) instead of plain hash when message authentication is needed.
- Check all return codes from pl_crypto functions. Non-zero returns indicate errors that must be handled.
- The personalization string in `pl_crypto_random` should be unique per application to seed the DRBG appropriately.
