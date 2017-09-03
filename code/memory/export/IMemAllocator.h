/*
 * IMemoryAllocator.h
 *
 *  Created on: Jul 25, 2017
 *      Author: psammand
 */

#ifndef IMEMALLOCATOR_H_
#define IMEMALLOCATOR_H_

#include <stdio.h>
#include <cstdint>

namespace ja_iot {
namespace memory {
// #define JAIOT_ENABLE_MEM_DEBUG
#define JAIOT_MEM_FILE_NAME( x ) static const char *const _jaiot_file_name = _x_;

enum class MemAlloctorType
{
  kWindows,
  kLinux,
  kFreeRTOS
};

class IMemAllocator
{
  public:

    virtual ~IMemAllocator () {}
    virtual void* alloc( size_t mem_size ) = 0;
    virtual void  free( void *p_memory )   = 0;

    void* alloc( const char *file_name, const uint32_t line_no, size_t mem_size );
    void  free( const char *file_name, const uint32_t line_no, void *p_memory );
};

class MemAllocatorFactory
{
  public:

    MemAllocatorFactory ();

    ~MemAllocatorFactory ();

  public:
    static IMemAllocator* get( MemAlloctorType mem_allocator_type );
    static IMemAllocator& get() { return ( *cur_mem_allocator_factory_ ); }
    static void         set( IMemAllocator *p_mem_allocator_factory ) { cur_mem_allocator_factory_ = p_mem_allocator_factory; }

  private:
    static IMemAllocator * cur_mem_allocator_factory_;
};

#ifdef JAIOT_ENABLE_MEM_DEBUG
#define jaiot_mem_alloc( __size__ ) do { return ( ja_iot::memory::MemAllocatorFactory::get().alloc( _jaiot_file_name, __LINE__, __size__ ) ); \
} while( 0 );

#define jaiot_mem_free( __mem_ptr__ ) do { return ( ja_iot::memory::MemAllocatorFactory::get().free( _jaiot_file_name, __LINE__, __mem_ptr__ ) ); \
} while( 0 );
#else
#define jaiot_mem_alloc( __size__ ) do { return ( ja_iot::memory::MemAllocatorFactory::get().alloc( __size__ ) ); \
} while( 0 );

#define jaiot_mem_free( __mem_ptr__ ) do { return ( ja_iot::memory::MemAllocatorFactory::get().free( __mem_ptr__ ) ); \
} while( 0 );
#endif


#define DECLARE_NEW_OPERATOR void *operator   new ( size_t size );
#define DECLARE_DELETE_OPERATOR void operator delete ( void *ptr );

#define DECLARE_MEMORY_OPERATORS DECLARE_NEW_OPERATOR DECLARE_DELETE_OPERATOR

#define DEFINE_NEW_OPERATOR( __x__ ) void *operator new ( size_t size ) { jaiot_mem_alloc( size ); }
#define DEFINE_DELETE_OPERATOR( __x__ ) void operator delete ( void *ptr ) { jaiot_mem_free( ptr ); }

#define DEFINE_MEMORY_OPERATORS( __x__ ) DEFINE_NEW_OPERATOR( __x__ ) DEFINE_DELETE_OPERATOR( __x__ )
}
}

#endif /* IMEMALLOCATOR_H_ */