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

#ifndef TEST_MONGOCRYPT_H
#define TEST_MONGOCRYPT_H

#include <bson/bson.h>
#include <stdint.h>

#include "mongocrypt-buffer-private.h"
#include "mongocrypt-ctx-private.h"
#include "mongocrypt-key-broker-private.h"
#include "mongocrypt-kms-ctx-private.h"
#include "mongocrypt-private.h"
#include "mongocrypt.h"

#include "test-mongocrypt-assert.h"
#include "test-mongocrypt-util.h"

struct __mongocrypt_tester_t;
typedef void (*_mongocrypt_test_fn)(struct __mongocrypt_tester_t *tester);

typedef enum tester_mongocrypt_flags {
    /// Default settings for creating a mongocrypt_t for testing
    TESTER_MONGOCRYPT_DEFAULT = 0,
    /// Create a mongocrypt_t that has the crypt_shared library loaded. A
    /// crypt_shared library must be present in the same directory as the test
    /// executable.
    TESTER_MONGOCRYPT_WITH_CRYPT_SHARED_LIB = 1 << 0,
    /// Short cache expiration
    TESTER_MONGOCRYPT_WITH_SHORT_CACHE = 1 << 3,
    /// Do not call `mongocrypt_init` yet to allow for further configuration of the resulting `mongocrypt_t`.
    TESTER_MONGOCRYPT_SKIP_INIT = 1 << 4,
} tester_mongocrypt_flags;

/* Arbitrary max of 2148 instances of temporary test data. Increase as needed.
 */
#define TEST_DATA_COUNT 2148

typedef struct __mongocrypt_tester_t {
    int test_count;
    char *test_names[TEST_DATA_COUNT];
    _mongocrypt_test_fn test_fns[TEST_DATA_COUNT];

    int file_count;
    char *file_paths[TEST_DATA_COUNT];
    _mongocrypt_buffer_t file_bufs[TEST_DATA_COUNT];

    int bson_count;
    bson_t test_bson[TEST_DATA_COUNT];

    int bin_count;
    mongocrypt_binary_t *test_bin[TEST_DATA_COUNT];

    int blob_count;
    uint8_t *test_blob[TEST_DATA_COUNT];

    // Overides used in run_ctx_to
    struct {
        char *collection_info;
        char *mongocryptd_reply;
        char *key_file;
    } paths;

    /* Example encrypted doc. */
    _mongocrypt_buffer_t encrypted_doc;
} _mongocrypt_tester_t;

// `_mongocrypt_tester_t` inherits extended alignment from libbson. To dynamically allocate, use aligned allocation
// (e.g. BSON_ALIGNED_ALLOC)
BSON_STATIC_ASSERT2(alignof__mongocrypt_tester_t, BSON_ALIGNOF(_mongocrypt_tester_t) >= BSON_ALIGNOF(bson_t));

/* Load a .json file as bson */
void _load_json_as_bson(const char *path, bson_t *out);

void _mongocrypt_tester_satisfy_kms(_mongocrypt_tester_t *tester, mongocrypt_kms_ctx_t *kms);

void _mongocrypt_tester_run_ctx_to(_mongocrypt_tester_t *tester,
                                   mongocrypt_ctx_t *ctx,
                                   mongocrypt_ctx_state_t stop_state);

mongocrypt_binary_t *_mongocrypt_tester_encrypted_doc(_mongocrypt_tester_t *tester);

/* Return a repeated character with no null terminator. */
char *_mongocrypt_repeat_char(char c, uint32_t times);

void _mongocrypt_tester_fill_buffer(_mongocrypt_buffer_t *buf, int n);

/* Return a new initialized mongocrypt_t for testing. */
mongocrypt_t *_mongocrypt_tester_mongocrypt(tester_mongocrypt_flags options);

/* Initialize a new mongocrypt_t for use in testing. */
bool _mongocrypt_init_for_test(mongocrypt_t *);

typedef enum { CRYPTO_REQUIRED, CRYPTO_OPTIONAL, CRYPTO_PROHIBITED } _mongocrypt_tester_crypto_spec_t;

void _mongocrypt_tester_install(_mongocrypt_tester_t *tester,
                                char *name,
                                _mongocrypt_test_fn fn,
                                _mongocrypt_tester_crypto_spec_t crypto_spec);

const char *_mongocrypt_tester_plaintext(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_crypto(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_log(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_data_key(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_ctx(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_ctx_encrypt(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_ctx_decrypt(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_ctx_rewrap_many_datakey(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_ciphertext(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_key_broker(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_local_kms(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_cache(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_buffer(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_ctx_setopt(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_key(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_marking(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_traverse_util(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_crypto_hooks(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_key_cache(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_kms_responses(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_status(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_csfle_lib(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_dll(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_endpoint(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_kek(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_cache_oauth(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_kms_ctx(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_mc_tokens(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_fle2_payloads(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_fle2_iev_v2_payloads(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_efc(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_cleanup(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_compact(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_fle2_encryption_placeholder(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_fle2_payload_uev(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_fle2_payload_uev_v2(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_fle2_payload_iup(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_fle2_payload_iup_v2(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_fle2_payload_find_equality_v2(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_fle2_payload_find_range_v2(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_fle2_payload_find_text(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_fle2_tag_and_encrypted_metadata_block(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_gcp_auth(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_range_encoding(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_range_edge_generation(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_range_mincover(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_mc_RangeOpts(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_mc_TextOpts(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_mc_FLE2RangeFindDriverSpec(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_mc_reader(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_mc_writer(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_opts(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_named_kms_providers(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_mc_cmp(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_text_search_str_encode(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_unicode_fold(_mongocrypt_tester_t *tester);

void _mongocrypt_tester_install_mc_schema_broker(_mongocrypt_tester_t *tester);

/* Conveniences for getting test data. */

/* Get a temporary bson_t from a JSON string. Do not free it. */
bson_t *_mongocrypt_tester_bson_from_str(_mongocrypt_tester_t *tester, const char *json);
#define TMP_BSON_STR(...) _mongocrypt_tester_bson_from_str(tester, __VA_ARGS__)

/* Get a temporary bson_t from a formattable JSON string. Do not free it. */
MLIB_ANNOTATE_PRINTF(2, 3)
bson_t *_mongocrypt_tester_bson_from_json(_mongocrypt_tester_t *tester, const char *json, ...);
#define TMP_BSON(...) _mongocrypt_tester_bson_from_json(tester, __VA_ARGS__)

// TMP_BSONF builds a temporary bson_t from an extended JSON string. Do not free it.
// Useful with BSON_STR to write JSON in-place.
// Supports tokens MC_BSON and MC_STR.
//
// Examples:
// bson_t *b = TMP_BSONF(BSON_STR({"foo": MC_STR}), "bar"); // { "foo" : "bar" }
// bson_t *b2 = TMP_BSONF(BSON_STR({"buzz": MC_BSON }), b); // { "buzz": { "foo": "bar" }}
bson_t *tmp_bsonf(_mongocrypt_tester_t *tester, const char *fmt, ...);
#define TMP_BSONF(...) tmp_bsonf(tester, __VA_ARGS__)

/* Get a temporary bson_t from a JSON file. Do not free it. */
bson_t *_mongocrypt_tester_file_as_bson(_mongocrypt_tester_t *tester, const char *path);
#define TEST_FILE_AS_BSON(path) _mongocrypt_tester_file_as_bson(tester, path)

/* Get a temporary binary from a JSON string. Do not free it. */
mongocrypt_binary_t *_mongocrypt_tester_bin_from_str(_mongocrypt_tester_t *tester, const char *json);
#define TEST_BSON_STR(...) _mongocrypt_tester_bin_from_str(tester, __VA_ARGS__)

/* Get a temporary binary from a formattable JSON string. Do not free it. */
MLIB_ANNOTATE_PRINTF(2, 3)
mongocrypt_binary_t *_mongocrypt_tester_bin_from_json(_mongocrypt_tester_t *tester, const char *json, ...);
#define TEST_BSON(...) _mongocrypt_tester_bin_from_json(tester, __VA_ARGS__)

/* Return a binary blob with the repeating sequence of 123. Do not free it. */
mongocrypt_binary_t *_mongocrypt_tester_bin(_mongocrypt_tester_t *tester, int size);
#define TEST_BIN(size) _mongocrypt_tester_bin(tester, size)

/* Return either a .json file as BSON or a .txt file as characters. Do not free
 * it. */
mongocrypt_binary_t *_mongocrypt_tester_file(_mongocrypt_tester_t *tester, const char *path);
#define TEST_FILE(path) _mongocrypt_tester_file(tester, path)

#define INSTALL_TEST(fn) _mongocrypt_tester_install(tester, #fn, fn, CRYPTO_REQUIRED)
#define INSTALL_TEST_CRYPTO(fn, crypto) _mongocrypt_tester_install(tester, #fn, fn, crypto)

void _load_json_as_bson(const char *path, bson_t *out);

extern bool _aes_ctr_is_supported_by_os;

void _test_ctx_wrap_and_feed_key(mongocrypt_ctx_t *ctx,
                                 const _mongocrypt_buffer_t *id,
                                 _mongocrypt_buffer_t *key,
                                 mongocrypt_status_t *status);

void _usleep(int64_t usec);

#endif
