#pragma once

#include "yyjson/yyjson.h"
#include "erl_nif.h"

#ifdef __GNUC__
#define likely(x)       __builtin_expect(!!(x), 1)
#define unlikely(x)     __builtin_expect(!!(x), 0)
#else
#define likely(x)       (x)
#define unlikely(x)     (x)
#endif

#define UNUSED(x) (void)(x)

struct result_t
{
    bool is_error;
    ERL_NIF_TERM term;
};

typedef struct result_t result_t;

struct consts
{
    ERL_NIF_TERM atom_ok;
    ERL_NIF_TERM atom_error;
    ERL_NIF_TERM atom_out_of_memory;
    ERL_NIF_TERM atom_invalid_json;
    ERL_NIF_TERM atom_invalid_type;
    ERL_NIF_TERM atom_nil;
    ERL_NIF_TERM atom_true;
    ERL_NIF_TERM atom_false;
    ERL_NIF_TERM atom_duplicate_object_keys;
    ERL_NIF_TERM atom_null;
};


static result_t object_to_term(ErlNifEnv *env, yyjson_val *obj);
static result_t array_to_term(ErlNifEnv *env, yyjson_val *arr);
static result_t val_to_term(ErlNifEnv *env, yyjson_val *val);
static result_t str_to_binary(ErlNifEnv *env, const char *str, size_t len);
extern struct consts CONSTS;
