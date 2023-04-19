#pragma once

#include "ex_yyjson.h"

yyjson_mut_val *encode_term(ErlNifEnv *env, ERL_NIF_TERM term, yyjson_mut_doc *doc);
yyjson_mut_val *encode_binary(ErlNifEnv *env, ERL_NIF_TERM term, yyjson_mut_doc *doc);
yyjson_mut_val *encode_map(ErlNifEnv *env, ERL_NIF_TERM term, yyjson_mut_doc *doc);
yyjson_mut_val *encode_list(ErlNifEnv *env, ERL_NIF_TERM term, yyjson_mut_doc *doc);
yyjson_mut_val *encode_integer(ErlNifEnv *env, ERL_NIF_TERM term, yyjson_mut_doc *doc);
yyjson_mut_val *encode_float(ErlNifEnv *env, ERL_NIF_TERM term, yyjson_mut_doc *doc);
yyjson_mut_val *encode_atom_as_str(ErlNifEnv *env, ERL_NIF_TERM term, yyjson_mut_doc *doc);
yyjson_mut_val *encode_atom(ErlNifEnv *env, ERL_NIF_TERM term, yyjson_mut_doc *doc);