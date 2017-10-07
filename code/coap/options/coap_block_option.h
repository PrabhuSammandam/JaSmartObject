#pragma once

#include <cstdint>
#include "BaseObject.h"
#include <IMemAllocator.h>

namespace ja_iot {
namespace coap {
class BlockOption
{
  uint32_t   _data = 0;

  public:

    BlockOption ();

    BlockOption( uint8_t szx, bool m, uint32_t num );

    uint8_t getSZX() const;
    void    setSZX( uint8_t szx );

    uint16_t getSize() const;
    void     setSize( uint16_t size );

    bool hasMore() const;
    void setMore( bool setValue );

    uint32_t getNum() const;
    void     setNum( uint32_t num );

    uint8_t getLen() const;
    void    setLen( uint8_t len );

    void decode( uint8_t *buf, uint8_t bufLen );
    void encode( uint8_t *buf, uint8_t &bufLen ) const;

    static uint8_t  sizeToSZX( uint16_t size );
    static uint16_t szxToSize( uint8_t szx );

    DEFINE_MEMORY_OPERATORS( BlockOption );
};
}
}
