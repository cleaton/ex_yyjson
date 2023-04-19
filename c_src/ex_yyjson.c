#include "ex_yyjson.h"
#include "encode_term.h"

// TODO: encode/decode functions are not yielding
// so for large json objects they might block the scheduler for more than the recommended ~1ms limit.
// An updated alternative could pass bytes processed to each recursive encode/decode call and yield after x bytes or time.
// jiffy uses this approach.

struct consts CONSTS;

inline static result_t
ok(ERL_NIF_TERM term)
{
    return (result_t){false, term};
}
inline static result_t err(ERL_NIF_TERM term)
{
    return (result_t){true, term};
}

static int ex_yyjson_init(ErlNifEnv *env, void **priv_data, ERL_NIF_TERM load_info)
{
    UNUSED(priv_data);
    UNUSED(load_info);
    // INIT atoms
    CONSTS.atom_ok = enif_make_atom(env, "ok");
    CONSTS.atom_error = enif_make_atom(env, "error");
    CONSTS.atom_out_of_memory = enif_make_atom(env, "out_of_memory");
    CONSTS.atom_invalid_json = enif_make_atom(env, "invalid_json");
    CONSTS.atom_invalid_type = enif_make_atom(env, "invalid_type");
    CONSTS.atom_nil = enif_make_atom(env, "nil");
    CONSTS.atom_null = enif_make_atom(env, "null");
    CONSTS.atom_true = enif_make_atom(env, "true");
    CONSTS.atom_false = enif_make_atom(env, "false");
    CONSTS.atom_duplicate_object_keys = enif_make_atom(env, "duplicate_object_keys");

    // Create a resource type fnor yyjson_doc
    // yyjson_doc_type = enif_open_resource_type(env, NULL, "yyjson_doc", yyjson_doc_dtor,
    //                                          ERL_NIF_RT_CREATE | ERL_NIF_RT_TAKEOVER,
    //                                          NULL);
    // if (!yyjson_doc_type) {
    //  return -1; // Initialization failed
    //}
    return 0; // Initialization succeeded
}

static result_t str_to_binary(ErlNifEnv *env, const char *str, size_t len)
{

    if (unlikely(!str))
    {
        return err(CONSTS.atom_invalid_json);
    }

    ERL_NIF_TERM bin_term;
    unsigned char *bin_data = enif_make_new_binary(env, len, &bin_term);
    if (unlikely(!bin_data))
    {
        return err(CONSTS.atom_out_of_memory);
    }
    memcpy(bin_data, str, len);

    return ok(bin_term);
}

static result_t object_to_term(ErlNifEnv *env, yyjson_val *obj)
{
    size_t idx, max, key_len;
    yyjson_val *key, *val;
    const char *key_str;
    max = yyjson_obj_size(obj);
    ERL_NIF_TERM *keys = enif_alloc(sizeof(ERL_NIF_TERM) * max);
    if (unlikely(!keys))
    {
        return err(CONSTS.atom_out_of_memory);
    }
    ERL_NIF_TERM *values = enif_alloc(sizeof(ERL_NIF_TERM) * max);
    if (unlikely(!values))
    {
        return err(CONSTS.atom_out_of_memory);
    }
    yyjson_obj_foreach(obj, idx, max, key, val)
    {
        key_str = yyjson_get_str(key);
        key_len = yyjson_get_len(key);
        if (unlikely(!key_str))
        {
            return err(CONSTS.atom_invalid_json);
        }

        const result_t kr = str_to_binary(env, key_str, key_len);
        if (unlikely(kr.is_error))
        {
            enif_free(keys);
            enif_free(values);
            return kr;
        }
        const result_t vr = val_to_term(env, val);
        if (unlikely(vr.is_error))
        {
            enif_free(keys);
            enif_free(values);
            return vr;
        }
        keys[idx] = kr.term;
        values[idx] = vr.term;
    }
    ERL_NIF_TERM map;
    int success = enif_make_map_from_arrays(env, keys, values, max, &map);
    enif_free(keys);
    enif_free(values);
    if (unlikely(!success))
    {
        return err(CONSTS.atom_duplicate_object_keys);
    }
    return ok(map);
}

static result_t array_to_term(ErlNifEnv *env, yyjson_val *val)
{
    size_t idx, max;
    max = yyjson_arr_size(val);
    ERL_NIF_TERM *arr = enif_alloc(sizeof(ERL_NIF_TERM) * max);
    if (unlikely(!arr))
    {
        return err(CONSTS.atom_out_of_memory);
    }
    yyjson_arr_foreach(val, idx, max, val)
    {
        const result_t vr = val_to_term(env, val);
        if (unlikely(vr.is_error))
        {
            enif_free(arr);
            return vr;
        }
        arr[idx] = vr.term;
    }
    ERL_NIF_TERM list = enif_make_list_from_array(env, arr, max);
    enif_free(arr);
    return ok(list);
}

static result_t val_to_term(ErlNifEnv *env, yyjson_val *val)
{
    const char *str = NULL;
    size_t len = 0;
    if (unlikely(!val))
    {
        return err(CONSTS.atom_invalid_json);
    }

    switch (yyjson_get_tag(val))
    {
    case YYJSON_TYPE_RAW | YYJSON_SUBTYPE_NONE:
        str = yyjson_get_raw(val);
        len = yyjson_get_len(val);
        return str_to_binary(env, str, len);
    case YYJSON_TYPE_NULL | YYJSON_SUBTYPE_NONE:
        return ok(CONSTS.atom_nil);
    case YYJSON_TYPE_STR | YYJSON_SUBTYPE_NONE:
        str = yyjson_get_str(val);
        len = yyjson_get_len(val);
        return str_to_binary(env, str, len);
    case YYJSON_TYPE_ARR | YYJSON_SUBTYPE_NONE:
        return array_to_term(env, val);
    case YYJSON_TYPE_OBJ | YYJSON_SUBTYPE_NONE:
        return object_to_term(env, val);
    case YYJSON_TYPE_BOOL | YYJSON_SUBTYPE_TRUE:
        return ok(CONSTS.atom_true);
    case YYJSON_TYPE_BOOL | YYJSON_SUBTYPE_FALSE:
        return ok(CONSTS.atom_false);
    case YYJSON_TYPE_NUM | YYJSON_SUBTYPE_UINT:
        return ok(enif_make_uint64(env, yyjson_get_uint(val)));
    case YYJSON_TYPE_NUM | YYJSON_SUBTYPE_SINT:
        return ok(enif_make_int64(env, yyjson_get_int(val)));
    case YYJSON_TYPE_NUM | YYJSON_SUBTYPE_REAL:
        return ok(enif_make_double(env, yyjson_get_real(val)));
    default:
        return err(CONSTS.atom_invalid_type);
    }
}



static ERL_NIF_TERM encode(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    // Check the argument count and type
    if (argc != 1)
    {
        return enif_make_badarg(env);
    }

    // Convert the term to a JSON document
    // Tiny allocations can be quite heavy on the default allocator.
    // Consider using a custom area allocator, ex jemalloc for better multicore performance.
    yyjson_doc *doc = yyjson_mut_doc_new(NULL);
    yyjson_mut_val *root = encode_term(env, argv[0], doc);
    if (unlikely(!root))
    {
        return CONSTS.atom_error;
    }
    yyjson_mut_doc_set_root(doc, root);

    // Write the JSON document to a string
    yyjson_write_flag flg = 0;
    size_t json_len = 0;
    char *json_str = yyjson_mut_write(doc, flg, &json_len);
    yyjson_mut_doc_free(doc);
    if (unlikely(!json_str))
    {
        // TODO better error handling
        return CONSTS.atom_out_of_memory;
    }

    // Convert the string to a binary
    ERL_NIF_TERM json_bin_term;
    unsigned char * bin = enif_make_new_binary(env, json_len, &json_bin_term);
    memcpy(bin, json_str, json_len);
    free(json_str);
    return json_bin_term;
}

// Define the NIF functions
// static ERL_NIF_TERM encode(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[]);
static ERL_NIF_TERM decode(ErlNifEnv *env, int argc, const ERL_NIF_TERM argv[])
{
    // Check the argument count and type
    if (argc != 1 || !enif_is_binary(env, argv[0]))
    {
        return enif_make_badarg(env);
    }

    // Get the JSON string and length from the binary term
    ErlNifBinary bin;
    enif_inspect_binary(env, argv[0], &bin);

    // Read the JSON string to an immutable document with validation enabled
    yyjson_read_flag flg = 0;
    yyjson_doc *doc = yyjson_read((const char *)bin.data, bin.size, flg);

    // Check the read result
    if (!doc)
    {
        return enif_make_tuple2(env, CONSTS.atom_error, CONSTS.atom_invalid_json);
    }

    // Get the root value of the document
    yyjson_val *val = yyjson_doc_get_root(doc);

    const result_t res = val_to_term(env, val);

    // Create a resource term to hold the document
    // ERL_NIF_TERM res = enif_make_resource(env, doc);

    // Release the document resource
    //enif_release_resource(doc);
    yyjson_doc_free(doc);

    if (unlikely(res.is_error))
    {
        return enif_make_tuple2(env, CONSTS.atom_error, res.term);
    }
    return enif_make_tuple2(env, CONSTS.atom_ok, res.term);
}

// Define the NIF mapping
static ErlNifFunc nif_funcs[] = {
    {"encode", 1, encode, 0},
    {"decode", 1, decode, 0}};

// Initialize the NIF library with the nif_init function
ERL_NIF_INIT(Elixir.ExYyjson, nif_funcs, ex_yyjson_init, NULL, NULL, NULL)