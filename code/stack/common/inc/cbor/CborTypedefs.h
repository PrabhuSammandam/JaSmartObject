#pragma once

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <cstdint>
#include <string.h>

typedef const uint8_t *cbor_data;
typedef uint8_t *cbor_mutable_data;

/** Specifies the Major type of ::cbor_item_t */
enum cbor_type : uint8_t
{
  CBOR_TYPE_UINT /** 0 - positive integers */,
  CBOR_TYPE_NEGINT /** 1 - negative integers*/,
  CBOR_TYPE_BYTESTRING /** 2 - byte strings */,
  CBOR_TYPE_STRING /** 3 - strings */,
  CBOR_TYPE_ARRAY /** 4 - arrays */,
  CBOR_TYPE_MAP /** 5 - maps */,
  CBOR_TYPE_TAG /** 6 - tags  */,
  CBOR_TYPE_FLOAT_CTRL         /** 7 - decimals and special values (true, false, nil, ...) */
};

/** Possible decoding errors */
enum cbor_error_code : uint8_t
{
  CBOR_ERR_NONE,
  CBOR_ERR_NOTENOUGHDATA,
  CBOR_ERR_NODATA,
  CBOR_ERR_MALFORMATED,
  CBOR_ERR_MEMERROR /** Memory error - item allocation failed. Is it too big for your allocator? */,
  CBOR_ERR_SYNTAXERROR          /** Stack parsing algorithm failed */
};

/** Possible widths of #CBOR_TYPE_UINT items */
enum cbor_int_width : uint8_t
{
  CBOR_INT_8  = 1,
  CBOR_INT_16 = 2,
  CBOR_INT_32 = 4,
  CBOR_INT_64 = 8
};

/** Possible widths of #CBOR_TYPE_FLOAT_CTRL items */
enum cbor_float_width : uint8_t
{
  CBOR_FLOAT_0  = 0 /** Internal use - ctrl and special values */,
  CBOR_FLOAT_16 = 2 /** Half float */,
  CBOR_FLOAT_32 = 4 /** Single float */,
  CBOR_FLOAT_64 = 8            /** Double */
};

/** Semantic mapping for CTRL simple values */
enum _cbor_ctrl : uint8_t
{
  CBOR_CTRL_NONE  = 0,
  CBOR_CTRL_FALSE = 20,
  CBOR_CTRL_TRUE  = 21,
  CBOR_CTRL_NULL  = 22,
  CBOR_CTRL_UNDEF = 23
};

/** Raw memory casts helper */
union _cbor_float_helper
{
  float    as_float;
  uint32_t as_uint;
};

/** Raw memory casts helper */
union _cbor_double_helper
{
  double   as_double;
  uint64_t as_uint;
};

/** The item handle */

/** High-level decoding error */
struct cbor_error
{
  /** Aproximate position */
  size_t   position;
  /** Description */
  cbor_error_code   code;
};

/** High-level decoding result */
struct cbor_load_result
{
  /** Error indicator */
  struct cbor_error   error;
  /** Number of bytes read*/
  size_t   read;
};


/** Streaming decoder result - status */
enum cbor_decoder_status
{
  CBOR_DECODER_FINISHED /** OK, finished */,
  CBOR_DECODER_NEDATA /** Not enough data - mismatch with MTB */,
  CBOR_DECODER_EBUFFER /** Buffer manipulation problem */,
  CBOR_DECODER_ERROR                            /** Malformed or reserved MTB/value */
};

/** Streaming decoder result */
struct cbor_decoder_result
{
  /** Bytes read */
  size_t   read;
  /** The result */
  enum cbor_decoder_status   status;
};