#include "encode_term.h"

yyjson_mut_val *encode_term(ErlNifEnv *env, ERL_NIF_TERM term, yyjson_mut_doc *doc)
{
    ErlNifTermType term_type = enif_term_type(env, term);

    switch (term_type)
    {
    case ERL_NIF_TERM_TYPE_ATOM:
        return encode_atom(env, term, doc);
    case ERL_NIF_TERM_TYPE_BITSTRING:
        return encode_binary(env, term, doc);
    case ERL_NIF_TERM_TYPE_FLOAT:
        return encode_float(env, term, doc);
    case ERL_NIF_TERM_TYPE_FUN:
        enif_raise_exception(env, enif_make_string(env, "Fun not supported", ERL_NIF_LATIN1));
        return NULL;
    case ERL_NIF_TERM_TYPE_INTEGER:
        return encode_integer(env, term, doc);
    case ERL_NIF_TERM_TYPE_LIST:
        return encode_list(env, term, doc);
    case ERL_NIF_TERM_TYPE_MAP:
        return encode_map(env, term, doc);
    case ERL_NIF_TERM_TYPE_PID:
        enif_raise_exception(env, enif_make_string(env, "Pid not supported", ERL_NIF_LATIN1));
        return NULL;
    case ERL_NIF_TERM_TYPE_PORT:
        enif_raise_exception(env, enif_make_string(env, "Port not supported", ERL_NIF_LATIN1));
        return NULL;
    case ERL_NIF_TERM_TYPE_REFERENCE:
        enif_raise_exception(env, enif_make_string(env, "Reference not supported", ERL_NIF_LATIN1));
        return NULL;
    case ERL_NIF_TERM_TYPE_TUPLE:
        enif_raise_exception(env, enif_make_string(env, "Tuple not supported", ERL_NIF_LATIN1));
        return NULL;
    default:
        enif_raise_exception(env, enif_make_string(env, "Uknown term type", ERL_NIF_LATIN1));
        return NULL;
    }
}

yyjson_mut_val *encode_binary(ErlNifEnv *env, ERL_NIF_TERM term, yyjson_mut_doc *doc)
{
    ErlNifBinary bin;
    if (!enif_inspect_binary(env, term, &bin))
    {
        enif_raise_exception(env, enif_make_string(env, "Failed to inspect binary", ERL_NIF_LATIN1));
        return NULL;
    }
    yyjson_mut_val *val = yyjson_mut_strncpy(doc, (const char *)bin.data, bin.size);
    if (val == NULL)
    {
        enif_raise_exception(env, enif_make_string(env, "Failed to create string", ERL_NIF_LATIN1));
        return NULL;
    }
    return val;
}

yyjson_mut_val *encode_map(ErlNifEnv *env, ERL_NIF_TERM term, yyjson_mut_doc *doc)
{
    yyjson_mut_val *obj = yyjson_mut_obj(doc);
    if (obj == NULL)
    {
        enif_raise_exception(env, enif_make_string(env, "Failed to create object", ERL_NIF_LATIN1));
        return NULL;
    }

    ERL_NIF_TERM key, value;
    ErlNifMapIterator iter;
    enif_map_iterator_create(env, term, &iter, ERL_NIF_MAP_ITERATOR_FIRST);

    while (enif_map_iterator_get_pair(env, &iter, &key, &value)) {

        yyjson_mut_val *key_val = NULL;
        if (enif_is_binary(env, key))
        {
            key_val = encode_binary(env, key, doc);
        } else if (enif_is_atom(env, key)) {
            key_val = encode_atom_as_str(env, key, doc);
        } else {
            enif_map_iterator_destroy(env, &iter);
            enif_raise_exception(env, enif_make_string(env, "Key is not a binary or atom", ERL_NIF_LATIN1));
            return NULL;
        }
        



        yyjson_mut_val *val = encode_term(env, value, doc);
        if (val == NULL)
        {
            enif_map_iterator_destroy(env, &iter);
            return NULL;
        }

        if (!yyjson_mut_obj_add(obj, key_val, val))
        {
            enif_map_iterator_destroy(env, &iter);
            enif_raise_exception(env, enif_make_string(env, "Failed to set key", ERL_NIF_LATIN1));
            return NULL;
        }
        enif_map_iterator_next(env, &iter);
    }

    return obj;
}

yyjson_mut_val *encode_list(ErlNifEnv *env, ERL_NIF_TERM term, yyjson_mut_doc *doc)
{
    yyjson_mut_val *arr = yyjson_mut_arr(doc);
    if (arr == NULL)
    {
        enif_raise_exception(env, enif_make_string(env, "Failed to create array", ERL_NIF_LATIN1));
        return NULL;
    }

    ERL_NIF_TERM head, tail;
    while (enif_get_list_cell(env, term, &head, &tail))
    {
        yyjson_mut_val *val = encode_term(env, head, doc);
        if (val == NULL)
        {
            // no need to free here, as memory is managed by the document
            return NULL;
        }
        bool append = yyjson_mut_arr_append(arr, val);
        if(unlikely(!append))
        {
            enif_raise_exception(env, enif_make_string(env, "Failed to append to array", ERL_NIF_LATIN1));
            return NULL;
        }
        term = tail;
    }

    return arr;
}

yyjson_mut_val *encode_integer(ErlNifEnv *env, ERL_NIF_TERM term, yyjson_mut_doc *doc)
{
    ErlNifSInt64 int_val;
    ErlNifUInt64 uint_val;

    if (enif_get_int64(env, term, &int_val))
    {
        return yyjson_mut_sint(doc, int_val);
    }
    else if (enif_get_uint64(env, term, &uint_val))
    {
        return yyjson_mut_uint(doc, uint_val);
    }
    else
    {
        enif_raise_exception(env, enif_make_string(env, "Failed to get integer value", ERL_NIF_LATIN1));
        return NULL;
    }
}

yyjson_mut_val *encode_float(ErlNifEnv *env, ERL_NIF_TERM term, yyjson_mut_doc *doc)
{
    double value;
    if (enif_get_double(env, term, &value))
    {
        return yyjson_mut_real(doc, value);
    }
    else
    {
        enif_raise_exception(env, enif_make_string(env, "Failed to get double value", ERL_NIF_LATIN1));
        return NULL;
    }
}

size_t latin1_to_utf8(const char *latin1, size_t latin1_len, char *utf8, size_t utf8_max_size) {
    size_t utf8_len = 0;
    unsigned char c;

    for (size_t i = 0; i < latin1_len && (c = latin1[i]) != '\0'; i++) {
        size_t char_size = (c < 0x80) ? 1 : 2;

        if (utf8_len + char_size >= utf8_max_size) {
            return 0;
        }

        if (char_size == 1) {
            *utf8++ = c;
        } else {
            *utf8++ = 0xC0 | (c >> 6);
            *utf8++ = 0x80 | (c & 0x3F);
        }
        
        utf8_len += char_size;
    }

    if (utf8_len + 1 > utf8_max_size) {
        return 0;
    }

    *utf8 = '\0';
    return utf8_len;
}

yyjson_mut_val *encode_atom_as_str(ErlNifEnv *env, ERL_NIF_TERM term, yyjson_mut_doc *doc)
{
    #define MAX_ATOM_LENGTH 256
    #define MAX_UTF8_LENGTH 512
    char atom_str[MAX_ATOM_LENGTH];
    char utf8_str[MAX_UTF8_LENGTH];
    size_t atom_length = enif_get_atom(env, term, atom_str, MAX_ATOM_LENGTH, ERL_NIF_LATIN1);
    if (atom_length > 0) {
        size_t utf8_len = latin1_to_utf8(atom_str, atom_length, utf8_str, MAX_UTF8_LENGTH);
        return yyjson_mut_strncpy(doc, utf8_str, utf8_len);
    } else {
        enif_raise_exception(env, enif_make_string(env, "Atom more than 256 chars", ERL_NIF_LATIN1));
        return NULL;
    }
}

yyjson_mut_val *encode_atom(ErlNifEnv *env, ERL_NIF_TERM term, yyjson_mut_doc *doc)
{
    if (enif_is_identical(term, CONSTS.atom_true))
    {
        return yyjson_mut_true(doc);
    }
    else if (enif_is_identical(term, CONSTS.atom_false))
    {
        return yyjson_mut_false(doc);
    }
    else if (enif_is_identical(term, CONSTS.atom_nil) || enif_is_identical(term, CONSTS.atom_null))
    {
        return yyjson_mut_null(doc);
    }
    return encode_atom_as_str(env, term, doc);
}