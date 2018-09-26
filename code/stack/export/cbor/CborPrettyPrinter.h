#pragma once

#if CBOR_PRETTY_PRINTER
#include "CborItem.h"
#include <stdio.h>

void cbor_describe( CborItem *item, FILE *out );
#endif