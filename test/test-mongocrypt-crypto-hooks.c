/*
 * Copyright 2019-present MongoDB, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mongocrypt-config.h"
#include "mongocrypt-crypto-private.h"
#include "mongocrypt-private.h"
#include "test-mongocrypt-crypto-std-hooks.h"

#include "test-mongocrypt.h"

#define IV_HEX "1F572A1B84EC8F99B7915AA2A2AEA2F4"
#define HMAC_HEX                                                                                                       \
    "60676DE9FD305FD2C0815763C422687270DA2416D94A917B276E9DCBB13F412F"                                                 \
    "92FA403AA8AE172BD2E4729ED352793795EE588A2977C9C1F218D2AAD779C997"
/* only the first 32 bytes are appended. */
#define HMAC_HEX_TAG "60676DE9FD305FD2C0815763C422687270DA2416D94A917B276E9DCBB13F412F"

#define HMAC_KEY_HEX "CCD3836C8F24AC5FAAFAAA630C5C6C5D210FD03934EA1440CD67E0DCDE3F8EA6"
#define ENCRYPTION_KEY_HEX "E1D1727BAF970E01181C0868CB9D3E574B47AC09771FF30FE2D093B0950C7DAF"
#define IV_KEY_HEX "0A9328FCB6405ABDF5B4BFEC243FE9CF503CD4F24360872B75F08A2A3961802B"
/* full 96 byte key consists of three "sub" keys */
#define KEY_HEX HMAC_KEY_HEX ENCRYPTION_KEY_HEX IV_KEY_HEX
#define HASH_HEX "489EC3238378DC624C74B8CC4598ACED2B7EA5DE5C5F7602D8761BAE92FD8ABE"
#define RANDOM_HEX                                                                                                     \
    "670ACBB44D4E04A279CC0B95D217493205A038C50F537F452C59EFF6541D0026670ACBB44"                                        \
    "D4E04A279CC0B95D217493205A038C50F537F452C59EFF6541D0026670ACBB44D4E04A279"                                        \
    "CC0B95D217493205A038C50F537F452C59EFF6541D0026"

/* a document containing the history of calls */
static char *call_history;

// APPEND_CALLHISTORY appends a formatted string to `call_history`.
#define APPEND_CALLHISTORY(...)                                                                                        \
    if (1) {                                                                                                           \
        char *previous = call_history;                                                                                 \
        char *addition = bson_strdup_printf(__VA_ARGS__);                                                              \
        call_history = bson_strdup_printf("%s%s", previous ? previous : "", addition);                                 \
        bson_free(addition);                                                                                           \
        bson_free(previous);                                                                                           \
    } else                                                                                                             \
        (void)0

static void _append_bin(const char *name, mongocrypt_binary_t *bin) {
    _mongocrypt_buffer_t tmp;
    char *hex;

    _mongocrypt_buffer_from_binary(&tmp, bin);
    hex = _mongocrypt_buffer_to_hex(&tmp);
    APPEND_CALLHISTORY("%s:%s\n", name, hex);
    bson_free(hex);
    _mongocrypt_buffer_cleanup(&tmp);
}

static bool _mock_aes_256_xxx_encrypt(void *ctx,
                                      mongocrypt_binary_t *key,
                                      mongocrypt_binary_t *iv,
                                      mongocrypt_binary_t *in,
                                      mongocrypt_binary_t *out,
                                      uint32_t *bytes_written,
                                      mongocrypt_status_t *status) {
    BSON_ASSERT(0 == strncmp("error_on:", (char *)ctx, strlen("error_on:")));
    APPEND_CALLHISTORY("call:%s\n", BSON_FUNC);
    _append_bin("key", key);
    if (NULL != iv) {
        _append_bin("iv", iv);
    }
    _append_bin("in", in);
    /* append it directly, don't encrypt. */
    uint8_t *out_u8 = out->data;
    memcpy(out_u8 + *bytes_written, in->data, in->len);
    *bytes_written += in->len;
    APPEND_CALLHISTORY("ret:%s\n", BSON_FUNC);
    if (0 == strcmp((char *)ctx, "error_on:aes_256_cbc_encrypt")
        || 0 == strcmp((char *)ctx, "error_on:aes_256_ctr_encrypt")
        || 0 == strcmp((char *)ctx, "error_on:aes_256_ecb_encrypt")) {
        mongocrypt_status_set(status, MONGOCRYPT_STATUS_ERROR_CLIENT, 1, (char *)ctx, -1);
        return false;
    }
    return true;
}

static bool _mock_aes_256_xxx_decrypt(void *ctx,
                                      mongocrypt_binary_t *key,
                                      mongocrypt_binary_t *iv,
                                      mongocrypt_binary_t *in,
                                      mongocrypt_binary_t *out,
                                      uint32_t *bytes_written,
                                      mongocrypt_status_t *status) {
    BSON_ASSERT(0 == strncmp("error_on:", (char *)ctx, strlen("error_on:")));
    APPEND_CALLHISTORY("call:%s\n", BSON_FUNC);
    _append_bin("key", key);
    _append_bin("iv", iv);
    _append_bin("in", in);
    /* append it directly, don't decrypt. */
    uint8_t *out_u8 = out->data;
    memcpy(out_u8 + *bytes_written, in->data, in->len);
    *bytes_written += in->len;
    APPEND_CALLHISTORY("ret:%s\n", BSON_FUNC);
    if (0 == strcmp((char *)ctx, "error_on:aes_256_cbc_decrypt")
        || 0 == strcmp((char *)ctx, "error_on:aes_256_ctr_decrypt")) {
        mongocrypt_status_set(status, MONGOCRYPT_STATUS_ERROR_CLIENT, 1, (char *)ctx, -1);
        return false;
    }
    return true;
}

static bool _hmac_sha_512(void *ctx,
                          mongocrypt_binary_t *key,
                          mongocrypt_binary_t *in,
                          mongocrypt_binary_t *out,
                          mongocrypt_status_t *status) {
    _mongocrypt_buffer_t tmp;

    BSON_ASSERT(0 == strncmp("error_on:", (char *)ctx, strlen("error_on:")));
    APPEND_CALLHISTORY("call:%s\n", BSON_FUNC);
    _append_bin("key", key);
    _append_bin("in", in);

    APPEND_CALLHISTORY("ret:%s\n", BSON_FUNC);

    _mongocrypt_buffer_copy_from_hex(&tmp, HMAC_HEX);
    memcpy(out->data, tmp.data, tmp.len);
    _mongocrypt_buffer_cleanup(&tmp);
    if (0 == strcmp((char *)ctx, "error_on:hmac_sha512")) {
        mongocrypt_status_set(status, MONGOCRYPT_STATUS_ERROR_CLIENT, 1, (char *)ctx, -1);
        return false;
    }
    return true;
}

static bool _hmac_sha_256(void *ctx,
                          mongocrypt_binary_t *key,
                          mongocrypt_binary_t *in,
                          mongocrypt_binary_t *out,
                          mongocrypt_status_t *status) {
    _mongocrypt_buffer_t tmp;

    BSON_ASSERT(0 == strncmp("error_on:", (char *)ctx, strlen("error_on:")));
    APPEND_CALLHISTORY("call:%s\n", BSON_FUNC);
    _append_bin("key", key);
    _append_bin("in", in);

    APPEND_CALLHISTORY("ret:%s\n", BSON_FUNC);

    _mongocrypt_buffer_copy_from_hex(&tmp, HASH_HEX);
    memcpy(out->data, tmp.data, tmp.len);
    _mongocrypt_buffer_cleanup(&tmp);
    if (0 == strcmp((char *)ctx, "error_on:hmac_sha256")) {
        mongocrypt_status_set(status, MONGOCRYPT_STATUS_ERROR_CLIENT, 1, (char *)ctx, -1);
        return false;
    }
    return true;
}

static bool _sha_256(void *ctx, mongocrypt_binary_t *in, mongocrypt_binary_t *out, mongocrypt_status_t *status) {
    _mongocrypt_buffer_t tmp;

    BSON_ASSERT(0 == strncmp("error_on:", (char *)ctx, strlen("error_on:")));
    APPEND_CALLHISTORY("call:%s\n", BSON_FUNC);
    _append_bin("in", in);

    APPEND_CALLHISTORY("ret:%s\n", BSON_FUNC);

    _mongocrypt_buffer_copy_from_hex(&tmp, HASH_HEX);
    memcpy(out->data, tmp.data, tmp.len);
    _mongocrypt_buffer_cleanup(&tmp);
    if (0 == strcmp((char *)ctx, "error_on:sha256")) {
        mongocrypt_status_set(status, MONGOCRYPT_STATUS_ERROR_CLIENT, 1, (char *)ctx, -1);
        return false;
    }
    return true;
}

static bool _random(void *ctx, mongocrypt_binary_t *out, uint32_t count, mongocrypt_status_t *status) {
    /* only have 32 bytes of random test data. */
    BSON_ASSERT(count <= 96);

    BSON_ASSERT(0 == strncmp("error_on:", (char *)ctx, strlen("error_on:")));
    APPEND_CALLHISTORY("call:%s\n", BSON_FUNC);
    APPEND_CALLHISTORY("count:%d\n", (int)count);
    APPEND_CALLHISTORY("ret:%s\n", BSON_FUNC);

    _mongocrypt_buffer_t tmp;
    _mongocrypt_buffer_copy_from_hex(&tmp, RANDOM_HEX);
    memcpy(out->data, tmp.data, count);
    _mongocrypt_buffer_cleanup(&tmp);
    if (0 == strcmp((char *)ctx, "error_on:random")) {
        mongocrypt_status_set(status, MONGOCRYPT_STATUS_ERROR_CLIENT, 1, (char *)ctx, -1);
        return false;
    }
    return true;
}

static bool _sign_rsaes_pkcs1_v1_5(void *ctx,
                                   mongocrypt_binary_t *key,
                                   mongocrypt_binary_t *in,
                                   mongocrypt_binary_t *out,
                                   mongocrypt_status_t *status) {
    _mongocrypt_buffer_t tmp;

    BSON_ASSERT(0 == strncmp("error_on:", (char *)ctx, strlen("error_on:")));
    APPEND_CALLHISTORY("call:%s\n", BSON_FUNC);
    _append_bin("key", key);
    _append_bin("in", in);

    APPEND_CALLHISTORY("ret:%s\n", BSON_FUNC);
    memset(out->data, 0, out->len);

    _mongocrypt_buffer_copy_from_hex(&tmp, HASH_HEX);
    memcpy(out->data, tmp.data, tmp.len);
    _mongocrypt_buffer_cleanup(&tmp);
    if (0 == strcmp((char *)ctx, "error_on:sign_rsaes_pkcs1_v1_5")) {
        mongocrypt_status_set(status, MONGOCRYPT_STATUS_ERROR_CLIENT, 1, (char *)ctx, -1);
        return false;
    }
    return true;
}

static mongocrypt_t *
_create_mongocrypt_and_hooks(_mongocrypt_tester_t *tester, const char *error_on, bool ctr_hook, bool ecb_hook) {
    bool ret;

    mongocrypt_t *crypt = mongocrypt_new();
    ASSERT_OK(mongocrypt_setopt_kms_provider_aws(crypt, "example", -1, "example", -1), crypt);
    ASSERT_OK(mongocrypt_setopt_kms_providers(crypt, TEST_BSON("{'gcp': { 'email': 'test', 'privateKey': 'AAAA'}}")),
              crypt);
    ret = mongocrypt_setopt_crypto_hooks(crypt,
                                         _mock_aes_256_xxx_encrypt,
                                         _mock_aes_256_xxx_decrypt,
                                         _random,
                                         _hmac_sha_512,
                                         _hmac_sha_256,
                                         _sha_256,
                                         (void *)error_on);
    ASSERT_OK(ret, crypt);
    ret = mongocrypt_setopt_crypto_hook_sign_rsaes_pkcs1_v1_5(crypt, _sign_rsaes_pkcs1_v1_5, (void *)error_on);
    ASSERT_OK(ret, crypt);
    if (ctr_hook) {
        ret = mongocrypt_setopt_aes_256_ctr(crypt,
                                            _mock_aes_256_xxx_encrypt,
                                            _mock_aes_256_xxx_decrypt,
                                            (void *)error_on);
        ASSERT_OK(ret, crypt);
    }
    if (ecb_hook) {
        ret = mongocrypt_setopt_aes_256_ecb(crypt, _mock_aes_256_xxx_encrypt, (void *)error_on);
        ASSERT_OK(ret, crypt);
    }
    ASSERT_OK(_mongocrypt_init_for_test(crypt), crypt);
    return crypt;
}

static mongocrypt_t *_create_mongocrypt(_mongocrypt_tester_t *tester, const char *error_on) {
    return _create_mongocrypt_and_hooks(tester, error_on, false, false);
}

static void
_test_crypto_hooks_encryption_helper(_mongocrypt_tester_t *tester, const char *error_on, bool ctr_hook, bool ecb_hook) {
    mongocrypt_t *crypt;
    bool ret;
    uint32_t bytes_written;
    mongocrypt_status_t *status;
    _mongocrypt_buffer_t iv, associated_data, key, plaintext, ciphertext;
    const char *expected_call_history = "call:_mock_aes_256_xxx_encrypt\n"
                                        "key:" ENCRYPTION_KEY_HEX "\n"
                                        "iv:" IV_HEX "\n"
                                        "in:BBBB0E0E0E0E0E0E0E0E0E0E0E0E0E0E\n"
                                        "ret:_mock_aes_256_xxx_encrypt\n"
                                        "call:_hmac_sha_512\n"
                                        "key:CCD3836C8F24AC5FAAFAAA630C5C6C5D210FD03934EA1440CD67E0DCDE3F8EA6\n"
                                        "in:AAAA" IV_HEX "BBBB0E0E0E0E0E0E0E0E0E0E0E0E0E0E0000000000000010\n"
                                        "ret:_hmac_sha_512\n";

    status = mongocrypt_status_new();
    crypt = _create_mongocrypt_and_hooks(tester, error_on, ctr_hook, ecb_hook);

    _mongocrypt_buffer_copy_from_hex(&iv, IV_HEX);
    _mongocrypt_buffer_copy_from_hex(&associated_data, "AAAA");
    _mongocrypt_buffer_copy_from_hex(&plaintext, "BBBB");

    call_history = NULL;

    if (ctr_hook || ecb_hook) {
        const _mongocrypt_value_encryption_algorithm_t *fle2alg = _mcFLE2Algorithm();
        _mongocrypt_buffer_copy_from_hex(&key, ENCRYPTION_KEY_HEX);
        _mongocrypt_buffer_init(&ciphertext);
        _mongocrypt_buffer_resize(&ciphertext, fle2alg->get_ciphertext_len(plaintext.len, status));
        ret =
            fle2alg
                ->do_encrypt(crypt->crypto, &iv, NULL /* aad */, &key, &plaintext, &ciphertext, &bytes_written, status);
    } else {
        const _mongocrypt_value_encryption_algorithm_t *fle1alg = _mcFLE1Algorithm();
        _mongocrypt_buffer_copy_from_hex(&key, KEY_HEX);
        _mongocrypt_buffer_init(&ciphertext);
        _mongocrypt_buffer_resize(&ciphertext, fle1alg->get_ciphertext_len(plaintext.len, status));
        ret = fle1alg->do_encrypt(crypt->crypto,
                                  &iv,
                                  &associated_data,
                                  &key,
                                  &plaintext,
                                  &ciphertext,
                                  &bytes_written,
                                  status);
    }

    if (0 == strcmp(error_on, "error_on:none")) {
        ASSERT_OK_STATUS(ret, status);
        ciphertext.len = bytes_written;

        /* Check the full trace. */
        ASSERT_STREQUAL(call_history, expected_call_history);

        /* Check the structure of the ciphertext */
        BSON_ASSERT(0
                    == _mongocrypt_buffer_cmp_hex(&ciphertext,
                                                  IV_HEX "BBBB0E0E0E0E0E0E0E0E0E0E0E0E0E0E" /* the "encrypted"
                                                                                             block which is
                                                                                             really plaintext.
                                                                                             BBBB + padding. */
                                                  HMAC_HEX_TAG));
    } else {
        ASSERT_FAILS_STATUS(ret, status, error_on);
    }

    _mongocrypt_buffer_cleanup(&key);
    _mongocrypt_buffer_cleanup(&iv);
    _mongocrypt_buffer_cleanup(&associated_data);
    _mongocrypt_buffer_cleanup(&plaintext);
    _mongocrypt_buffer_cleanup(&ciphertext);
    mongocrypt_status_destroy(status);
    mongocrypt_destroy(crypt);
    bson_free(call_history);
}

static void _test_crypto_hooks_encryption(_mongocrypt_tester_t *tester) {
    _test_crypto_hooks_encryption_helper(tester, "error_on:none", false, false);
    _test_crypto_hooks_encryption_helper(tester, "error_on:aes_256_cbc_encrypt", false, false);
    _test_crypto_hooks_encryption_helper(tester, "error_on:aes_256_ctr_encrypt", true, false);
    _test_crypto_hooks_encryption_helper(tester, "error_on:aes_256_ecb_encrypt", false, true);
    _test_crypto_hooks_encryption_helper(tester, "error_on:hmac_sha512", false, false);
}

static void
_test_crypto_hooks_decryption_helper(_mongocrypt_tester_t *tester, const char *error_on, bool ctr_hook, bool ecb_hook) {
    mongocrypt_t *crypt;
    bool ret;
    uint32_t bytes_written;
    mongocrypt_status_t *status;
    _mongocrypt_buffer_t associated_data, key, plaintext, ciphertext;
    const char *expected_call_history = "call:_hmac_sha_512\n"
                                        "key:" HMAC_KEY_HEX "\n"
                                        "in:AAAA" IV_HEX "BBBB0E0E0E0E0E0E0E0E0E0E0E0E0E0E0000000000000010\n"
                                        "ret:_hmac_sha_512\n"
                                        "call:_mock_aes_256_xxx_decrypt\n"
                                        "key:" ENCRYPTION_KEY_HEX "\n"
                                        "iv:" IV_HEX "\n"
                                        "in:BBBB0E0E0E0E0E0E0E0E0E0E0E0E0E0E\n"
                                        "ret:_mock_aes_256_xxx_decrypt\n";

    status = mongocrypt_status_new();
    crypt = _create_mongocrypt_and_hooks(tester, error_on, ctr_hook, ecb_hook);

    _mongocrypt_buffer_copy_from_hex(&associated_data, "AAAA");
    _mongocrypt_buffer_copy_from_hex(&ciphertext, IV_HEX "BBBB0E0E0E0E0E0E0E0E0E0E0E0E0E0E" HMAC_HEX_TAG);

    call_history = NULL;

    if (ctr_hook || ecb_hook) {
        const _mongocrypt_value_encryption_algorithm_t *fle2alg = _mcFLE2Algorithm();
        _mongocrypt_buffer_copy_from_hex(&key, ENCRYPTION_KEY_HEX);
        _mongocrypt_buffer_init(&plaintext);
        _mongocrypt_buffer_resize(&plaintext, fle2alg->get_plaintext_len(ciphertext.len, status));

        ret = fle2alg->do_decrypt(crypt->crypto, NULL /* aad */, &key, &ciphertext, &plaintext, &bytes_written, status);
    } else {
        const _mongocrypt_value_encryption_algorithm_t *fle1alg = _mcFLE1Algorithm();
        _mongocrypt_buffer_copy_from_hex(&key, KEY_HEX);
        _mongocrypt_buffer_init(&plaintext);
        _mongocrypt_buffer_resize(&plaintext, fle1alg->get_plaintext_len(ciphertext.len, status));

        ret =
            fle1alg->do_decrypt(crypt->crypto, &associated_data, &key, &ciphertext, &plaintext, &bytes_written, status);
    }

    if (0 == strcmp(error_on, "error_on:none")) {
        ASSERT_OK_STATUS(ret, status);
        plaintext.len = bytes_written;

        /* Check the full trace. */
        ASSERT_STREQUAL(call_history, expected_call_history);

        /* Check the resulting plaintext */
        BSON_ASSERT(0 == _mongocrypt_buffer_cmp_hex(&plaintext, "BBBB"));
    } else {
        ASSERT_FAILS_STATUS(ret, status, error_on);
    }

    _mongocrypt_buffer_cleanup(&key);
    _mongocrypt_buffer_cleanup(&associated_data);
    _mongocrypt_buffer_cleanup(&plaintext);
    _mongocrypt_buffer_cleanup(&ciphertext);
    mongocrypt_status_destroy(status);
    mongocrypt_destroy(crypt);
    bson_free(call_history);
}

static void _test_crypto_hooks_decryption(_mongocrypt_tester_t *tester) {
    _test_crypto_hooks_decryption_helper(tester, "error_on:none", false, false);
    _test_crypto_hooks_decryption_helper(tester, "error_on:aes_256_cbc_decrypt", false, false);
    _test_crypto_hooks_decryption_helper(tester, "error_on:aes_256_ctr_decrypt", true, false);
    _test_crypto_hooks_decryption_helper(tester, "error_on:aes_256_ecb_encrypt", false, true);
    _test_crypto_hooks_decryption_helper(tester, "error_on:hmac_sha512", false, false);
}

static void _test_crypto_hooks_iv_gen_helper(_mongocrypt_tester_t *tester, char *error_on) {
    mongocrypt_t *crypt;
    bool ret;
    mongocrypt_status_t *status;
    _mongocrypt_buffer_t associated_data, key, plaintext, iv;
    char *expected_iv = bson_strndup(HMAC_HEX_TAG, 16 * 2); /* only the first 16 bytes are used for IV. */
    const char *expected_call_history = "call:_hmac_sha_512\n"
                                        "key:" IV_KEY_HEX "\n"
                                        "in:AAAA0000000000000010BBBB\n"
                                        "ret:_hmac_sha_512\n";

    status = mongocrypt_status_new();
    crypt = _create_mongocrypt(tester, error_on);

    _mongocrypt_buffer_copy_from_hex(&associated_data, "AAAA");
    _mongocrypt_buffer_copy_from_hex(&key, KEY_HEX);
    _mongocrypt_buffer_copy_from_hex(&plaintext, "BBBB");

    _mongocrypt_buffer_init(&iv);
    _mongocrypt_buffer_resize(&iv, MONGOCRYPT_IV_LEN);

    call_history = NULL;

    ret = _mongocrypt_calculate_deterministic_iv(crypt->crypto, &key, &plaintext, &associated_data, &iv, status);

    if (0 == strcmp(error_on, "error_on:none")) {
        ASSERT_OK_STATUS(ret, status);

        /* Check the full trace. */
        ASSERT_STREQUAL(call_history, expected_call_history);

        /* Check the resulting iv */
        BSON_ASSERT(0 == _mongocrypt_buffer_cmp_hex(&iv, expected_iv));
    } else {
        ASSERT_FAILS_STATUS(ret, status, error_on);
    }

    bson_free(expected_iv);
    _mongocrypt_buffer_cleanup(&key);
    _mongocrypt_buffer_cleanup(&associated_data);
    _mongocrypt_buffer_cleanup(&plaintext);
    _mongocrypt_buffer_cleanup(&iv);
    mongocrypt_status_destroy(status);
    mongocrypt_destroy(crypt);
    bson_free(call_history);
}

static void _test_crypto_hooks_iv_gen(_mongocrypt_tester_t *tester) {
    _test_crypto_hooks_iv_gen_helper(tester, "error_on:none");
    _test_crypto_hooks_iv_gen_helper(tester, "error_on:hmac_sha512");
}

static void _test_crypto_hooks_random_helper(_mongocrypt_tester_t *tester, const char *error_on) {
    mongocrypt_t *crypt;
    bool ret;
    mongocrypt_status_t *status;
    _mongocrypt_buffer_t random;
    const char *expected_call_history = "call:_random\n"
                                        "count:96\n"
                                        "ret:_random\n";

    status = mongocrypt_status_new();
    crypt = _create_mongocrypt(tester, error_on);

    _mongocrypt_buffer_init(&random);
    _mongocrypt_buffer_resize(&random, 96);

    call_history = NULL;

    ret = _mongocrypt_random(crypt->crypto, &random, random.len, status);

    if (0 == strcmp(error_on, "error_on:none")) {
        ASSERT_OK_STATUS(ret, status);

        /* Check the full trace. */
        ASSERT_STREQUAL(call_history, expected_call_history);

        /* Check the resulting iv */
        BSON_ASSERT(0 == _mongocrypt_buffer_cmp_hex(&random, RANDOM_HEX));
    } else {
        ASSERT_FAILS_STATUS(ret, status, error_on);
    }

    _mongocrypt_buffer_cleanup(&random);
    mongocrypt_status_destroy(status);
    mongocrypt_destroy(crypt);
    bson_free(call_history);
}

static void _test_crypto_hooks_random(_mongocrypt_tester_t *tester) {
    _test_crypto_hooks_random_helper(tester, "error_on:none");
    _test_crypto_hooks_random_helper(tester, "error_on:random");
}

static void _test_kms_request_helper(_mongocrypt_tester_t *tester, const char *error_on) {
    mongocrypt_t *crypt;
    mongocrypt_status_t *status;
    mongocrypt_ctx_t *ctx;
    bool ret;

    status = mongocrypt_status_new();
    crypt = _create_mongocrypt(tester, error_on);
    ctx = mongocrypt_ctx_new(crypt);

    call_history = NULL;

    ASSERT_OK(mongocrypt_ctx_setopt_masterkey_aws(ctx, "us-east-1", -1, "cmk", -1), ctx);

    mongocrypt_ctx_datakey_init(ctx);
    ret = mongocrypt_ctx_status(ctx, status);

    if (0 == strcmp(error_on, "error_on:none")) {
        ASSERT_OK_STATUS(ret, status);

        /* The call history includes some random data, just assert we've called
         * our hooks. */
        BSON_ASSERT(strstr(call_history, "call:_hmac_sha_256"));
        BSON_ASSERT(strstr(call_history, "call:_sha_256"));
    } else {
        ASSERT_FAILS_STATUS(ret, status, error_on);
    }

    mongocrypt_ctx_destroy(ctx);
    mongocrypt_status_destroy(status);
    mongocrypt_destroy(crypt);
    bson_free(call_history);
}

static void _test_kms_request(_mongocrypt_tester_t *tester) {
    _test_kms_request_helper(tester, "error_on:none");
    _test_kms_request_helper(tester, "error_on:hmac_sha256");
    _test_kms_request_helper(tester, "error_on:sha256");
}

static void _test_crypto_hooks_unset(_mongocrypt_tester_t *tester) {
    mongocrypt_t *crypt;

    crypt = mongocrypt_new();
    mongocrypt_setopt_kms_provider_aws(crypt, "example", -1, "example", -1);
    ASSERT_OK(_mongocrypt_init_for_test(crypt), crypt);
    mongocrypt_destroy(crypt);
}

/* test a bug fix, that an error on explicit encryption in the crypto hooks sets
 * the context state */
static void _test_crypto_hooks_explicit_err(_mongocrypt_tester_t *tester) {
    mongocrypt_t *crypt;
    mongocrypt_ctx_t *ctx;
    mongocrypt_binary_t *bin, *key_id;
    const char *deterministic = MONGOCRYPT_ALGORITHM_DETERMINISTIC_STR;

    call_history = NULL;

    /* error on something during encryption. */
    crypt = _create_mongocrypt(tester, "error_on:hmac_sha512");

    ctx = mongocrypt_ctx_new(crypt);
    key_id = mongocrypt_binary_new_from_data(MONGOCRYPT_DATA_AND_LEN("aaaaaaaaaaaaaaaa"));

    ASSERT_OK(mongocrypt_ctx_setopt_algorithm(ctx, deterministic, -1), ctx);
    ASSERT_OK(mongocrypt_ctx_setopt_key_id(ctx, key_id), ctx);
    ASSERT_OK(mongocrypt_ctx_explicit_encrypt_init(ctx, TEST_BSON("{'v': 123}")), ctx);

    _mongocrypt_tester_run_ctx_to(tester, ctx, MONGOCRYPT_CTX_READY);
    bin = mongocrypt_binary_new();
    ASSERT_FAILS(mongocrypt_ctx_finalize(ctx, bin), ctx, "error_on:hmac_sha512");
    BSON_ASSERT(MONGOCRYPT_CTX_ERROR == mongocrypt_ctx_state(ctx));
    mongocrypt_binary_destroy(bin);
    mongocrypt_binary_destroy(key_id);
    mongocrypt_ctx_destroy(ctx);
    mongocrypt_destroy(crypt);
    bson_free(call_history);
}

/* validate that sha256 errors are handled correctly */
static void _test_crypto_hooks_explicit_sha256_err(_mongocrypt_tester_t *tester) {
    mongocrypt_t *crypt;
    mongocrypt_status_t *status;
    mongocrypt_ctx_t *ctx;

    status = mongocrypt_status_new();
    crypt = _create_mongocrypt(tester, "error_on:sha256");
    ctx = mongocrypt_ctx_new(crypt);

    call_history = NULL;

    ASSERT_OK(mongocrypt_ctx_setopt_masterkey_aws(ctx, "us-east-1", -1, "cmk", -1), ctx);
    ASSERT_FAILS(mongocrypt_ctx_datakey_init(ctx), ctx, "failed to create KMS message");

    mongocrypt_ctx_destroy(ctx);
    mongocrypt_status_destroy(status);
    mongocrypt_destroy(crypt);
    bson_free(call_history);
}

static void _test_crypto_hook_sign_rsaes_pkcs1_v1_5(_mongocrypt_tester_t *tester) {
    mongocrypt_t *crypt;
    mongocrypt_ctx_t *ctx;

    crypt = _create_mongocrypt(tester, "error_on:none");
    call_history = NULL;

    ctx = mongocrypt_ctx_new(crypt);
    mongocrypt_ctx_setopt_key_encryption_key(ctx,
                                             TEST_BSON("{'provider': 'gcp', 'projectId': 'test', 'location': "
                                                       "'global', 'keyRing': 'ring', 'keyName': 'key'}"));
    ASSERT_OK(mongocrypt_ctx_datakey_init(ctx), ctx);

    BSON_ASSERT(strstr(call_history, "call:_sign_rsaes_pkcs1_v1_5"));
    BSON_ASSERT(strstr(call_history, "key:000000"));

    mongocrypt_ctx_destroy(ctx);
    mongocrypt_destroy(crypt);
    bson_free(call_history);

    /* Test error when creating a data key. */
    crypt = _create_mongocrypt(tester, "error_on:sign_rsaes_pkcs1_v1_5");
    ctx = mongocrypt_ctx_new(crypt);
    call_history = NULL;

    mongocrypt_ctx_setopt_key_encryption_key(ctx,
                                             TEST_BSON("{'provider': 'gcp', 'projectId': 'test', 'location': "
                                                       "'global', 'keyRing': 'ring', 'keyName': 'key'}"));
    ASSERT_FAILS(mongocrypt_ctx_datakey_init(ctx), ctx, "error_on:sign_rsaes_pkcs1_v1_5");

    mongocrypt_ctx_destroy(ctx);
    mongocrypt_destroy(crypt);
    bson_free(call_history);

    /* Test error when encrypting. */
    crypt = _create_mongocrypt(tester, "error_on:sign_rsaes_pkcs1_v1_5");
    ctx = mongocrypt_ctx_new(crypt);
    call_history = NULL;

    ASSERT_OK(mongocrypt_ctx_encrypt_init(ctx, "test", -1, TEST_FILE("./test/example/cmd.json")), ctx);
    _mongocrypt_tester_run_ctx_to(tester, ctx, MONGOCRYPT_CTX_NEED_MONGO_KEYS);
    ASSERT_FAILS(mongocrypt_ctx_mongo_feed(ctx, TEST_FILE("./test/data/key-document-gcp.json")),
                 ctx,
                 "error_on:sign_rsaes_pkcs1_v1_5");

    mongocrypt_ctx_destroy(ctx);
    mongocrypt_destroy(crypt);
    bson_free(call_history);
}

#ifdef MONGOCRYPT_ENABLE_CRYPTO_LIBCRYPTO
bool _native_crypto_aes_256_ecb_encrypt(aes_256_args_t args);

static bool _aes_256_ecb_encrypt(void *ctx,
                                 mongocrypt_binary_t *key,
                                 mongocrypt_binary_t *iv,
                                 mongocrypt_binary_t *in,
                                 mongocrypt_binary_t *out,
                                 uint32_t *bytes_written,
                                 mongocrypt_status_t *status) {
    _mongocrypt_buffer_t key_buf;
    _mongocrypt_buffer_from_binary(&key_buf, key);
    if (iv) {
        CLIENT_ERR("IV expected to be NULL in this mode");
        return false;
    }
    _mongocrypt_buffer_t in_buf;
    _mongocrypt_buffer_from_binary(&in_buf, in);
    _mongocrypt_buffer_t out_buf;
    _mongocrypt_buffer_from_binary(&out_buf, out);

    aes_256_args_t args = {&key_buf, NULL, &in_buf, &out_buf, bytes_written, status};

    return _native_crypto_aes_256_ecb_encrypt(args);
}

static void _test_fle2_crypto_via_ecb_hook(_mongocrypt_tester_t *tester) {
    const _mongocrypt_value_encryption_algorithm_t *fle2alg = _mcFLE2Algorithm();
    bool ret;
    _mongocrypt_buffer_t key;
    _mongocrypt_buffer_t iv;
    _mongocrypt_buffer_t plaintext;
    _mongocrypt_buffer_t ciphertext_reg;
    _mongocrypt_buffer_t ciphertext_ecb;
    _mongocrypt_buffer_t plaintext_ecb;
    uint32_t bytes_written;
    mongocrypt_status_t *status = mongocrypt_status_new();

    _mongocrypt_buffer_copy_from_hex(&iv, IV_HEX);
    _mongocrypt_buffer_copy_from_hex(&plaintext, "4f6c64204d63446f6e616c64206861642061206661726d2e20456965696f0a");
    _mongocrypt_buffer_copy_from_hex(&key, ENCRYPTION_KEY_HEX);

    /* Encrypt data using native CTR and ECB-hook and compare */

    mongocrypt_t *crypt_reg = mongocrypt_new();
    _mongocrypt_buffer_init(&ciphertext_reg);
    _mongocrypt_buffer_resize(&ciphertext_reg, fle2alg->get_ciphertext_len(plaintext.len, status));
    ret = fle2alg->do_encrypt(crypt_reg->crypto,
                              &iv,
                              NULL /* aad */,
                              &key,
                              &plaintext,
                              &ciphertext_reg,
                              &bytes_written,
                              status);
    ASSERT_OK(ret, crypt_reg);

    mongocrypt_t *crypt_ecb = mongocrypt_new();
    ret = mongocrypt_setopt_aes_256_ecb(crypt_ecb, _aes_256_ecb_encrypt, NULL);
    ASSERT_OK(ret, crypt_ecb);
    _mongocrypt_buffer_init(&ciphertext_ecb);
    _mongocrypt_buffer_resize(&ciphertext_ecb, fle2alg->get_ciphertext_len(plaintext.len, status));
    ret = fle2alg->do_encrypt(crypt_ecb->crypto,
                              &iv,
                              NULL /* aad */,
                              &key,
                              &plaintext,
                              &ciphertext_ecb,
                              &bytes_written,
                              status);
    ASSERT_OK(ret, crypt_ecb);

    ASSERT(0 == _mongocrypt_buffer_cmp(&ciphertext_reg, &ciphertext_ecb));

    /* Decrypt data using ECB-hook and compare to original */

    _mongocrypt_buffer_init(&plaintext_ecb);
    _mongocrypt_buffer_resize(&plaintext_ecb, fle2alg->get_plaintext_len(ciphertext_ecb.len, status));
    ret = fle2alg->do_decrypt(crypt_ecb->crypto,
                              NULL /* aad */,
                              &key,
                              &ciphertext_ecb,
                              &plaintext_ecb,
                              &bytes_written,
                              status);
    ASSERT_OK(ret, crypt_ecb);

    ASSERT(0 == _mongocrypt_buffer_cmp(&plaintext, &plaintext_ecb));

    _mongocrypt_buffer_cleanup(&key);
    _mongocrypt_buffer_cleanup(&iv);
    _mongocrypt_buffer_cleanup(&plaintext);
    _mongocrypt_buffer_cleanup(&ciphertext_reg);
    _mongocrypt_buffer_cleanup(&ciphertext_ecb);
    _mongocrypt_buffer_cleanup(&plaintext_ecb);
    mongocrypt_destroy(crypt_reg);
    mongocrypt_destroy(crypt_ecb);
    mongocrypt_status_destroy(status);
}
#endif

static void test_is_crypto_available_with_crypto_required(_mongocrypt_tester_t *tester) {
    // libmongocrypt is built with native crypto.
    ASSERT(mongocrypt_is_crypto_available());
}

static void test_is_crypto_available_with_crypto_prohibited(_mongocrypt_tester_t *tester) {
    // libmongocrypt is not built with native crypto.
    ASSERT(!mongocrypt_is_crypto_available());
}

static int ctr_encrypt_count;

static bool _aes_256_ctr_encrypt_and_count(void *ctx,
                                           mongocrypt_binary_t *key,
                                           mongocrypt_binary_t *iv,
                                           mongocrypt_binary_t *in,
                                           mongocrypt_binary_t *out,
                                           uint32_t *bytes_written,
                                           mongocrypt_status_t *status) {
    ctr_encrypt_count++;
    return _std_hook_native_crypto_aes_256_ctr_encrypt(ctx, key, iv, in, out, bytes_written, status);
}

static int ctr_decrypt_count;

static bool _aes_256_ctr_decrypt_and_count(void *ctx,
                                           mongocrypt_binary_t *key,
                                           mongocrypt_binary_t *iv,
                                           mongocrypt_binary_t *in,
                                           mongocrypt_binary_t *out,
                                           uint32_t *bytes_written,
                                           mongocrypt_status_t *status) {
    ctr_decrypt_count++;
    return _std_hook_native_crypto_aes_256_ctr_decrypt(ctx, key, iv, in, out, bytes_written, status);
}

static void test_setting_only_ctr_hook(_mongocrypt_tester_t *tester) {
    // Test that the CTR hook can be set without setting other crypto hooks.
    // This enables supporting macOS <= 10.14 in bindings using libmongocrypt with native crypto.
    // macOS <= 10.14 does not support native CTR encryption.

    if (!_aes_ctr_is_supported_by_os) {
        TEST_PRINTF("Common Crypto with no CTR support detected. Skipping.");
        return;
    }

    // Configure KMS providers with local KEK used to encrypt key documents.
    mongocrypt_binary_t *kms_providers =
        TEST_BSON("{'local' : { 'key': 'AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA"
                  "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA'}}");
    _mongocrypt_buffer_t key_id1, key_id2;
    _mongocrypt_buffer_copy_from_hex(&key_id1, "12345678123498761234123456789012");
    _mongocrypt_buffer_copy_from_hex(&key_id2, "ABCDEFAB123498761234123456789012");

    mongocrypt_t *crypt = mongocrypt_new();
    ASSERT_OK(
        mongocrypt_setopt_aes_256_ctr(crypt, _aes_256_ctr_encrypt_and_count, _aes_256_ctr_decrypt_and_count, NULL),
        crypt);
    ASSERT_OK(mongocrypt_setopt_kms_providers(crypt, kms_providers), crypt);
    ASSERT_OK(_mongocrypt_init_for_test(crypt), crypt);

    // Encrypt with an algorithm using the CTR hook.
    ctr_encrypt_count = 0;
    {
        mongocrypt_ctx_t *ctx = mongocrypt_ctx_new(crypt);
        ASSERT_OK(mongocrypt_ctx_setopt_key_id(ctx, _mongocrypt_buffer_as_binary(&key_id1)), ctx);
        ASSERT_OK(mongocrypt_ctx_setopt_contention_factor(ctx, 1), ctx);
        ASSERT_OK(mongocrypt_ctx_setopt_algorithm(ctx, MONGOCRYPT_ALGORITHM_INDEXED_STR, -1), ctx);
        ASSERT_OK(mongocrypt_ctx_explicit_encrypt_init(ctx, TEST_BSON("{'v' : 123}")), ctx);

        ASSERT_STATE_EQUAL(mongocrypt_ctx_state(ctx), MONGOCRYPT_CTX_NEED_MONGO_KEYS);
        ASSERT_OK(mongocrypt_ctx_mongo_feed(ctx,
                                            TEST_FILE("./test/data/keys/"
                                                      "12345678123498761234123456789012-local-"
                                                      "document.json")),
                  ctx);
        ASSERT_OK(mongocrypt_ctx_mongo_done(ctx), ctx);
        ASSERT_STATE_EQUAL(mongocrypt_ctx_state(ctx), MONGOCRYPT_CTX_READY);
        mongocrypt_binary_t *ciphertext = mongocrypt_binary_new();
        ASSERT_OK(mongocrypt_ctx_finalize(ctx, ciphertext), ctx);
        mongocrypt_binary_destroy(ciphertext);
        mongocrypt_ctx_destroy(ctx);
    }
    // CTR encrypt hook is called.
    ASSERT_CMPINT(ctr_encrypt_count, ==, 1);

    // Decrypt with an algorithm using the CTR hook.
    ctr_decrypt_count = 0;
    {
        // `ieev_payload_base64` is an IEEV payload, which uses the CTR hook to decrypt.
        const char *ieev_payload_base64 = "BxI0VngSNJh2EjQSNFZ4kBICQ7uhTd9C2oI8M1afRon0ZaYG0s6oTmt0aBZ9kO4S4mm5vId01"
                                          "BsW7tBHytA8pDJ2IiWBCmah3OGH2M4ET7PSqekQD4gkUCo4JeEttx4yj05Ou4D6yZUmYfVKmE"
                                          "ljge16NCxKm7Ir9gvmQsp8x1wqGBzpndA6gkqFxsxfvQ/"
                                          "cIqOwMW9dGTTWsfKge+jYkCUIFMfms+XyC/8evQhjjA+qR6eEmV+N/"
                                          "kwpR7Q7TJe0lwU5kw2kSe3/KiPKRZZTbn8znadvycfJ0cCWGad9SQ==";

        mongocrypt_ctx_t *ctx = mongocrypt_ctx_new(crypt);
        ASSERT_OK(mongocrypt_ctx_explicit_decrypt_init(
                      ctx,
                      TEST_BSON("{'v':{'$binary':{'base64': '%s','subType':'6'}}}", ieev_payload_base64)),
                  ctx);

        ASSERT_STATE_EQUAL(mongocrypt_ctx_state(ctx), MONGOCRYPT_CTX_NEED_MONGO_KEYS);
        ASSERT_OK(mongocrypt_ctx_mongo_feed(ctx,
                                            TEST_FILE("./test/data/keys/"
                                                      "ABCDEFAB123498761234123456789012-local-"
                                                      "document.json")),
                  ctx);
        ASSERT_OK(mongocrypt_ctx_mongo_done(ctx), ctx);
        ASSERT_STATE_EQUAL(mongocrypt_ctx_state(ctx), MONGOCRYPT_CTX_READY);
        mongocrypt_binary_t *decrypted = mongocrypt_binary_new();
        ASSERT_OK(mongocrypt_ctx_finalize(ctx, decrypted), ctx);
        ASSERT_MONGOCRYPT_BINARY_EQUAL_BSON(decrypted, TEST_BSON("{'v': 'value123'}"));
        mongocrypt_binary_destroy(decrypted);
        mongocrypt_ctx_destroy(ctx);
    }
    // CTR decrypt hook is called repeatedly.
    ASSERT_CMPINT(ctr_decrypt_count, ==, 4);

    _mongocrypt_buffer_cleanup(&key_id2);
    _mongocrypt_buffer_cleanup(&key_id1);
    mongocrypt_destroy(crypt);
}

void _mongocrypt_tester_install_crypto_hooks(_mongocrypt_tester_t *tester) {
    INSTALL_TEST_CRYPTO(_test_crypto_hooks_encryption, CRYPTO_OPTIONAL);
    INSTALL_TEST_CRYPTO(_test_crypto_hooks_decryption, CRYPTO_OPTIONAL);
    INSTALL_TEST_CRYPTO(_test_crypto_hooks_iv_gen, CRYPTO_OPTIONAL);
    INSTALL_TEST_CRYPTO(_test_crypto_hooks_random, CRYPTO_OPTIONAL);
    INSTALL_TEST_CRYPTO(_test_kms_request, CRYPTO_OPTIONAL);
    INSTALL_TEST_CRYPTO(_test_crypto_hooks_unset, CRYPTO_PROHIBITED);
    INSTALL_TEST_CRYPTO(_test_crypto_hooks_explicit_err, CRYPTO_OPTIONAL);
    INSTALL_TEST_CRYPTO(_test_crypto_hooks_explicit_sha256_err, CRYPTO_OPTIONAL);
    INSTALL_TEST_CRYPTO(_test_crypto_hook_sign_rsaes_pkcs1_v1_5, CRYPTO_OPTIONAL);
    INSTALL_TEST_CRYPTO(test_is_crypto_available_with_crypto_required, CRYPTO_REQUIRED);
    INSTALL_TEST_CRYPTO(test_is_crypto_available_with_crypto_prohibited, CRYPTO_PROHIBITED);
    INSTALL_TEST_CRYPTO(test_setting_only_ctr_hook, CRYPTO_REQUIRED);
#ifdef MONGOCRYPT_ENABLE_CRYPTO_LIBCRYPTO
    INSTALL_TEST(_test_fle2_crypto_via_ecb_hook);
#endif
}
