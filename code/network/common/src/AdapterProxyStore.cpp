/*
 * AdapterProxyStore.cpp
 *
 *  Created on: Jul 2, 2017
 *      Author: psammand
 */

#include <stdio.h>
#include <AdapterProxyStore.h>
#include "AdapterProxy.h"
#include "PtrArray.h"

namespace ja_iot
{
namespace network
{
using AdapterProxyArray = ja_iot::base::StaticPtrArray<AdapterProxyBase *, ADAPTER_PROXY_STORE_CAPACITY>;

struct AdapterProxyStoreImplData
{
    AdapterProxyArray    _adapterProxyArray{};
};

static AdapterProxyStoreImplData _adapterProxyStoreImplData{};
AdapterProxyStore *AdapterProxyStore::p_instance_{ nullptr };

AdapterProxyStore & network::AdapterProxyStore::Inst()
{
    if( p_instance_ == nullptr )
    {
        static AdapterProxyStore _instance{};
        p_instance_ = &_instance;
    }

    return ( *p_instance_ );
}
AdapterProxyStore::AdapterProxyStore ()
{
    impl_data_ = &_adapterProxyStoreImplData;
}

void AdapterProxyStore::RegisterAdapterProxy( const AdapterProxyBase *adapterProxy )
{
    printf( "registering adapter proxy\n" );
    impl_data_->_adapterProxyArray.Add( (AdapterProxyBase *) adapterProxy );
}

uint16_t AdapterProxyStore::GetNoOfAdapters() const
{
    return ( impl_data_->_adapterProxyArray.Count() );
}

IAdapter * AdapterProxyStore::GetAdapter( uint16_t index ) const
{
    auto adapterProxyBase = impl_data_->_adapterProxyArray[ index ];

    if( adapterProxyBase != nullptr )
    {
        return ( adapterProxyBase->GetAdapter() );
    }

    return ( nullptr );
}
uint16_t AdapterProxyStore::GetStoreCapacity() const
{
    return ( impl_data_->_adapterProxyArray.GetSize() );
}
}
}
