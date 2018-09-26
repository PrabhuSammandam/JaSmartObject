#include "libbase_ptr_array.h"
#include "cute.h"
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <functional>

#include "PtrArray.h"

using namespace ja_iot::base;
using namespace std;

void ptr_array_test_1()
{
  uint32_t *           u32_array[10] {};

  PtrArray<uint32_t *> ptr_array( &u32_array[0], 10 );

  ptr_array.Add( (uint32_t *) 10 );
  ptr_array.Add( (uint32_t *) 20 );

  ASSERTM( "Ptr Array Count ", ptr_array.Count() == 2 );
  ASSERTM( "Ptr Array Index for 1st item", ptr_array.GetIndex( (uint32_t *) 10 ) == 0 );
  ASSERTM( "Ptr Array Index for 2st item", ptr_array.GetIndex( (uint32_t *) 20 ) == 1 );
}

class IfAddr
{
  int   index;

  public:
    IfAddr( int index )
    {
      this->index = index;
      cout << "called constructor" << endl;
    }
    ~IfAddr ()
    {
      cout << "called destructor" << endl;
    }
    IfAddr( const IfAddr &other )
    {
      this->index = other.index;
      cout << "called copy constructor" << endl;
    }
    IfAddr( const IfAddr &&other )
    {
      this->index = other.index;
      cout << "called move constructor" << endl;
    }

    IfAddr & operator = ( const IfAddr &other )
    {
      cout << "called copy assignment" << endl;
      return ( *this );
    }
    IfAddr & operator = ( const IfAddr &&other )
    {
      cout << "called move assignment" << endl;
      return ( *this );
    }

    int get_index() const
    {
      return ( index );
    }

    void set_index( int index )
    {
      this->index = index;
    }
};

void test_vector()
{
  std::vector<IfAddr *> if_addrs{};

  for(int i = 0; i < 20; i++)
  {
	  if_addrs.push_back(new IfAddr{1});
  }

  auto if1 = new IfAddr{ 10 };

  if_addrs.push_back( if1 );
  cout << "pushed local object" << endl;
  if_addrs.emplace_back( new IfAddr{ 20 } );
  cout << "pushed using emblac" << endl;

  // IfAddr& item = if_addrs.at(0);
  auto item = std::find( if_addrs.cbegin(), if_addrs.cend(), if1 );

  if( item != if_addrs.cend() )
  {
    cout << "index " << ( *item )->get_index();
  }

  std::for_each( if_addrs.cbegin(), if_addrs.cend(), [] ( IfAddr *i ) {
    delete i;
  } );

  if_addrs.clear();
}

class Option
{
  int   no     = 0;
  int   length = 0;

  public:
    Option ()
    {
    }
    Option( int no, int length )
    {
      this->no     = no;
      this->length = length;
    }

    int get_length() const
    {
      return ( length );
    }

    void set_length( int length )
    {
      this->length = length;
    }

    int get_no() const
    {
      return ( no );
    }

    void set_no( int no )
    {
      this->no = no;
    }
};

bool operator < ( const Option &obj1, const Option &obj2 )
{
  return ( obj1.get_no() < obj2.get_no() );
}

void test_multiset()
{
  std::multiset<Option> optionSet{};

  Option opt{ 10, 1 };

  optionSet.insert( opt );
  optionSet.insert( { 20, 2 } );
  optionSet.insert( { 20, 3 } );
  optionSet.insert( { 20, 4 } );

  for( auto it = optionSet.cbegin(); it != optionSet.cend(); ++it )
  {
    cout << "Option no " << ( *it ).get_no() << " length " << ( *it ).get_length() << endl;
  }

  auto opt10 = std::find_if( optionSet.cbegin(), optionSet.cend(), [] ( const Option &opt ) {
    if( opt.get_no() == 10 )
    {
      return ( true );
    }

    return ( false );
  } );

  cout << "Option no " << ( *opt10 ).get_no() << " length " << ( *opt10 ).get_length() << endl;
}

class MsgProducer
{
  std::function<void(uint8_t *data, uint16_t data_buf_size)>   callback = nullptr;

  public:
    MsgProducer () : callback{}
    {
    }
    void setHandler( std::function<void(uint8_t *data, uint16_t data_buf_size)> callback )
    {
      this->callback = callback;
    }

    void send_msg()
    {
      if( callback )
      {
        callback( nullptr, 10 );
      }
    }
};

class MsgHandler
{
  public:
    MsgHandler () {}
    void handler_msg( uint8_t *data, uint16_t data_buf_size )
    {
      cout << "called the handler_msg api" << endl;
    }
};

void test_callback_functor()
{
  MsgHandler handler{};
  MsgProducer producer{};

  auto callback = std::bind( &MsgHandler::handler_msg, handler, std::placeholders::_1, std::placeholders::_2 );

  producer.setHandler( callback );
  producer.send_msg();

  producer.setHandler( nullptr );
  producer.send_msg();

  auto lambda_callback = [&] ( uint8_t *data, uint16_t data_buf_size ) {
      handler.handler_msg( data, data_buf_size );
    };

  producer.setHandler( lambda_callback );
  producer.send_msg();
}

cute::suite make_suite_libbase_ptr_array()
{
  cute::suite s{};
  // s.push_back( CUTE( ptr_array_test_1 ) );
   s.push_back( CUTE( test_vector ) );
  // s.push_back( CUTE( test_multiset ) );
//  s.push_back( CUTE( test_callback_functor ) );
  return ( s );
}
