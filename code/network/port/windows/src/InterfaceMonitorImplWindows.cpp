/*
 * InterfaceMonitorImplWindows.cpp
 *
 *  Created on: Jul 6, 2017
 *      Author: psammand
 */

#ifdef _OS_WINDOWS_

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <mstcpip.h>
#include <ws2ipdef.h>
#include <iptypes.h>
#include <mswsock.h>
#include "ScopedMutex.h"
#include "OsalMgr.h"
#include "port/windows/inc/InterfaceMonitorImplWindows.h"
#include "SimpleList.h"

using namespace ja_iot::osal;
using namespace ja_iot::base;

namespace ja_iot {
namespace network {
//
using InterfaceAddressList          = SimpleList<InterfaceAddress, MAX_NO_OF_INTERFACE_ADDRESS>;
using InterfaceAddressPtrList       = StaticPtrArray<InterfaceAddress *, MAX_NO_OF_INTERFACE_ADDRESS>;
using InterfaceEventHandlerPtrArray = StaticPtrArray<IInterfaceEventHandler *, MAX_NO_OF_INTERFACE_EVENT_HANDLER>;

static DWORD WINAPI InterfaceMonitorThread( PVOID context );

class InterfaceMonitorImplWindowsData
{
  public:

    InterfaceMonitorImplWindowsData( InterfaceMonitorImplWindows *host ) : host_{ host } {}

  public:
    IP_ADAPTER_ADDRESSES* get_adapters();
    bool                  is_valid_addr( IP_ADAPTER_UNICAST_ADDRESS *p_unicast_adapter_addr );
    void                  get_if_addr_list_for_index( uint16_t index, PtrArray<InterfaceAddress *> &if_addr_ptr_list );
    bool                  register_for_addr_change();
    bool                  unregister_for_addr_change();
    void                  reset();
    void                  run_if_monitor_thread();
    void                  handle_interface_addr_change();
    void                  notify_if_modified();
    void                  notify_if_state_changed( InterfaceStatusFlag interface_status );

  public:
    InterfaceMonitorImplWindows *   host_                  = nullptr;
    Mutex *                         access_mutex_          = nullptr;
    HANDLE                          shutdown_event_        = nullptr;
    HANDLE                          monitor_thread_handle_ = nullptr;
    bool                            is_old_if_addr_lost_   = false;
    InterfaceAddressPtrList         curr_if_addr_ptr_list_;
    InterfaceAddressPtrList         new_if_addr_ptr_list_;
    InterfaceAddressList            if_addr_store_;
    InterfaceEventHandlerPtrArray   if_event_handler_ptr_array_;
};

InterfaceMonitorImplWindows::InterfaceMonitorImplWindows ()
{
  pimpl_ = new InterfaceMonitorImplWindowsData{ this };
}

ErrCode InterfaceMonitorImplWindows::StartMonitor( AdapterType adapter_type )
{
  pimpl_->access_mutex_ = OsalMgr::Inst()->AllocMutex();
  pimpl_->curr_if_addr_ptr_list_.Clear();

  if( !pimpl_->register_for_addr_change() )
  {
    pimpl_->reset();

    return ( ErrCode::ERR );
  }

  return ( ErrCode::OK );
}

ErrCode InterfaceMonitorImplWindows::StopMonitor( AdapterType adapter_type )
{
  pimpl_->unregister_for_addr_change();
  pimpl_->reset();

  return ( ErrCode::OK );
}

ErrCode InterfaceMonitorImplWindows::GetInterfaceAddrList( InterfaceAddressPtrArray &if_ptr_array, bool skip_if_down )
{
  ScopedMutex lock( pimpl_->access_mutex_ );

  if( pimpl_->curr_if_addr_ptr_list_.Count() > 0 )
  {
    for( int i = 0; i < pimpl_->curr_if_addr_ptr_list_.Count(); i++ )
    {
      auto if_addr = pimpl_->curr_if_addr_ptr_list_.GetItem( i );

      if( if_addr != nullptr )
      {
        if( skip_if_down && ( ( if_addr->getFlags() & IFF_UP ) != IFF_UP ) )
        {
          continue;
        }

        auto newIfAddr = new InterfaceAddress( *if_addr );
        if_ptr_array.Add( newIfAddr );
      }
    }
  }

  return ( ErrCode::OK );
}

ErrCode InterfaceMonitorImplWindows::GetNewlyFoundInterface( InterfaceAddressPtrArray &newly_found_if_addr_ptr_list )
{
  pimpl_->access_mutex_->Lock();

  pimpl_->is_old_if_addr_lost_ = false;

  if( pimpl_->new_if_addr_ptr_list_.Count() > 0 )
  {
    for( int i = 0; i < pimpl_->new_if_addr_ptr_list_.Count(); i++ )
    {
      auto if_addr = pimpl_->new_if_addr_ptr_list_.GetItem( i );

      if( if_addr != nullptr )
      {
        newly_found_if_addr_ptr_list.Add( new InterfaceAddress{ *if_addr } );

        /* free the interface address */
        pimpl_->if_addr_store_.Free( if_addr );
      }
    }
  }

  pimpl_->new_if_addr_ptr_list_.Clear();

  bool is_old_if_addr_lost{ pimpl_->is_old_if_addr_lost_ };
  bool is_new_if_addr_found{ pimpl_->new_if_addr_ptr_list_.Count() > 0 };

  pimpl_->access_mutex_->Unlock();

  if( is_old_if_addr_lost )
  {
    pimpl_->notify_if_state_changed( InterfaceStatusFlag::DOWN );
  }

  if( is_new_if_addr_found )
  {
    pimpl_->notify_if_state_changed( InterfaceStatusFlag::UP );
  }

  return ( ErrCode::OK );
}

void InterfaceMonitorImplWindows::AddInterfaceEventHandler( IInterfaceEventHandler *interface_event_handler )
{
  if( interface_event_handler != nullptr )
  {
    pimpl_->if_event_handler_ptr_array_.Add( interface_event_handler );
  }
}

void InterfaceMonitorImplWindows::RemoveInterfaceEventHandler( IInterfaceEventHandler *interface_event_handler )
{
  if( interface_event_handler != nullptr )
  {
    pimpl_->if_event_handler_ptr_array_.Remove( interface_event_handler );
  }
}

void InterfaceMonitorImplWindowsData::notify_if_state_changed( InterfaceStatusFlag interface_status )
{
  if( if_event_handler_ptr_array_.Count() > 0 )
  {
    InterfaceEvent interface_event{ InterfaceEventType::kInterfaceStateChanged };
    interface_event.setInterfaceStatusFlag( interface_status );

    for( int i = 0; i < if_event_handler_ptr_array_.Count(); i++ )
    {
      auto if_modified_handler = if_event_handler_ptr_array_.GetItem( i );

      if( if_modified_handler != nullptr )
      {
        if_modified_handler->HandleInterfaceEvent( &interface_event );
      }
    }
  }
}

void InterfaceMonitorImplWindowsData::notify_if_modified()
{
  if( if_event_handler_ptr_array_.Count() > 0 )
  {
    InterfaceEvent interface_event{ InterfaceEventType::kInterfaceModified };
    interface_event.setAdapterType( AdapterType::IP );

    for( int i = 0; i < if_event_handler_ptr_array_.Count(); i++ )
    {
      auto if_modified_handler = if_event_handler_ptr_array_.GetItem( i );

      if( if_modified_handler != nullptr )
      {
        if_modified_handler->HandleInterfaceEvent( &interface_event );
      }
    }
  }
}

IP_ADAPTER_ADDRESSES * InterfaceMonitorImplWindowsData::get_adapters()
{
  ULONG                 u32_adapter_addr_buf_len = 0;
  IP_ADAPTER_ADDRESSES *p_adapter_adrr_start_ptr = nullptr;

  /*We don't need most of the default information, so optimize this call by not asking for them. */
  ULONG                 flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME;

  /*
   *   Call up to 3 times: once to get the size, once to get the data, and once more
   *   just in case there was an increase in length in between the first two. If the
   *   length is still increasing due to more addresses being added, even this may fail
   *   and we'll have to wait for the next IP address change notification.
   */
  for( int i = 0; i < 3; i++ )
  {
    ULONG ret = GetAdaptersAddresses( AF_UNSPEC, flags, NULL, p_adapter_adrr_start_ptr, &u32_adapter_addr_buf_len );

    if( ERROR_BUFFER_OVERFLOW == ret )
    {
      // Redo with updated length.
      if( p_adapter_adrr_start_ptr != nullptr )
      {
        delete p_adapter_adrr_start_ptr;
      }

      p_adapter_adrr_start_ptr = (IP_ADAPTER_ADDRESSES *) new unsigned char[u32_adapter_addr_buf_len];

      if( p_adapter_adrr_start_ptr == nullptr )
      {
        break;
      }

      continue;
    }

    if( NO_ERROR != ret )
    {
      break;
    }

    // Succeeded getting adapters
    return ( p_adapter_adrr_start_ptr );
  }

  if( p_adapter_adrr_start_ptr != NULL )
  {
    delete p_adapter_adrr_start_ptr;
  }

  return ( NULL );
}

bool InterfaceMonitorImplWindowsData::is_valid_addr( IP_ADAPTER_UNICAST_ADDRESS *p_unicast_adapter_addr )
{
  if( p_unicast_adapter_addr->Address.lpSockaddr->sa_family != AF_INET6 )
  {
    // All IPv4 addresses are valid.
    return ( true );
  }

  if( p_unicast_adapter_addr->Address.lpSockaddr->sa_family == AF_INET6 )
  {
    struct sockaddr_in6 *ipv6_socket_addr = (struct sockaddr_in6 *) p_unicast_adapter_addr->Address.lpSockaddr;

    // if the addr is link local then it it valid
    if( ( ipv6_socket_addr->sin6_addr.u.Byte[0] == 0xfe ) && ( ( ipv6_socket_addr->sin6_addr.u.Byte[1] & 0xc0 ) == 0x80 ) )
    {
      return ( true );
    }
  }

  /*
   *  PSOCKADDR_IN6 sockAddr = (PSOCKADDR_IN6) pAddress->Address.lpSockaddr;
   *
   *  if( Ipv6UnicastAddressScope( sockAddr->sin6_addr.s6_addr ) == ScopeLevelLink )
   *  {
   *      // IPv6 link local addresses are valid.
   *      return ( true );
   *  }
   */

  // Other IPv6 addresses are valid if they are DNS eligible.
  // That is, ignore temporary addresses.
  return ( ( p_unicast_adapter_addr->Flags & IP_ADAPTER_ADDRESS_DNS_ELIGIBLE ) != 0 );
}


inline void InterfaceMonitorImplWindowsData::get_if_addr_list_for_index( uint16_t index, PtrArray<InterfaceAddress *> &if_addr_ptr_list )
{
  if( index < 0 )
  {
    return;
  }

  auto p_adapters_list = get_adapters();

  if( p_adapters_list == nullptr )
  {
    return;
  }

  for( IP_ADAPTER_ADDRESSES *p_cur_adapter_addr = p_adapters_list; p_cur_adapter_addr != nullptr; p_cur_adapter_addr = p_cur_adapter_addr->Next )
  {
    printf( "adapter address index %u, type %u OperStatus %d \n", (uint32_t) p_cur_adapter_addr->IfIndex, (uint32_t) p_cur_adapter_addr->IfType, p_cur_adapter_addr->OperStatus );

    if( ( ( index > 0 ) && ( p_cur_adapter_addr->IfIndex != index ) ) || ( p_cur_adapter_addr->IfType & IF_TYPE_SOFTWARE_LOOPBACK ) || ( ( p_cur_adapter_addr->OperStatus & IfOperStatusUp ) == 0 ) )
    {
      continue;
    }

    for( IP_ADAPTER_UNICAST_ADDRESS *p_unicast_adapter_addr = p_cur_adapter_addr->FirstUnicastAddress;
      p_unicast_adapter_addr != NULL;
      p_unicast_adapter_addr = p_unicast_adapter_addr->Next )
    {
      char temp_addr_buf[INET6_ADDRSTRLEN] = { 0 };

      if( p_unicast_adapter_addr->Address.lpSockaddr->sa_family == AF_INET )
      {
        struct sockaddr_in *p_ipv4_addr = (struct sockaddr_in *) p_unicast_adapter_addr->Address.lpSockaddr;

        if( !inet_ntop( AF_INET, (void *) &p_ipv4_addr->sin_addr, temp_addr_buf, sizeof( temp_addr_buf ) ) )
        {
          continue;
        }
      }
      else if( p_unicast_adapter_addr->Address.lpSockaddr->sa_family == AF_INET6 )
      {
        struct sockaddr_in6 *p_ipv6_addr = (struct sockaddr_in6 *) p_unicast_adapter_addr->Address.lpSockaddr;

        if( !inet_ntop( AF_INET6, (void *) &p_ipv6_addr->sin6_addr, temp_addr_buf, sizeof( temp_addr_buf ) ) )
        {
          continue;
        }
      }

      printf( "Found address %s\n", &temp_addr_buf[0] );

      if( !is_valid_addr( p_unicast_adapter_addr ) )
      {
        continue;
      }

      auto interfaceAddress = if_addr_store_.Alloc();
      auto ip_addr_family   = ( p_unicast_adapter_addr->Address.lpSockaddr->sa_family == AF_INET ) ? IpAddrFamily::IPV4 : IpAddrFamily::IPv6;
      interfaceAddress->setIndex( p_cur_adapter_addr->IfIndex );
      interfaceAddress->setFlags( IFF_UP );
      interfaceAddress->setFamily( ip_addr_family );
      interfaceAddress->set_addr( &temp_addr_buf[0] );

      if_addr_ptr_list.Add( interfaceAddress );
    }
  }

  delete p_adapters_list;
}

inline bool InterfaceMonitorImplWindowsData::register_for_addr_change()
{
  shutdown_event_ = CreateEvent( NULL, TRUE, FALSE, NULL );

  if( shutdown_event_ == 0 )
  {
    return ( false );
  }

  monitor_thread_handle_ = CreateThread( nullptr, 0, InterfaceMonitorThread, this, 0, nullptr );

  if( monitor_thread_handle_ == 0 )
  {
    CloseHandle( shutdown_event_ );
    shutdown_event_ = 0;
    return ( false );
  }

  handle_interface_addr_change();

  return ( true );
}

inline bool InterfaceMonitorImplWindowsData::unregister_for_addr_change()
{
  if( shutdown_event_ != nullptr )
  {
    SetEvent( shutdown_event_ );
    WaitForSingleObject( monitor_thread_handle_, INFINITE );
    CloseHandle( shutdown_event_ );
    shutdown_event_ = nullptr;
  }

  if( monitor_thread_handle_ != nullptr )
  {
    CloseHandle( monitor_thread_handle_ );
    monitor_thread_handle_ = nullptr;
  }

  return ( true );
}

inline void InterfaceMonitorImplWindowsData::reset()
{
  if( new_if_addr_ptr_list_.Count() > 0 )
  {
    for( int i = 0; i < new_if_addr_ptr_list_.Count(); i++ )
    {
      auto if_addr = new_if_addr_ptr_list_.GetItem( i );

      if( if_addr != nullptr )
      {
        if_addr_store_.Free( if_addr );
      }
    }
  }

  new_if_addr_ptr_list_.Clear();

  if( curr_if_addr_ptr_list_.Count() > 0 )
  {
    for( int i = 0; i < curr_if_addr_ptr_list_.Count(); i++ )
    {
      auto if_addr = curr_if_addr_ptr_list_.GetItem( i );

      if( if_addr != nullptr )
      {
        if_addr_store_.Free( if_addr );
      }
    }
  }

  curr_if_addr_ptr_list_.Clear();

  if( access_mutex_ != nullptr )
  {
    OsalMgr::Inst()->FreeMutex( access_mutex_ );
    access_mutex_ = nullptr;
  }
}

inline void InterfaceMonitorImplWindowsData::run_if_monitor_thread()
{
  {
    OVERLAPPED overlapped = { 0 };
    WSADATA wsaData{};

    if( WSAStartup( MAKEWORD( 2, 2 ), &wsaData ) != NO_ERROR )
    {
      return;
    }

    SOCKET nwmSocket = WSASocketW( AF_INET6, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );

    if( INVALID_SOCKET == nwmSocket )
    {
      WSACleanup();

      return;
    }

    // Put socket into dual IPv4/IPv6 mode.
    BOOL ipv6Only = FALSE;

    if( SOCKET_ERROR == setsockopt( nwmSocket, IPPROTO_IPV6, IPV6_V6ONLY, (char *) &ipv6Only, sizeof( ipv6Only ) ) )
    {
      closesocket( nwmSocket );
      WSACleanup();

      return;
    }

    overlapped.hEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    if( NULL == overlapped.hEvent )
    {
      closesocket( nwmSocket );

      WSACleanup();
      return;
    }

    WSAEVENT eventList[2]  = { overlapped.hEvent, shutdown_event_, };
    DWORD    bytesReturned = 0;

    for(;; )
    {
      if( SOCKET_ERROR == WSAIoctl( nwmSocket, SIO_ADDRESS_LIST_CHANGE, NULL, 0, NULL, 0, &bytesReturned, &overlapped, NULL ) )
      {
        int err = WSAGetLastError();

        if( err != ERROR_IO_PENDING )
        {
          break;
        }
        else
        {
          // Wait for an address change or a request to cancel the thread.
          DWORD waitStatus = WSAWaitForMultipleEvents( _countof( eventList ), eventList, FALSE, WSA_INFINITE, FALSE );

          if( waitStatus != WSA_WAIT_EVENT_0 )
          {
            // The cancel event was signaled.  There is no need to call CancelIo
            // here, because we will close the socket handle below, causing any
            // pending I/O to be canceled then.
            break;
          }

          WSAResetEvent( overlapped.hEvent );
        }
      }

      // We have a change to process.  The address change callback ignores the parameters, so we just pass default values.
      handle_interface_addr_change();
    }

    closesocket( nwmSocket );
    CloseHandle( overlapped.hEvent );
    WSACleanup();
    return;
  }
}

static DWORD WINAPI InterfaceMonitorThread( PVOID context )
{
  InterfaceMonitorImplWindowsData *implData = (InterfaceMonitorImplWindowsData *) context;

  if( implData != nullptr )
  {
    implData->run_if_monitor_thread();
  }

  return ( 1 );
}

inline void InterfaceMonitorImplWindowsData::handle_interface_addr_change()
{
  InterfaceAddressPtrList temp_old_if_addr_list{};
  ScopedMutex lock( access_mutex_ );

  if( curr_if_addr_ptr_list_.Count() > 0 )
  {
    /* copy the old interface addresses to temp */
    for( int i = 0; i < curr_if_addr_ptr_list_.Count(); i++ )
    {
      auto curr_if = curr_if_addr_ptr_list_.GetItem( i );

      if( curr_if == nullptr )
      {
        continue;
      }

      if( !temp_old_if_addr_list.Add( curr_if ) )
      {
        break;
      }
    }
  }

  curr_if_addr_ptr_list_.Clear();   // clear it to get new addresses

  get_if_addr_list_for_index( 0, curr_if_addr_ptr_list_ );

  // old - 5 6 7 8 9
  // new - 5 6 7 8 9 - case 1
  // new - 2 6 7 8 9 - case 2

  is_old_if_addr_lost_ = false;

  // check for the old address mising
  if( temp_old_if_addr_list.Count() > 0 )
  {
    for( int i = 0; i < temp_old_if_addr_list.Count(); i++ )
    {
      auto old_if = temp_old_if_addr_list.GetItem( i );

      if( old_if == nullptr )
      {
        continue;
      }

      is_old_if_addr_lost_ = true;

      if( curr_if_addr_ptr_list_.Count() > 0 )
      {
        for( int i = 0; i < curr_if_addr_ptr_list_.Count(); i++ )
        {
          auto new_if = curr_if_addr_ptr_list_.GetItem( i );

          if( new_if == nullptr )
          {
            continue;
          }

          if( old_if == new_if )
          {
            is_old_if_addr_lost_ = false;
            break;
          }
        }
      }

      if( is_old_if_addr_lost_ )
      {
        break;
      }
    }
  }

  // new - 5 6 7 8
  // old -          - case 1
  // old - 1 2 3	  - case 2
  // old - 5 6 7 8  - case 3
  // check for any new address addition
  if( curr_if_addr_ptr_list_.Count() > 0 )
  {
    for( int i = 0; i < curr_if_addr_ptr_list_.Count(); i++ )
    {
      auto new_if = curr_if_addr_ptr_list_.GetItem( i );

      if( new_if == nullptr )
      {
        continue;
      }

      bool found{ false };

      if( temp_old_if_addr_list.Count() > 0 )
      {
        for( int i = 0; i < temp_old_if_addr_list.Count(); i++ )
        {
          auto oldAddr = temp_old_if_addr_list.GetItem( i );

          if( oldAddr == nullptr )
          {
            continue;
          }

          if( oldAddr == new_if )
          {
            found = true;
            break;
          }
        }
      }

      if( !found )
      {
        auto newIfAddr = if_addr_store_.Alloc();
        *newIfAddr = *new_if;

        new_if_addr_ptr_list_.Add( newIfAddr );
      }
    }
  }

  if( new_if_addr_ptr_list_.Count() > 0 )
  {
    notify_if_modified();
  }

  if( temp_old_if_addr_list.Count() > 0 )
  {
    for( int i = 0; i < temp_old_if_addr_list.Count(); i++ )
    {
      auto oldAddr = temp_old_if_addr_list.GetItem( i );

      if( oldAddr == nullptr )
      {
        continue;
      }

      if_addr_store_.Free( oldAddr );
    }
  }
}
}
}

#endif// #ifdef _OS_WINDOWS_
