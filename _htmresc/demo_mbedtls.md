# demo_mbedtls

Location: SIMCOM_SDK_SET/sc_demo/V1/src/demo_mbedtls.c

## Overview

This demo demonstrates the use of mbedTLS cryptographic library functions directly on the ASR1603 module for RSA encryption/decryption with various key sizes (1024, 2048, 4096 bits), AES ECB encryption/decryption, and file-based message digest (hash) computation. It uses the mbedTLS API directly rather than the platform crypto abstraction layer.

## Main Features

- RSA public-key encryption and decryption with 1024-bit, 2048-bit, and 4096-bit PEM keys
- Block-by-block RSA encryption for data larger than the RSA block size
- AES-128 ECB mode encryption and decryption
- Base64 encoding and decoding (commented out in demo)
- File-based SHA-256 hash computation using mbedTLS message digest API
- Embedded PEM key pairs for all three RSA key sizes

## Key Header Dependencies

- `mbedtls/entropy.h` -- Provides mbedtls_entropy_context, mbedtls_entropy_init, mbedtls_entropy_func, mbedtls_entropy_free for random number generation seeding
- `mbedtls/ctr_drbg.h` -- Provides mbedtls_ctr_drbg_context, mbedtls_ctr_drbg_init, mbedtls_ctr_drbg_seed, mbedtls_ctr_drbg_random, mbedtls_ctr_drbg_free for deterministic random byte generation
- `mbedtls/pk.h` -- Provides mbedtls_pk_context, mbedtls_pk_init, mbedtls_pk_parse_key, mbedtls_pk_parse_public_key, mbedtls_pk_free, mbedtls_pk_rsa for public-key operations
- `mbedtls/rsa.h` -- Provides mbedtls_rsa_context, mbedtls_rsa_init, mbedtls_rsa_get_len, mbedtls_rsa_pkcs1_encrypt, mbedtls_rsa_pkcs1_decrypt, mbedtls_rsa_complete, mbedtls_rsa_check_pubkey, mbedtls_rsa_check_privkey, mbedtls_rsa_free
- `mbedtls/aes.h` -- Provides mbedtls_aes_context, mbedtls_aes_init, mbedtls_aes_setkey_enc, mbedtls_aes_setkey_dec, mbedtls_aes_crypt_ecb, mbedtls_aes_free
- `mbedtls/md.h` -- Provides mbedtls_md_context_t, mbedtls_md, mbedtls_md_setup, mbedtls_md_info_from_type, mbedtls_md_get_size, mbedtls_md_free
- `mbedtls/md_internal.h` -- Provides md_info structure with starts_func, update_func, finish_func function pointers
- `mbedtls/base64.h` -- Provides mbedtls_base64_encode, mbedtls_base64_decode (used in commented-out code)
- `simcom_file.h` -- Provides sAPI_fopen, sAPI_fread, sAPI_fclose for file I/O in hash computation

## SDK APIs Observed

- `sAPI_AesEncrypt(key, keybits, input, output)` -- AES-128 ECB encryption wrapper (16-byte blocks)
- `sAPI_AesDecrypt(key, keybits, input, output)` -- AES-128 ECB decryption wrapper (16-byte blocks)
- `sAPI_RsaEncrypt(output, input, length, publickey)` -- RSA public-key encryption with automatic block splitting for data larger than RSA block size
- `sAPI_RsaDecrypt(output, output_max_len, input, length, privatekey)` -- RSA private-key decryption with automatic block reassembly
- `mbedtls_md_file(md_info, path, output)` -- Compute SHA-256 hash of a file, reading in 1024-byte chunks
- `mbedtls_rsa_get_len(ctx)` -- Get the RSA key byte length (128 for RSA-1024, 256 for RSA-2048, 512 for RSA-4096)
- `mbedtls_ctr_drbg_random(ctx, output, len)` -- Generate random bytes via DRBG

## Usage

The demo presents an interactive menu:

1. **TEST FOR RSA 1024** (Option 1): Enter data length. Encrypts test data with RSA-1024, then decrypts and displays both.
2. **TEST FOR RSA 2048** (Option 2): Same as above with RSA-2048 keys.
3. **TEST FOR RSA 4096** (Option 3): Same as above with RSA-4096 keys.
4. **MD FILE** (Option 4, SC_MD_FILE): Computes SHA-256 hash of file `c:/hash_test.txt`.
5. **Back** (Option 99): Return to parent menu.

## Implementation Notes

- RSA encryption automatically splits data into blocks of `rsa_size - 11` bytes (PKCS#1 v1.5 padding overhead) and encrypts each block independently via `mbedtls_rsa_pkcs1_encrypt`.
- RSA decryption reads `rsa_size` byte blocks and decrypts each via `mbedtls_rsa_pkcs1_decrypt`.
- Key lengths: RSA-1024 = 128 bytes, RSA-2048 = 256 bytes, RSA-4096 = 512 bytes.
- The `mbedtls_md_file` function demonstrates streaming hash computation: it opens a file, sets up the MD context with `md_info->starts_func`, reads in 1024-byte chunks calling `md_info->update_func`, then finalizes with `md_info->finish_func`.
- Test data is auto-generated as a repeating pattern of '0' through '9'.
- The AES and Base64 test code is disabled (`#if 0` block in sAPP_EncrypeDemo).
- The file hash function uses `sAPI_fopen` and `sAPI_fread` (SIMCom file APIs) wrapped around mbedTLS digest functions.
- The demo includes full PEM-encoded RSA key pairs for all three key sizes as static arrays.

## Best Practices

- Use RSA-2048 as the minimum key size for production. RSA-1024 is considered weak by modern standards.
- Always seed the DRBG with `mbedtls_ctr_drbg_seed` before using any random number operations.
- Free all mbedTLS contexts (`mbedtls_pk_free`, `mbedtls_ctr_drbg_free`, `mbedtls_entropy_free`, `mbedtls_aes_free`, `mbedtls_md_free`) to prevent memory leaks.
- For file hashing, use the streaming approach (start/update/finish) to avoid loading the entire file into memory.
- Store PEM private keys securely. The demo embeds them as static arrays for testing only.
- When using AES, choose the appropriate mode (CBC, CTR, GCM) rather than ECB for production, as ECB does not provide semantic security.
