//
// Define some Macros & tools to integral duktools more easy
//
#ifndef WALLY_DUKTOOLS

#define WALLY_DUKTOOLS

// To setup the object :
// duk_ret_t js_newObject(duk_context *ctx)
// {
//    duk_idx_t obj_idx = duk_push_object(ctx);
//#define DUK_PUSH_PROP_INT(a,b) duk_push_c_function(ctx,js_setter,1); duk_push_int(ctx, b); duk_put_prop_string(ctx,obj_idx, a); duk_push_c_function(ctx,js_getter,0); duk_push_int(ctx, b); duk_put_prop_string(ctx,obj_idx, a);
#define DUK_PUSH_PROP_INT(a,b) duk_push_int(ctx, b); duk_put_prop_string(ctx, obj_idx, a);
#define DUK_PUSH_PROP_UINT(a,b) duk_push_uint(ctx, b); duk_put_prop_string(ctx, obj_idx, a);
#define DUK_PUSH_PROP_BOOL(a,b) duk_push_boolean(ctx, b); duk_put_prop_string(ctx, obj_idx, a);
#define DUK_PUSH_PROP_STRING(a,b) duk_push_string(ctx, b);duk_put_prop_string(ctx, obj_idx, a);
#define DUK_PUSH_PROP_POINTER(a,b) duk_push_pointer(ctx, b);duk_put_prop_string(ctx, obj_idx, a);
#define DUK_PUSH_PROP_FLOAT(a,b) duk_push_number(ctx, b);duk_put_prop_string(ctx, obj_idx, a);
#define DUK_PUSH_PROP_DOUBLE(a,b) duk_push_number(ctx, b);duk_put_prop_string(ctx, obj_idx, a);

#endif
