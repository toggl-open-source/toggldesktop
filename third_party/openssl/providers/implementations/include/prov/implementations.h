/*
 * Copyright 2019-2020 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the Apache License 2.0 (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include <openssl/core.h>
#include <openssl/types.h>

/* Digests */
extern const OSSL_DISPATCH sha1_functions[];
extern const OSSL_DISPATCH sha224_functions[];
extern const OSSL_DISPATCH sha256_functions[];
extern const OSSL_DISPATCH sha384_functions[];
extern const OSSL_DISPATCH sha512_functions[];
extern const OSSL_DISPATCH sha512_224_functions[];
extern const OSSL_DISPATCH sha512_256_functions[];
extern const OSSL_DISPATCH sha3_224_functions[];
extern const OSSL_DISPATCH sha3_256_functions[];
extern const OSSL_DISPATCH sha3_384_functions[];
extern const OSSL_DISPATCH sha3_512_functions[];
extern const OSSL_DISPATCH keccak_kmac_128_functions[];
extern const OSSL_DISPATCH keccak_kmac_256_functions[];
extern const OSSL_DISPATCH shake_128_functions[];
extern const OSSL_DISPATCH shake_256_functions[];
extern const OSSL_DISPATCH blake2s256_functions[];
extern const OSSL_DISPATCH blake2b512_functions[];
extern const OSSL_DISPATCH md5_functions[];
extern const OSSL_DISPATCH md5_sha1_functions[];
extern const OSSL_DISPATCH sm3_functions[];
extern const OSSL_DISPATCH md2_functions[];
extern const OSSL_DISPATCH md4_functions[];
extern const OSSL_DISPATCH mdc2_functions[];
extern const OSSL_DISPATCH wp_functions[];
extern const OSSL_DISPATCH ripemd160_functions[];

/* Ciphers */
extern const OSSL_DISPATCH null_functions[];
extern const OSSL_DISPATCH aes256ecb_functions[];
extern const OSSL_DISPATCH aes192ecb_functions[];
extern const OSSL_DISPATCH aes128ecb_functions[];
extern const OSSL_DISPATCH aes256cbc_functions[];
extern const OSSL_DISPATCH aes192cbc_functions[];
extern const OSSL_DISPATCH aes128cbc_functions[];
extern const OSSL_DISPATCH aes256cbc_cts_functions[];
extern const OSSL_DISPATCH aes192cbc_cts_functions[];
extern const OSSL_DISPATCH aes128cbc_cts_functions[];
extern const OSSL_DISPATCH aes256ofb_functions[];
extern const OSSL_DISPATCH aes192ofb_functions[];
extern const OSSL_DISPATCH aes128ofb_functions[];
extern const OSSL_DISPATCH aes256cfb_functions[];
extern const OSSL_DISPATCH aes192cfb_functions[];
extern const OSSL_DISPATCH aes128cfb_functions[];
extern const OSSL_DISPATCH aes256cfb1_functions[];
extern const OSSL_DISPATCH aes192cfb1_functions[];
extern const OSSL_DISPATCH aes128cfb1_functions[];
extern const OSSL_DISPATCH aes256cfb8_functions[];
extern const OSSL_DISPATCH aes192cfb8_functions[];
extern const OSSL_DISPATCH aes128cfb8_functions[];
extern const OSSL_DISPATCH aes256ctr_functions[];
extern const OSSL_DISPATCH aes192ctr_functions[];
extern const OSSL_DISPATCH aes128ctr_functions[];
extern const OSSL_DISPATCH aes256xts_functions[];
extern const OSSL_DISPATCH aes128xts_functions[];
#ifndef OPENSSL_NO_OCB
extern const OSSL_DISPATCH aes256ocb_functions[];
extern const OSSL_DISPATCH aes192ocb_functions[];
extern const OSSL_DISPATCH aes128ocb_functions[];
#endif /* OPENSSL_NO_OCB */
extern const OSSL_DISPATCH aes256gcm_functions[];
extern const OSSL_DISPATCH aes192gcm_functions[];
extern const OSSL_DISPATCH aes128gcm_functions[];
extern const OSSL_DISPATCH aes256ccm_functions[];
extern const OSSL_DISPATCH aes192ccm_functions[];
extern const OSSL_DISPATCH aes128ccm_functions[];
extern const OSSL_DISPATCH aes256wrap_functions[];
extern const OSSL_DISPATCH aes192wrap_functions[];
extern const OSSL_DISPATCH aes128wrap_functions[];
extern const OSSL_DISPATCH aes256wrappad_functions[];
extern const OSSL_DISPATCH aes192wrappad_functions[];
extern const OSSL_DISPATCH aes128wrappad_functions[];
extern const OSSL_DISPATCH aes256cbc_hmac_sha1_functions[];
extern const OSSL_DISPATCH aes128cbc_hmac_sha1_functions[];
extern const OSSL_DISPATCH aes256cbc_hmac_sha256_functions[];
extern const OSSL_DISPATCH aes128cbc_hmac_sha256_functions[];

#ifndef OPENSSL_NO_ARIA
extern const OSSL_DISPATCH aria256gcm_functions[];
extern const OSSL_DISPATCH aria192gcm_functions[];
extern const OSSL_DISPATCH aria128gcm_functions[];
extern const OSSL_DISPATCH aria256ccm_functions[];
extern const OSSL_DISPATCH aria192ccm_functions[];
extern const OSSL_DISPATCH aria128ccm_functions[];
extern const OSSL_DISPATCH aria256ecb_functions[];
extern const OSSL_DISPATCH aria192ecb_functions[];
extern const OSSL_DISPATCH aria128ecb_functions[];
extern const OSSL_DISPATCH aria256cbc_functions[];
extern const OSSL_DISPATCH aria192cbc_functions[];
extern const OSSL_DISPATCH aria128cbc_functions[];
extern const OSSL_DISPATCH aria256ofb_functions[];
extern const OSSL_DISPATCH aria192ofb_functions[];
extern const OSSL_DISPATCH aria128ofb_functions[];
extern const OSSL_DISPATCH aria256cfb_functions[];
extern const OSSL_DISPATCH aria192cfb_functions[];
extern const OSSL_DISPATCH aria128cfb_functions[];
extern const OSSL_DISPATCH aria256cfb1_functions[];
extern const OSSL_DISPATCH aria192cfb1_functions[];
extern const OSSL_DISPATCH aria128cfb1_functions[];
extern const OSSL_DISPATCH aria256cfb8_functions[];
extern const OSSL_DISPATCH aria192cfb8_functions[];
extern const OSSL_DISPATCH aria128cfb8_functions[];
extern const OSSL_DISPATCH aria256ctr_functions[];
extern const OSSL_DISPATCH aria192ctr_functions[];
extern const OSSL_DISPATCH aria128ctr_functions[];
#endif /* OPENSSL_NO_ARIA */
#ifndef OPENSSL_NO_CAMELLIA
extern const OSSL_DISPATCH camellia256ecb_functions[];
extern const OSSL_DISPATCH camellia192ecb_functions[];
extern const OSSL_DISPATCH camellia128ecb_functions[];
extern const OSSL_DISPATCH camellia256cbc_functions[];
extern const OSSL_DISPATCH camellia192cbc_functions[];
extern const OSSL_DISPATCH camellia128cbc_functions[];
extern const OSSL_DISPATCH camellia256ofb_functions[];
extern const OSSL_DISPATCH camellia192ofb_functions[];
extern const OSSL_DISPATCH camellia128ofb_functions[];
extern const OSSL_DISPATCH camellia256cfb_functions[];
extern const OSSL_DISPATCH camellia192cfb_functions[];
extern const OSSL_DISPATCH camellia128cfb_functions[];
extern const OSSL_DISPATCH camellia256cfb1_functions[];
extern const OSSL_DISPATCH camellia192cfb1_functions[];
extern const OSSL_DISPATCH camellia128cfb1_functions[];
extern const OSSL_DISPATCH camellia256cfb8_functions[];
extern const OSSL_DISPATCH camellia192cfb8_functions[];
extern const OSSL_DISPATCH camellia128cfb8_functions[];
extern const OSSL_DISPATCH camellia256ctr_functions[];
extern const OSSL_DISPATCH camellia192ctr_functions[];
extern const OSSL_DISPATCH camellia128ctr_functions[];
#endif /* OPENSSL_NO_CAMELLIA */
#ifndef OPENSSL_NO_BF
extern const OSSL_DISPATCH blowfish128ecb_functions[];
extern const OSSL_DISPATCH blowfish128cbc_functions[];
extern const OSSL_DISPATCH blowfish64ofb64_functions[];
extern const OSSL_DISPATCH blowfish64cfb64_functions[];
#endif /* OPENSSL_NO_BF */
#ifndef OPENSSL_NO_IDEA
extern const OSSL_DISPATCH idea128ecb_functions[];
extern const OSSL_DISPATCH idea128cbc_functions[];
extern const OSSL_DISPATCH idea128ofb64_functions[];
extern const OSSL_DISPATCH idea128cfb64_functions[];
#endif /* OPENSSL_NO_IDEA */
#ifndef OPENSSL_NO_CAST
extern const OSSL_DISPATCH cast5128ecb_functions[];
extern const OSSL_DISPATCH cast5128cbc_functions[];
extern const OSSL_DISPATCH cast5128ofb64_functions[];
extern const OSSL_DISPATCH cast5128cfb64_functions[];
#endif /* OPENSSL_NO_CAST */
#ifndef OPENSSL_NO_SEED
extern const OSSL_DISPATCH seed128ecb_functions[];
extern const OSSL_DISPATCH seed128cbc_functions[];
extern const OSSL_DISPATCH seed128ofb128_functions[];
extern const OSSL_DISPATCH seed128cfb128_functions[];
#endif /* OPENSSL_NO_SEED */
#ifndef OPENSSL_NO_SM4
extern const OSSL_DISPATCH sm4128ecb_functions[];
extern const OSSL_DISPATCH sm4128cbc_functions[];
extern const OSSL_DISPATCH sm4128ctr_functions[];
extern const OSSL_DISPATCH sm4128ofb128_functions[];
extern const OSSL_DISPATCH sm4128cfb128_functions[];
#endif /* OPENSSL_NO_SM4 */
#ifndef OPENSSL_NO_RC5
extern const OSSL_DISPATCH rc5128ecb_functions[];
extern const OSSL_DISPATCH rc5128cbc_functions[];
extern const OSSL_DISPATCH rc5128ofb64_functions[];
extern const OSSL_DISPATCH rc5128cfb64_functions[];
#endif /* OPENSSL_NO_RC5 */
#ifndef OPENSSL_NO_RC2
extern const OSSL_DISPATCH rc2128ecb_functions[];
extern const OSSL_DISPATCH rc2128cbc_functions[];
extern const OSSL_DISPATCH rc240cbc_functions[];
extern const OSSL_DISPATCH rc264cbc_functions[];
extern const OSSL_DISPATCH rc2128cfb128_functions[];
extern const OSSL_DISPATCH rc2128ofb128_functions[];
#endif /* OPENSSL_NO_RC2 */
#ifndef OPENSSL_NO_DES
extern const OSSL_DISPATCH tdes_ede3_ecb_functions[];
extern const OSSL_DISPATCH tdes_ede3_cbc_functions[];
# ifndef FIPS_MODULE
extern const OSSL_DISPATCH tdes_ede3_ofb_functions[];
extern const OSSL_DISPATCH tdes_ede3_cfb_functions[];
extern const OSSL_DISPATCH tdes_ede3_cfb8_functions[];
extern const OSSL_DISPATCH tdes_ede3_cfb1_functions[];

extern const OSSL_DISPATCH tdes_ede2_ecb_functions[];
extern const OSSL_DISPATCH tdes_ede2_cbc_functions[];
extern const OSSL_DISPATCH tdes_ede2_ofb_functions[];
extern const OSSL_DISPATCH tdes_ede2_cfb_functions[];

extern const OSSL_DISPATCH tdes_desx_cbc_functions[];
extern const OSSL_DISPATCH tdes_wrap_cbc_functions[];

extern const OSSL_DISPATCH des_ecb_functions[];
extern const OSSL_DISPATCH des_cbc_functions[];
extern const OSSL_DISPATCH des_ofb64_functions[];
extern const OSSL_DISPATCH des_cfb64_functions[];
extern const OSSL_DISPATCH des_cfb1_functions[];
extern const OSSL_DISPATCH des_cfb8_functions[];
# endif /* FIPS_MODULE */
#endif /* OPENSSL_NO_DES */

#ifndef OPENSSL_NO_RC4
extern const OSSL_DISPATCH rc440_functions[];
extern const OSSL_DISPATCH rc4128_functions[];
# ifndef OPENSSL_NO_MD5
extern const OSSL_DISPATCH rc4_hmac_md5_functions[];
# endif /* OPENSSL_NO_MD5 */
#endif /* OPENSSL_NO_RC4 */
#ifndef OPENSSL_NO_CHACHA
extern const OSSL_DISPATCH chacha20_functions[];
# ifndef OPENSSL_NO_POLY1305
extern const OSSL_DISPATCH chacha20_poly1305_functions[];
# endif /* OPENSSL_NO_POLY1305 */
#endif /* OPENSSL_NO_CHACHA */


#ifndef OPENSSL_NO_SIV
extern const OSSL_DISPATCH aes128siv_functions[];
extern const OSSL_DISPATCH aes192siv_functions[];
extern const OSSL_DISPATCH aes256siv_functions[];
#endif /* OPENSSL_NO_SIV */

/* MACs */
extern const OSSL_DISPATCH blake2bmac_functions[];
extern const OSSL_DISPATCH blake2smac_functions[];
extern const OSSL_DISPATCH cmac_functions[];
extern const OSSL_DISPATCH gmac_functions[];
extern const OSSL_DISPATCH hmac_functions[];
extern const OSSL_DISPATCH kmac128_functions[];
extern const OSSL_DISPATCH kmac256_functions[];
extern const OSSL_DISPATCH siphash_functions[];
extern const OSSL_DISPATCH poly1305_functions[];

/* KDFs / PRFs */
extern const OSSL_DISPATCH kdf_pbkdf2_functions[];
extern const OSSL_DISPATCH kdf_pkcs12_functions[];
#ifndef OPENSSL_NO_SCRYPT
extern const OSSL_DISPATCH kdf_scrypt_functions[];
#endif
extern const OSSL_DISPATCH kdf_tls1_prf_functions[];
extern const OSSL_DISPATCH kdf_hkdf_functions[];
extern const OSSL_DISPATCH kdf_sshkdf_functions[];
extern const OSSL_DISPATCH kdf_sskdf_functions[];
extern const OSSL_DISPATCH kdf_x963_kdf_functions[];
extern const OSSL_DISPATCH kdf_kbkdf_functions[];
#ifndef OPENSSL_NO_CMS
extern const OSSL_DISPATCH kdf_x942_kdf_functions[];
#endif
extern const OSSL_DISPATCH kdf_krb5kdf_functions[];

/* RNGs */
extern const OSSL_DISPATCH test_rng_functions[];
extern const OSSL_DISPATCH drbg_hash_functions[];
extern const OSSL_DISPATCH drbg_hmac_functions[];
extern const OSSL_DISPATCH drbg_ctr_functions[];
extern const OSSL_DISPATCH crngt_functions[];

/* Key management */
extern const OSSL_DISPATCH dh_keymgmt_functions[];
extern const OSSL_DISPATCH dhx_keymgmt_functions[];
extern const OSSL_DISPATCH dsa_keymgmt_functions[];
extern const OSSL_DISPATCH rsa_keymgmt_functions[];
extern const OSSL_DISPATCH rsapss_keymgmt_functions[];
extern const OSSL_DISPATCH x25519_keymgmt_functions[];
extern const OSSL_DISPATCH x448_keymgmt_functions[];
extern const OSSL_DISPATCH ed25519_keymgmt_functions[];
extern const OSSL_DISPATCH ed448_keymgmt_functions[];
extern const OSSL_DISPATCH ec_keymgmt_functions[];
extern const OSSL_DISPATCH kdf_keymgmt_functions[];
extern const OSSL_DISPATCH mac_legacy_keymgmt_functions[];
extern const OSSL_DISPATCH cmac_legacy_keymgmt_functions[];

/* Key Exchange */
extern const OSSL_DISPATCH dh_keyexch_functions[];
extern const OSSL_DISPATCH x25519_keyexch_functions[];
extern const OSSL_DISPATCH x448_keyexch_functions[];
extern const OSSL_DISPATCH ecdh_keyexch_functions[];
extern const OSSL_DISPATCH kdf_tls1_prf_keyexch_functions[];
extern const OSSL_DISPATCH kdf_hkdf_keyexch_functions[];
extern const OSSL_DISPATCH kdf_scrypt_keyexch_functions[];

/* Signature */
extern const OSSL_DISPATCH dsa_signature_functions[];
extern const OSSL_DISPATCH rsa_signature_functions[];
extern const OSSL_DISPATCH ed25519_signature_functions[];
extern const OSSL_DISPATCH ed448_signature_functions[];
extern const OSSL_DISPATCH ecdsa_signature_functions[];
extern const OSSL_DISPATCH mac_legacy_hmac_signature_functions[];
extern const OSSL_DISPATCH mac_legacy_siphash_signature_functions[];
extern const OSSL_DISPATCH mac_legacy_poly1305_signature_functions[];
extern const OSSL_DISPATCH mac_legacy_cmac_signature_functions[];

/* Asym Cipher */
extern const OSSL_DISPATCH rsa_asym_cipher_functions[];

/* Encoders */
extern const OSSL_DISPATCH rsa_priv_to_text_encoder_functions[];
extern const OSSL_DISPATCH rsa_pub_to_text_encoder_functions[];
extern const OSSL_DISPATCH rsa_priv_to_der_encoder_functions[];
extern const OSSL_DISPATCH rsa_pub_to_der_encoder_functions[];
extern const OSSL_DISPATCH rsa_priv_to_pem_encoder_functions[];
extern const OSSL_DISPATCH rsa_pub_to_pem_encoder_functions[];

extern const OSSL_DISPATCH dh_priv_to_text_encoder_functions[];
extern const OSSL_DISPATCH dh_pub_to_text_encoder_functions[];
extern const OSSL_DISPATCH dh_param_to_text_encoder_functions[];
extern const OSSL_DISPATCH dh_priv_to_der_encoder_functions[];
extern const OSSL_DISPATCH dh_pub_to_der_encoder_functions[];
extern const OSSL_DISPATCH dh_param_to_der_encoder_functions[];
extern const OSSL_DISPATCH dh_priv_to_pem_encoder_functions[];
extern const OSSL_DISPATCH dh_pub_to_pem_encoder_functions[];
extern const OSSL_DISPATCH dh_param_to_pem_encoder_functions[];

extern const OSSL_DISPATCH dsa_priv_to_text_encoder_functions[];
extern const OSSL_DISPATCH dsa_pub_to_text_encoder_functions[];
extern const OSSL_DISPATCH dsa_param_to_text_encoder_functions[];
extern const OSSL_DISPATCH dsa_priv_to_der_encoder_functions[];
extern const OSSL_DISPATCH dsa_pub_to_der_encoder_functions[];
extern const OSSL_DISPATCH dsa_param_to_der_encoder_functions[];
extern const OSSL_DISPATCH dsa_priv_to_pem_encoder_functions[];
extern const OSSL_DISPATCH dsa_pub_to_pem_encoder_functions[];
extern const OSSL_DISPATCH dsa_param_to_pem_encoder_functions[];

extern const OSSL_DISPATCH x25519_priv_to_text_encoder_functions[];
extern const OSSL_DISPATCH x25519_pub_to_text_encoder_functions[];
extern const OSSL_DISPATCH x25519_priv_to_der_encoder_functions[];
extern const OSSL_DISPATCH x25519_pub_to_der_encoder_functions[];
extern const OSSL_DISPATCH x25519_priv_to_pem_encoder_functions[];
extern const OSSL_DISPATCH x25519_pub_to_pem_encoder_functions[];

extern const OSSL_DISPATCH x448_priv_to_text_encoder_functions[];
extern const OSSL_DISPATCH x448_pub_to_text_encoder_functions[];
extern const OSSL_DISPATCH x448_priv_to_der_encoder_functions[];
extern const OSSL_DISPATCH x448_pub_to_der_encoder_functions[];
extern const OSSL_DISPATCH x448_priv_to_pem_encoder_functions[];
extern const OSSL_DISPATCH x448_pub_to_pem_encoder_functions[];

extern const OSSL_DISPATCH ed25519_priv_to_text_encoder_functions[];
extern const OSSL_DISPATCH ed25519_pub_to_text_encoder_functions[];
extern const OSSL_DISPATCH ed25519_priv_to_der_encoder_functions[];
extern const OSSL_DISPATCH ed25519_pub_to_der_encoder_functions[];
extern const OSSL_DISPATCH ed25519_priv_to_pem_encoder_functions[];
extern const OSSL_DISPATCH ed25519_pub_to_pem_encoder_functions[];

extern const OSSL_DISPATCH ed448_priv_to_text_encoder_functions[];
extern const OSSL_DISPATCH ed448_pub_to_text_encoder_functions[];
extern const OSSL_DISPATCH ed448_priv_to_der_encoder_functions[];
extern const OSSL_DISPATCH ed448_pub_to_der_encoder_functions[];
extern const OSSL_DISPATCH ed448_priv_to_pem_encoder_functions[];
extern const OSSL_DISPATCH ed448_pub_to_pem_encoder_functions[];

extern const OSSL_DISPATCH ec_priv_to_text_encoder_functions[];
extern const OSSL_DISPATCH ec_pub_to_text_encoder_functions[];
extern const OSSL_DISPATCH ec_param_to_text_encoder_functions[];
extern const OSSL_DISPATCH ec_priv_to_der_encoder_functions[];
extern const OSSL_DISPATCH ec_pub_to_der_encoder_functions[];
extern const OSSL_DISPATCH ec_param_to_der_encoder_functions[];
extern const OSSL_DISPATCH ec_priv_to_pem_encoder_functions[];
extern const OSSL_DISPATCH ec_pub_to_pem_encoder_functions[];
extern const OSSL_DISPATCH ec_param_to_pem_encoder_functions[];

/* Decoders */
extern const OSSL_DISPATCH der_to_dh_decoder_functions[];
extern const OSSL_DISPATCH der_to_dhx_decoder_functions[];
extern const OSSL_DISPATCH der_to_dsa_decoder_functions[];
extern const OSSL_DISPATCH msblob_to_dsa_decoder_functions[];
extern const OSSL_DISPATCH pvk_to_dsa_decoder_functions[];
extern const OSSL_DISPATCH der_to_ec_decoder_functions[];
extern const OSSL_DISPATCH der_to_x25519_decoder_functions[];
extern const OSSL_DISPATCH der_to_x448_decoder_functions[];
extern const OSSL_DISPATCH der_to_ed25519_decoder_functions[];
extern const OSSL_DISPATCH der_to_ed448_decoder_functions[];
extern const OSSL_DISPATCH der_to_rsa_decoder_functions[];
extern const OSSL_DISPATCH der_to_rsapss_decoder_functions[];
extern const OSSL_DISPATCH msblob_to_rsa_decoder_functions[];
extern const OSSL_DISPATCH pvk_to_rsa_decoder_functions[];
extern const OSSL_DISPATCH pem_to_der_decoder_functions[];

extern const OSSL_DISPATCH file_store_functions[];
