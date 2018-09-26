/*
 * cplusplus.cpp
 *
 *  Created on: May 24, 2017
 *      Author: psammand
 */

#include <cstddef>
#include <c_types.h>

#include "esp_libc.h"

#define LOOP { while( 1 ){} }

namespace std {
void __throw_length_error( const char * ) LOOP
void __throw_length_error( char *const str ) LOOP
void __throw_bad_function_call() LOOP
void __throw_logic_error( const char * ) LOOP
void __throw_bad_alloc() LOOP
void __throw_out_of_range_fmt( const char *, ... ) LOOP
void __throw_bad_cast( void ) LOOP
void __throw_out_of_range( const char * ) LOOP
void __throw_runtime_error( const char * ) LOOP
void __throw_ios_failure( const char * ) LOOP
}

void * ICACHE_FLASH_ATTR operator new ( size_t size )
{
  return ( os_malloc( size ) );
}

void * ICACHE_FLASH_ATTR operator new[] ( size_t size )
{
  return ( os_malloc( size ) );
}

void ICACHE_FLASH_ATTR operator delete ( void *ptr )
{
  os_free( ptr );
}

void ICACHE_FLASH_ATTR operator delete ( void *ptr, unsigned int size )
{
  os_free( ptr );
}

void ICACHE_FLASH_ATTR operator delete[] ( void *ptr )
{
  os_free( ptr );
}

extern "C" void ICACHE_FLASH_ATTR __cxa_pure_virtual( void )
{
}

extern "C" int atexit( void ( *function )( void ) )
{
  return ( 0 );
}