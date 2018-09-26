/*
 * IMemoryAllocator.h
 *
 *  Created on: Jul 25, 2017
 *      Author: psammand
 */

#pragma once

#include <cstdint>
#include <cstddef>

namespace ja_iot
{
  namespace memory
  {
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

      virtual ~IMemAllocator()
      {
      }

      virtual void* alloc(size_t mem_size) = 0;
      virtual void free(void* p_memory) = 0;

      void* alloc(const char* file_name, const uint32_t line_no, size_t mem_size) const;
      void free(const char* file_name, const uint32_t line_no, void* p_memory) const;
    };

    class MemStamp
    {
    public:
      char const*const _fileName;
      int const _lineNum;
      const int line_no_;

      MemStamp(char const* filename, const int lineNo) : _fileName(filename), _lineNum(lineNo), line_no_{lineNo}
      {
      }

      ~MemStamp()
      {
      }
    };

    class MemAllocatorFactory
    {
    public:

      MemAllocatorFactory();

      ~MemAllocatorFactory();

      static IMemAllocator* create_mem_allocator(MemAlloctorType mem_allocator_type);
      static IMemAllocator& get() { return *cur_mem_allocator_factory_; }
      static void set(IMemAllocator* p_mem_allocator_factory) { cur_mem_allocator_factory_ = p_mem_allocator_factory; }

    private:
      static IMemAllocator* cur_mem_allocator_factory_;
    };


    template <class T>
    T* operator +(const MemStamp& memStamp, T* p)
    {
#ifdef JAIOT_ENABLE_MEM_DEBUG
      printf("P[%p] F[%-20s] L[%-6u]\n", p, memStamp._fileName, memStamp._lineNum);
#endif
      return (p);
    }

    template <class T>
    void operator -(const MemStamp& memStamp, T* p)
    {
      delete p;
#ifdef JAIOT_ENABLE_MEM_DEBUG
      printf("P[%p] F[%-20s] L[%-6u]\n", p, memStamp._fileName, memStamp._lineNum);
#endif
    }

#ifdef JAIOT_ENABLE_MEM_DEBUG
#define mnew ja_iot::memory::MemStamp( _jaiot_file_name, __LINE__ ) + new
#define mdelete ja_iot::memory::MemStamp( _jaiot_file_name, __LINE__ ) -

#define mnew_g( __size__ ) do { return ( ja_iot::memory::MemAllocatorFactory::get().alloc( _jaiot_file_name, __LINE__, __size__ ) ); \
} while( 0 );

#define mdelete_g( __mem_ptr__ ) do { return ( ja_iot::memory::MemAllocatorFactory::get().free( _jaiot_file_name, __LINE__, __mem_ptr__ ) ); \
} while( 0 );

#else
#define mnew new
#define mdelete delete

#define mnew_g( __size__ )  ja_iot::memory::MemAllocatorFactory::get().alloc( __size__ )

#define mdelete_g( __mem_ptr__ )  ja_iot::memory::MemAllocatorFactory::get().free( __mem_ptr__ )

#endif /* JAIOT_ENABLE_MEM_DEBUG */

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
