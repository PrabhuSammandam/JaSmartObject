/*
 * AdapterProxyBase.h
 *
 *  Created on: Jul 1, 2017
 *      Author: psammand
 */

#ifndef NETWORK_INC_ADAPTERPROXYBASE_H_
#define NETWORK_INC_ADAPTERPROXYBASE_H_

#include "AdapterProxyStore.h"

namespace ja_iot {
namespace network {
class IAdapter;

class AdapterProxyBase
{
public:

   AdapterProxyBase ();

   virtual ~AdapterProxyBase () {}

   virtual IAdapter* GetAdapter() = 0;
};

inline AdapterProxyBase::AdapterProxyBase ()
{
   AdapterProxyStore::Inst().RegisterAdapterProxy( this );
}

template<class T>
class AdapterProxy : public AdapterProxyBase
{
public:

   AdapterProxy () {}

   virtual ~AdapterProxy () {}

   IAdapter* GetAdapter() override
   {
      return ( &holding_adapter_ );
   }

private:

   T holding_adapter_{};
};
}
}

#endif /* NETWORK_INC_ADAPTERPROXYBASE_H_ */