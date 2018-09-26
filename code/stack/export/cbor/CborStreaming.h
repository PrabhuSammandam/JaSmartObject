#pragma once

#include "CborTypedefs.h"
#include "CborListener.h"

/** Stateless decoder
 *
 * Will try parsing the \p buffer and will invoke the appropriate callback on success.
 * Decodes one item at a time. No memory allocations occur.
 *
 * @param buffer Input buffer
 * @param buffer_size Length of the buffer
 * @param callbacks The callback bundle
 * @param context An arbitrary pointer to allow for maintaining context.
 */
cbor_decoder_result cbor_stream_decode(uint8_t* buffer, size_t buffer_size, CborListener& listener);