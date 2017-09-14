/*
 * StaticArray.h
 *
 *  Created on: Jun 30, 2017
 *      Author: psammand
 */

#ifndef __PTR_ARRAY_H__
#define __PTR_ARRAY_H__

//#include <functional>
#include <stddef.h>
#include <cstdint>

namespace ja_iot {
namespace base {
template<typename T>
class PtrArray
{
  private:

    T *        _list;
    uint16_t   _count = 0;
    uint16_t   _capacity;

  public:

    PtrArray( T *itemBasePtr, uint16_t capacity ) : _list{ itemBasePtr }, _capacity{ capacity }
    {
      if( itemBasePtr != nullptr )
      {
        for( int i = 0; i < capacity; i++ )
        {
          _list[i] = nullptr;
        }
      }
    }

    bool Add( T entry )
    {
      if( IsFull() )
      {
        return ( false );
      }

      for( uint16_t i = 0; i < _capacity; i++ )
      {
        if( _list[i] == nullptr )
        {
          _list[i] = entry;
          _count++;
          return ( true );
        }
      }

      return ( false );
    }

    void Remove( T listEntry )
    {
      for( uint16_t i = 0; i < _capacity; i++ )
      {
        if( _list[i] == listEntry )
        {
          _count--;
          _list[i] = nullptr;
          break;
        }
      }
    }

    uint16_t GetSize() { return ( _capacity ); }

    uint16_t Count() { return ( _count ); }

    bool IsFull()
    {
      return ( _capacity == _count );
    }

    T GetItem( uint16_t index )
    {
      if( ( index >= 0 ) && ( index < _capacity ) )
      {
        return ( _list[index] );
      }

      return ( nullptr );
    }

    uint16_t GetIndex( T item )
    {
      for( uint16_t i = 0; i < _capacity; i++ )
      {
        if( _list[i] == item )
        {
          return ( i );
        }
      }

      return ( 0xFFFF );
    }

    void Clear()
    {
      for( uint16_t i = 0; i < _capacity; i++ )
      {
        _list[i] = nullptr;
      }

      _count = 0;
    }

    T operator [] ( int index )
    {
      if( ( index < 0 ) || ( index >= _capacity ) ){ return ( nullptr ); }

      return ( _list[index] );
    }

#if 0

    template<typename P>
    class PtrArrayIterator : public std::iterator<std::random_access_iterator_tag, P>
    {
      public:

        PtrArrayIterator( P *dataPtr = nullptr ) : _dataPtr{ dataPtr } {}

        PtrArrayIterator( const PtrArrayIterator<P> &rawIterator ) = default;

        ~PtrArrayIterator () {}

        PtrArrayIterator<P> & operator = ( const PtrArrayIterator<P> &rawIterator ) = default;
        PtrArrayIterator<P> & operator = ( P *ptr )
        {
          _dataPtr = ptr;
          return ( *this );
        }

        operator bool () const
        {
          if( _dataPtr )
          {
            return ( true );
          }
          else
          {
            return ( false );
          }
        }
        bool operator == ( const PtrArrayIterator<P> &rawIterator ) const { return ( _dataPtr == rawIterator.getConstPtr() ); }
        bool operator != ( const PtrArrayIterator<P> &rawIterator ) const { return ( _dataPtr != rawIterator.getConstPtr() ); }

        PtrArrayIterator<P> & operator += ( const ptrdiff_t &movement ) { _dataPtr += movement; return ( *this ); }
        PtrArrayIterator<P> & operator -= ( const ptrdiff_t &movement ) { _dataPtr -= movement; return ( *this ); }

        PtrArrayIterator<P> & operator ++ ()
        {
          ++_dataPtr;
          return ( *this );
        }
        PtrArrayIterator<P> & operator -- () { --_dataPtr; return ( *this ); }

        PtrArrayIterator<P> operator + ( const ptrdiff_t &movement ) { auto oldPtr = _dataPtr; _dataPtr += movement; auto temp( *this ); _dataPtr = oldPtr; return ( temp ); }
        PtrArrayIterator<P> operator - ( const ptrdiff_t &movement ) { auto oldPtr = _dataPtr; _dataPtr -= movement; auto temp( *this ); _dataPtr = oldPtr; return ( temp ); }

        ptrdiff_t operator - ( const PtrArrayIterator<P> &rawIterator ) { return ( std::distance( rawIterator.getPtr(), this->getPtr() ) ); }

        P & operator         * () { return ( *_dataPtr ); }
        const P   & operator * () const { return ( *_dataPtr ); }
        P * operator         -> () { return ( _dataPtr ); }

        P*       getPtr() const { return ( _dataPtr ); }
        const P* getConstPtr() const { return ( _dataPtr ); }

      protected:
        P *_dataPtr{ nullptr };
    };

    typedef PtrArrayIterator<T> iterator;
    typedef PtrArrayIterator<const T> const_iterator;

    iterator begin() { return ( iterator( &_list[0] ) ); }
    iterator end() { return ( iterator( &_list[_capacity] ) ); }

    const_iterator cbegin() { return ( const_iterator( &_list[0] ) ); }
    const_iterator cend() { return ( const_iterator( &_list[_capacity] ) ); }
#endif
};

template<typename T, uint16_t capacity = 10>
class StaticPtrArray : public PtrArray<T>
{
  private:
    T   _item_Array[capacity];

  public:

    StaticPtrArray () : PtrArray<T>( &_item_Array[0], capacity ) {}
};
}
}

#endif /* __PTR_ARRAY_H__ */
