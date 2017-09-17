/*
 * AdapterStore.h
 *
 *  Created on: Jul 1, 2017
 *      Author: psammand
 */

#ifndef NETWORK_INC_ADAPTERSTORE_H_
#define NETWORK_INC_ADAPTERSTORE_H_

#include <cstdint>

constexpr uint16_t ADAPTER_PROXY_STORE_CAPACITY = 6;

namespace ja_iot {
namespace network {
class AdapterProxyBase;
class IAdapter;
class AdapterProxyStoreImplData;

class AdapterProxyStore {
public:

  static AdapterProxyStore& Inst();

  void                      RegisterAdapterProxy(AdapterProxyBase const *adapter_proxy);
  uint16_t                  GetNoOfAdapters() const;
  uint16_t                  GetStoreCapacity() const;
  IAdapter                * GetAdapter(uint16_t index) const;

private:

  AdapterProxyStore();
  ~AdapterProxyStore() {}

  AdapterProxyStore(const AdapterProxyStore&)            = delete;
  AdapterProxyStore& operator=(const AdapterProxyStore&) = delete;

  AdapterProxyStoreImplData *impl_data_;
  static AdapterProxyStore *p_instance_;
};
}
}


#endif /* NETWORK_INC_ADAPTERSTORE_H_ */
