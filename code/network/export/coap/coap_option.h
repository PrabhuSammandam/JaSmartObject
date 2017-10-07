#pragma once

#include <cstdint>
#include <cstddef>

namespace ja_iot {
namespace network {
constexpr uint16_t OPTION_ERROR_CRITICAL_MASK          = 0x0100;
constexpr uint16_t OPTION_ERROR_CRITICAL_UNKNOWN       = 0x0101;
constexpr uint16_t OPTION_ERROR_CRITICAL_REPEAT_MORE   = 0x0102;
constexpr uint16_t OPTION_ERROR_CRITICAL_LEN_RANGE_OUT = 0x0104;
constexpr uint16_t OPTION_ERROR_UNKNOWN                = 0x0001;
constexpr uint16_t OPTION_ERROR_REPEAT_MORE            = 0x0002;
constexpr uint16_t OPTION_ERROR_LEN_RANGE_OUT          = 0x0004;

enum class ContentFormatId
{
  NONE              = 0xFFFF,
  TEXT__PLAIN       = 0,
  APP__LINK_FORMAT  = 40,
  APP__XML          = 41,
  APP__OCTET_STREAM = 42,
  APP__EXI          = 47,
  APP__JSON         = 50
};

enum class OptionType
{
  IF_MATCH       = 1,
  URI_HOST       = 3,
  ETAG           = 4,
  IF_NONE_MATCH  = 5,
  OBSERVE        = 6,
  URI_PORT       = 7,
  LOCATION_PATH  = 8,
  URI_PATH       = 11,
  CONTENT_FORMAT = 12,
  MAX_AGE        = 14,
  URI_QUERY      = 15,
  ACCEPT         = 17,
  LOCATION_QUERY = 20,
  BLOCK_2        = 23,
  BLOCK_1        = 27,
  SIZE2          = 28,
  PROXY_URI      = 35,
  PROXU_SCHEME   = 39,
  SIZE1          = 60
};

class Option
{
  uint16_t   _no   = 0;
  uint16_t   _len  = 0;
  uint8_t *  _val  = nullptr;
  Option *   _next = nullptr;

  void * operator new ( size_t len, uint16_t buf_len );
  void operator   delete ( void *mem, uint16_t buf_len );

  public:

    Option () {}

    ~Option ();

    Option( uint16_t no, uint16_t len, uint8_t *val );

    Option( Option const &opt );

    uint16_t GetNo() const { return ( _no ); }
    void     SetNo( uint16_t no ) { _no = no; }

    uint16_t GetLen() const { return ( _len ); }
    void     SetLen( uint16_t len ) { _len = len; }

    uint8_t* GetVal() const { return ( _val ); }
    void     SetVal( uint8_t *val ) { _val = val; }

    Option* GetNext() const { return ( _next ); }
    void    SetNext( Option *opt ) { _next = opt; }

    void Print() const;

  public:
    static void    Destroy( Option *opt );
    static Option* Allocate( uint16_t no, uint16_t len, uint8_t *val );
    static Option* Allocate();
};

class OptionListIterator
{
  public:
    Option * _node;

    OptionListIterator () { _node = nullptr; }

    explicit OptionListIterator( Option *node ) :
      _node( node )
    {
    }

    ~OptionListIterator () { _node = nullptr; }

    bool operator != ( const OptionListIterator &other ) const { return ( _node != other._node ); }

    bool operator == ( const OptionListIterator &other ) const { return ( _node == other._node ); }

    void operator ++ () { _node = _node->GetNext(); }

    void operator ++ ( int ) { _node = _node->GetNext(); }

    Option * operator * () const { return ( _node ); }
};

class OptionList
{
  Option * _first = nullptr;
  Option * _last = nullptr;

  public:

    OptionList ();

    ~OptionList ();

    bool IsEmpty() const { return ( _first == nullptr && _last == nullptr ); }

    void     InsertFront( Option *option );
    void     InsertEnd( Option *option );
    void     Insert( Option *option );
    void     FreeList();
    void     Print() const;
    uint16_t Size() const;

    /*iterator*/
    OptionListIterator begin() const { return ( OptionListIterator( _first ) ); }

    OptionListIterator end() const { return ( OptionListIterator( nullptr ) ); }
};
}
}
