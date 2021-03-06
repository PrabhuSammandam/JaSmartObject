#include <options/coap_option.h>
#include <stdio.h>
#include <cstring>
#include <IMemAllocator.h>

namespace ja_iot {
namespace coap {
using namespace ja_iot::memory;

Option::Option( uint16_t no, uint16_t len, uint8_t *val )
{
  _next = nullptr;
  _no   = no;
  _len  = len;

  if( len > 0 )
  {
    _val = (uint8_t *) ( this ) + sizeof( Option );
    std::memcpy( _val, val, len );
  }
  else
  {
    _val = nullptr;
  }
}

Option::Option( Option const &opt ) :
  Option( opt._no, opt._len, opt._val )
{
}

Option::~Option ()
{
  _next = nullptr;
}

void * Option::operator new ( size_t len, uint16_t buf_len )
{
  return ( MemAllocatorFactory::get().alloc( ( sizeof( Option ) + buf_len ) ) );// ( JaCoapIF::MemAlloc( ( sizeof( Option ) + buf_len ) ) );
}

void Option::operator delete ( void *mem, uint16_t buf_len )
{
  MemAllocatorFactory::get().free( mem );
}

void Option::Print() const
{
  printf( "Number : %u\n", GetNo() );
  printf( "Length : %u\n", GetLen() );
  printf( "Value : " );

  for( size_t i = 0; i < GetLen(); i++ )
  {
    printf( "%c", _val[i] );
  }

  printf( "\n" );
}

void Option::Destroy( Option *opt )
{
  opt->~Option ();
  Option::operator delete ( opt, 0 );
}

Option * Option::Allocate( uint16_t no, uint16_t len, uint8_t *val )
{
  return ( new(len) Option{ no,
                            len,
                            val } );
}

Option * Option::Allocate()
{
  return ( new(0) Option{ 0,
                          0,
                          nullptr } );
}

/*************************OPTION_LIST*********************/
OptionList::OptionList ()
{
  this->_first = nullptr;
  this->_last  = nullptr;
}

OptionList::~OptionList ()
{
  FreeList();
}

void OptionList::InsertFront( Option *option )
{
  if( IsEmpty() )
  {
    _first = option;
    _last  = option;
  }
  else
  {
    option->SetNext( _first );
    _first = option;
  }
}

void OptionList::InsertEnd( Option *option )
{
  if( IsEmpty() )
  {
    _first = option;
    _last  = option;
  }
  else
  {
    _last->SetNext( option );
    _last = option;
  }
}

void OptionList::Insert( Option *option )
{
  if( IsEmpty() )
  {
    InsertFront( option );
  }
  else
  {
    if( option->GetNo() < _first->GetNo() )
    {
      InsertFront( option );
    }
    else if( option->GetNo() >= _last->GetNo() )
    {
      InsertEnd( option );
    }
    else
    {
      auto temp_opt = _first;

      while( temp_opt != _last )
      {
        if( ( option->GetNo() < temp_opt->GetNext()->GetNo() ) && ( option->GetNo() >= temp_opt->GetNo() ) )
        {
          auto temp_next = temp_opt->GetNext();
          temp_opt->SetNext( option );
          option->SetNext( temp_next );
          break;
        }

        temp_opt = temp_opt->GetNext();
      }
    }
  }
}

void OptionList::FreeList()
{
  if( !IsEmpty() )
  {
    Option *temp = _first->GetNext();

    while( temp != nullptr )
    {
      _first->SetNext( _first->GetNext()->GetNext() );
      Option::Destroy( temp );
      temp = _first->GetNext();
    }

    Option::Destroy( _first );
    _first = nullptr;
    _last  = nullptr;
  }
}

void OptionList::Print() const
{
  printf( "Options\n" );

  for( auto it = begin(); it != end(); ++it )
  {
    ( *it )->Print();
  }
}

uint16_t OptionList::Size() const
{
  if( IsEmpty() )
  {
    return ( 0 );
  }

  uint16_t count = 0;

  for( auto it = begin(); it != end(); ++it )
  {
    ++count;
  }

  return ( count );
}
}
}