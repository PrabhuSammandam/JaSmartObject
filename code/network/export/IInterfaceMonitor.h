/*
 * IInterfaceMonitor.h
 *
 *  Created on: Jul 6, 2017
 *      Author: psammand
 */

#ifndef IINTERFACEMONITOR_H_
#define IINTERFACEMONITOR_H_

#include "ErrCode.h"
#include "InterfaceAddress.h"
#include "PtrArray.h"
#include "TransportType.h"

using ErrCode = ja_iot::base::ErrCode;

namespace ja_iot {
namespace network {
enum class InterfaceEventType
{
  kInterfaceAdded,
  kInterfaceRemoved,
  kInterfaceModified,
  kInterfaceStateChanged
};

/**
 * Event class for the interface monitor. The InterfaceMonitor class will inform
 * the clients by calling the handler function with this InterfaceEvent object.
 */
class InterfaceEvent
{
public:
  InterfaceEvent(InterfaceEventType interface_event_type)
    : interface_event_type_{ interface_event_type }
  {
  }

  InterfaceEventType getInterfaceEventType() const
  {
    return (interface_event_type_);
  }
  void setInterfaceEventType(InterfaceEventType interface_event_type)
  {
    interface_event_type_ = interface_event_type;
  }
  AdapterType getAdapterType() const { return (adapter_type_); }
  void setAdapterType(AdapterType adapter_type)
  {
    adapter_type_ = adapter_type;
  }
  InterfaceStatusFlag getInterfaceStatusFlag() const
  {
    return (interface_status_flag_);
  }
  void setInterfaceStatusFlag(InterfaceStatusFlag interface_status_flag)
  {
    interface_status_flag_ = interface_status_flag;
  }

private:
  InterfaceEventType interface_event_type_;
  AdapterType adapter_type_ = AdapterType::TYPE_DEFAULT;
  InterfaceStatusFlag interface_status_flag_ = InterfaceStatusFlag::DOWN;
};

class IInterfaceEventHandler
{
public:
  IInterfaceEventHandler() {}

  virtual ~IInterfaceEventHandler() {}

public:
  virtual ErrCode HandleInterfaceEvent(InterfaceEvent* interface_event) = 0;
};

class IInterfaceMonitor
{
public:
  virtual ~IInterfaceMonitor() {}

  virtual ErrCode StartMonitor(AdapterType adapter_type) = 0;
  virtual ErrCode StopMonitor(AdapterType adapter_type) = 0;

  /**
   * This function will return all the found interfaces in the system.
   * @param interface_address_ptr_array	- interface address list which will be
   * populated.
   * @param skip_if_down					- this flag will control whether to
   * include only the interfaces which are UP.
   * @return
   */
  virtual ErrCode GetInterfaceAddrList(
    InterfaceAddressPtrArray& interface_address_ptr_array,
    bool skip_if_down = true) = 0;
  /**
   * This function should be called only after getting the kInterfaceModified
   * event notification. Calling this function
   * will return all the newly found interfaces.
   * @param interface_address_ptr_array	- interface address list which will be
   * populated.
   * @return
   */
  virtual ErrCode GetNewlyFoundInterface(
    InterfaceAddressPtrArray& interface_address_ptr_array) = 0;

  virtual void AddInterfaceEventHandler(
    IInterfaceEventHandler* p_interface_event_handler) = 0;
  virtual void RemoveInterfaceEventHandler(
    IInterfaceEventHandler* p_interface_event_handler) = 0;

private:
};

#if 0

class InterfaceMonitorProxyBase;

class InterfaceMonitorMgr
{
  public:

    static InterfaceMonitorMgr& Inst();

    void RegisterInterfaceMonitor( InterfaceMonitorProxyBase *interface_monitor_proxy );

    IInterfaceMonitor* GetInterfaceMonitor() { return ( interface_monitor_ ); }

  private:

    InterfaceMonitorMgr () {}

    ~InterfaceMonitorMgr () {}

    InterfaceMonitorMgr( const InterfaceMonitorMgr & )               = delete;
    InterfaceMonitorMgr & operator = ( const InterfaceMonitorMgr & ) = delete;

    IInterfaceMonitor *interface_monitor_{ nullptr };
    static InterfaceMonitorMgr * p_instance_;
};


class InterfaceMonitorProxyBase
{
  public:

    InterfaceMonitorProxyBase ();

    virtual ~InterfaceMonitorProxyBase () {}

    virtual IInterfaceMonitor* GetInterfaceMonitor() = 0;
};

inline InterfaceMonitorProxyBase::InterfaceMonitorProxyBase ()
{
  InterfaceMonitorMgr::Inst().RegisterInterfaceMonitor( this );
}


template<class T>
class InterfaceMonitorProxy : public InterfaceMonitorProxyBase
{
  public:

    InterfaceMonitorProxy () {}

    virtual ~InterfaceMonitorProxy () {}

    IInterfaceMonitor* GetInterfaceMonitor() override { return ( &_host ); }

  private:

    T _host{};
};

#endif
}
}

#endif /* IINTERFACEMONITOR_H_ */
