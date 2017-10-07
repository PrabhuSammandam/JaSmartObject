/*
 * InterfaceAddress.h
 *
 *  Created on: Jul 5, 2017
 *      Author: psammand
 */

#ifndef INTERFACEADDRESS_H_
#define INTERFACEADDRESS_H_

#include <ip_addr.h>
#include <string.h>
#include <cstdint>
#include "PtrArray.h"
#include "IMemAllocator.h"

constexpr uint16_t kInterfaceAddressMaxSize = 66;

namespace ja_iot {
namespace network {

using IpAddrFamily = ja_iot::base::IpAddrFamily;
//
class InterfaceAddress
{
  public:

    InterfaceAddress () {}

    InterfaceAddress( uint32_t index, uint32_t flags, IpAddrFamily family, const char *address );

    InterfaceAddress( const InterfaceAddress &other );

    IpAddrFamily getFamily() const { return ( family_ ); }
    void         setFamily( IpAddrFamily family ) { family_ = family; }
    uint32_t     getFlags() const { return ( flags_ ); }
    void         setFlags( uint32_t flags ) { flags_ = flags; }
    uint32_t     getIndex() const { return ( index_ ); }
    void         setIndex( uint32_t index ) { index_ = index; }
    void         set_addr( const char *addr );
    char*		 get_addr(){return &address_[0];}

    InterfaceAddress & operator = ( const InterfaceAddress &other );
    bool operator               == ( const InterfaceAddress &other );

    void clear_address();

    DEFINE_MEMORY_OPERATORS( x );

  private:

    uint32_t       index_                             = 0;
    uint32_t       flags_                             = 0;
    IpAddrFamily   family_                            = IpAddrFamily::IPV4;
    char           address_[kInterfaceAddressMaxSize] = { 0 };
};

using InterfaceAddressPtrArray = ja_iot::base::PtrArray<InterfaceAddress *>;
}
}


#endif /* INTERFACEADDRESS_H_ */
