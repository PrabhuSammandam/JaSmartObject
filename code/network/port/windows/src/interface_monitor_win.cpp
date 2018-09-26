/*
 * InterfaceMonitorImplWindows.cpp
 *
 *  Created on: Jul 6, 2017
 *      Author: psammand
 */

#ifdef _OS_WINDOWS_

#include <memory>
#include <vector>
#include <functional>
#include <algorithm>
#include <forward_list>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <ws2ipdef.h>
#include <iptypes.h>
#include "ScopedMutex.h"
#include "OsalMgr.h"
#include <port/windows/inc/interface_monitor_win.h>
#include <common/inc/logging_network.h>

using namespace ja_iot::osal;
using namespace ja_iot::base;

namespace ja_iot {
namespace network {
struct InterfaceMonitorCbData
{
  pfn_interface_monitor_cb   cz_if_monitor_cb = nullptr;
  void *                     pv_user_data     = nullptr;
  InterfaceMonitorCbData( const pfn_interface_monitor_cb cz_if_monitor_cb, void *pv_user_data )
    : cz_if_monitor_cb{ cz_if_monitor_cb },
    pv_user_data{ pv_user_data }
  {
  }
};

static DWORD WINAPI InterfaceMonitorThread( PVOID context );

class InterfaceMonitorImplWindowsData
{
  public:
    InterfaceMonitorImplWindowsData( InterfaceMonitorImplWindows *host ) : host_{ host }
    {
    }

    std::unique_ptr<IP_ADAPTER_ADDRESSES> get_adapters() const;
    bool                                  is_valid_addr( IP_ADAPTER_UNICAST_ADDRESS *p_unicast_adapter_addr ) const;
    std::vector<InterfaceAddress *>       get_if_addr_list_for_index( uint16_t index ) const;
    bool                                  register_for_addr_change();
    bool                                  unregister_for_addr_change();
    void                                  reset();
    void                                  run_if_monitor_thread();
    void                                  handle_interface_addr_change();
    void                                  notify_if_modified();
    void                                  notify_if_state_changed( InterfaceStatusFlag interface_status );

    InterfaceMonitorImplWindows *host_{};
    Mutex *access_mutex_{};
    HANDLE shutdown_event_{};
    HANDLE monitor_thread_handle_{};
    bool   is_old_if_addr_lost_ = false;
    std::vector<InterfaceAddress *> curr_if_addr_ptr_list_{};
    std::vector<InterfaceAddress *> new_if_addr_ptr_list_{};
    std::forward_list<InterfaceMonitorCbData *> _if_monitor_clients_list{};
};

InterfaceMonitorImplWindows::InterfaceMonitorImplWindows ()
{
  pimpl_ = std::make_unique<InterfaceMonitorImplWindowsData>( this );
}
InterfaceMonitorImplWindows::~InterfaceMonitorImplWindows ()
{
}

ErrCode InterfaceMonitorImplWindows::start_monitor( uint16_t adapter_type )
{
  auto ret_status = ErrCode::OK;

  DBG_INFO( "InterfaceMonitorImplWindows::StartMonitor:%d# ENTER adapter_type[%x]", __LINE__, (int) adapter_type );

  pimpl_->access_mutex_ = OsalMgr::Inst()->AllocMutex();

  if( pimpl_->access_mutex_ == nullptr )
  {
    DBG_ERROR( "InterfaceMonitorImplWindows::StartMonitor:%d# AllocMutex FAILED", __LINE__ );
    ret_status = ErrCode::ERR;
    goto exit_label_;
  }

  pimpl_->curr_if_addr_ptr_list_.clear();

  if( !pimpl_->register_for_addr_change() )
  {
    DBG_ERROR( "InterfaceMonitorImplWindows::StartMonitor:%d# FAILED to register for addr change", __LINE__ );
    pimpl_->reset();
    ret_status = ErrCode::ERR;
  }

exit_label_:
  DBG_INFO( "InterfaceMonitorImplWindows::StartMonitor:%d# EXIT status %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

ErrCode InterfaceMonitorImplWindows::stop_monitor( uint16_t adapter_type )
{
  auto ret_status = ErrCode::OK;

  DBG_INFO( "InterfaceMonitorImplWindows::StopMonitor:%d# ENTER AdapterType %x", __LINE__, (int) adapter_type );

  pimpl_->unregister_for_addr_change();
  pimpl_->reset();

  DBG_INFO( "InterfaceMonitorImplWindows::StopMonitor:%d# EXIT %d", __LINE__, (int) ret_status );

  return ( ret_status );
}

std::vector<InterfaceAddress *> InterfaceMonitorImplWindows::get_interface_addr_list( bool skip_if_down )
{
  std::vector<InterfaceAddress *> if_ptr_array{};

  DBG_INFO( "InterfaceMonitorImplWindows::get_interface_addr_list:%d# ENTER skip_if_down %d", __LINE__, skip_if_down );

  ScopedMutex lock( pimpl_->access_mutex_ );

  DBG_INFO( "InterfaceMonitorImplWindows::get_interface_addr_list:%d# Current no of if address %d", __LINE__, static_cast<int>( pimpl_->curr_if_addr_ptr_list_.size() ) );

  for( auto &if_addr : pimpl_->curr_if_addr_ptr_list_ )
  {
    if( !( skip_if_down && ( ( if_addr->get_flags() & IFF_UP ) != IFF_UP ) ) )
    {
#ifdef _DEBUG_
      char ascii_interface_addr[64] = {0};
			inet_ntop( if_addr->get_family() == IpAddrFamily::IPv4 ? AF_INET : AF_INET6,
        (void *) ( if_addr->get_addr() ),
        &ascii_interface_addr[0], sizeof( ascii_interface_addr ) );
      DBG_INFO(
        "InterfaceMonitorImplWindows::get_interface_addr_list:%d# Adding if_addr idx[%d], family[%d], flags[%x], addr[%s]",
        __LINE__,
        if_addr->get_index(),
        int(if_addr->get_family() ),
        int(if_addr->get_flags() ),
        &ascii_interface_addr[0] );
#endif /*_DEBUG_*/

      if_ptr_array.push_back( new InterfaceAddress{ *if_addr } );
    }
  }

  DBG_INFO( "InterfaceMonitorImplWindows::get_interface_addr_list:%d# EXIT", __LINE__ );

  return ( if_ptr_array );
}

std::vector<InterfaceAddress *> InterfaceMonitorImplWindows::get_newly_found_interface()
{
  DBG_INFO( "InterfaceMonitorImplWindows::get_newly_found_interface:%d# ENTER", __LINE__ );

  pimpl_->access_mutex_->Lock();

  std::vector<InterfaceAddress *> newly_found_if_addr_ptr_list{};

  pimpl_->is_old_if_addr_lost_ = false;

  DBG_INFO( "InterfaceMonitorImplWindows::get_newly_found_interface:%d# Newly found if_addr count[%d]", __LINE__, static_cast<int>( pimpl_->new_if_addr_ptr_list_.size() ) );
  newly_found_if_addr_ptr_list.swap( pimpl_->new_if_addr_ptr_list_ );
  pimpl_->new_if_addr_ptr_list_.clear();

  const auto is_new_if_addr_found{ pimpl_->new_if_addr_ptr_list_.size() > 0 };

  DBG_INFO( "InterfaceMonitorImplWindows::get_newly_found_interface:%d# old if_addr lost [%d], new if_addr_found[%d]", __LINE__, pimpl_->is_old_if_addr_lost_, is_new_if_addr_found );

  pimpl_->access_mutex_->Unlock();

  if( pimpl_->is_old_if_addr_lost_ )
  {
    DBG_INFO( "InterfaceMonitorImplWindows::get_newly_found_interface:%d# Notifying IF_DOWN", __LINE__ );
    pimpl_->notify_if_state_changed( InterfaceStatusFlag::DOWN );
  }

  if( is_new_if_addr_found )
  {
    DBG_INFO( "InterfaceMonitorImplWindows::get_newly_found_interface:%d# Notifying IF_UP", __LINE__ );
    pimpl_->notify_if_state_changed( InterfaceStatusFlag::UP );
  }

  DBG_INFO( "InterfaceMonitorImplWindows::get_newly_found_interface:%d# EXIT", __LINE__ );

  return ( newly_found_if_addr_ptr_list );
}

void InterfaceMonitorImplWindows::add_interface_event_callback( pfn_interface_monitor_cb cz_if_monitor_cb, void *pv_user_data )
{
  if( cz_if_monitor_cb )
  {
    pimpl_->_if_monitor_clients_list.push_front( new InterfaceMonitorCbData{ cz_if_monitor_cb, pv_user_data } );
  }
}

void InterfaceMonitorImplWindows::remove_interface_event_callback( pfn_interface_monitor_cb cz_if_monitor_cb )
{
  if( cz_if_monitor_cb )
  {
    for( auto it = pimpl_->_if_monitor_clients_list.begin(); it != pimpl_->_if_monitor_clients_list.end(); ++it )
    {
      if( ( *it )->cz_if_monitor_cb == cz_if_monitor_cb )
      {
        delete ( *it );
        break;
      }
    }
  }
}

void InterfaceMonitorImplWindowsData::notify_if_state_changed( const InterfaceStatusFlag interface_status )
{
  if( !_if_monitor_clients_list.empty() )
  {
    InterfaceEvent interface_event{ InterfaceEventType::kInterfaceStateChanged };
    interface_event.set_status_flag( interface_status );

    for( auto &callback : _if_monitor_clients_list )
    {
      if( callback->cz_if_monitor_cb )
      {
        DBG_INFO( "InterfaceMonitorImplWindowsData::notify_if_state_changed:%d# Calling the HandleEvente", __LINE__ );
        callback->cz_if_monitor_cb( &interface_event, callback->pv_user_data );
      }
    }
  }
}

void InterfaceMonitorImplWindowsData::notify_if_modified()
{
  if( !_if_monitor_clients_list.empty() )
  {
    InterfaceEvent interface_event{ InterfaceEventType::kInterfaceModified };
    interface_event.set_adapter_type( k_adapter_type_ip );

    for( auto &callback : _if_monitor_clients_list )
    {
      if( callback->cz_if_monitor_cb )
      {
        DBG_INFO( "InterfaceMonitorImplWindowsData::notify_if_modified:%d# Calling the HandleEvente", __LINE__ );
        callback->cz_if_monitor_cb( &interface_event, callback->pv_user_data );
      }
    }
  }
}

std::unique_ptr<IP_ADAPTER_ADDRESSES> InterfaceMonitorImplWindowsData::get_adapters() const
{
  ULONG                 u32_adapter_addr_buf_len = 0;
  IP_ADAPTER_ADDRESSES *p_adapter_adrr_start_ptr = nullptr;

  /*We don't need most of the default information, so optimize this call by not asking for them. */
  const ULONG           flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME;

  DBG_INFO( "InterfaceMonitorImplWindowsData::get_adapters:%d# ENTER", __LINE__ );

  /*
   *   Call up to 3 times: once to get the size, once to get the msg_data, and once more
   *   just in case there was an increase in length in between the first two. If the
   *   length is still increasing due to more addresses being added, even this may fail
   *   and we'll have to wait for the next IP address change notification.
   */
  for( int i = 0; i < 3; i++ )
  {
    DBG_INFO( "InterfaceMonitorImplWindowsData::get_adapters:%d# calling GetAdaptersAddresses [%d] times", __LINE__, i );
    const auto ret = GetAdaptersAddresses( AF_UNSPEC, flags, nullptr, p_adapter_adrr_start_ptr, &u32_adapter_addr_buf_len );

    if( ERROR_BUFFER_OVERFLOW == ret )
    {
      // Redo with updated length.
      if( p_adapter_adrr_start_ptr != nullptr )
      {
        delete[] p_adapter_adrr_start_ptr;
      }

      p_adapter_adrr_start_ptr = reinterpret_cast<IP_ADAPTER_ADDRESSES *>( new unsigned char[u32_adapter_addr_buf_len] );

      if( p_adapter_adrr_start_ptr == nullptr )
      {
        DBG_ERROR( "InterfaceMonitorImplWindowsData::get_adapters:%d# FAILED to alloc memory", __LINE__ );
        break;
      }

      continue;
    }

    if( NO_ERROR != ret )
    {
      break;
    }

    DBG_INFO( "InterfaceMonitorImplWindowsData::get_adapters:%d# EXIT SUCCESS", __LINE__ );
    // Succeeded getting adapters
    return ( std::unique_ptr<IP_ADAPTER_ADDRESSES>( p_adapter_adrr_start_ptr ) /*p_adapter_adrr_start_ptr*/ );
  }

  if( p_adapter_adrr_start_ptr != nullptr )
  {
    delete[] p_adapter_adrr_start_ptr;
  }

  DBG_INFO( "InterfaceMonitorImplWindowsData::get_adapters:%d# EXIT FAILED", __LINE__ );

  return ( nullptr );
}

bool InterfaceMonitorImplWindowsData::is_valid_addr( IP_ADAPTER_UNICAST_ADDRESS *p_unicast_adapter_addr ) const
{
  if( p_unicast_adapter_addr->Address.lpSockaddr->sa_family != AF_INET6 )
  {
    // All IPv4 addresses are valid.
    return ( true );
  }

    const auto ipv6_socket_addr = reinterpret_cast<struct sockaddr_in6 *>( p_unicast_adapter_addr->Address.lpSockaddr );

    // if the addr is link local then it it valid
    if( ( ipv6_socket_addr->sin6_addr.u.Byte[0] == 0xfe ) && ( ( ipv6_socket_addr->sin6_addr.u.Byte[1] & 0xc0 ) == 0x80 ) )
    {
      return ( true );
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


inline std::vector<InterfaceAddress *> InterfaceMonitorImplWindowsData::get_if_addr_list_for_index( const uint16_t index ) const
{
  DBG_INFO( "InterfaceMonitorImplWindowsData::get_if_addr_list_for_index:%d# ENTER idx[%d]", __LINE__, index );
  std::vector<InterfaceAddress *> if_addr_ptr_list{};

  const auto p_adapters_list = get_adapters();

  if( !p_adapters_list )
  {
    DBG_ERROR( "InterfaceMonitorImplWindowsData::get_if_addr_list_for_index:%d# FAILED to get adapters list", __LINE__ );
    goto exit_label_;
  }

  for( IP_ADAPTER_ADDRESSES *p_cur_adapter_addr = p_adapters_list.get(); p_cur_adapter_addr != nullptr; p_cur_adapter_addr
      = p_cur_adapter_addr->Next )
  {
    DBG_INFO( "InterfaceMonitorImplWindowsData::get_if_addr_list_for_index:%d# adapter index[%d], if_type[%d], if_status[%d]"
            , __LINE__, uint32_t( p_cur_adapter_addr->IfIndex ), uint32_t( p_cur_adapter_addr->IfType ), p_cur_adapter_addr->
      OperStatus );

    if( ( ( index > 0 ) && ( p_cur_adapter_addr->IfIndex != index ) )
      || ( p_cur_adapter_addr->IfType & IF_TYPE_SOFTWARE_LOOPBACK )
      || ( ( p_cur_adapter_addr->OperStatus & IfOperStatusUp ) == 0 ) )
    {
      continue;
    }

    for( auto p_unicast_adapter_addr = p_cur_adapter_addr->FirstUnicastAddress;
      p_unicast_adapter_addr != nullptr;
      p_unicast_adapter_addr = p_unicast_adapter_addr->Next )
    {
      char temp_addr_buf[64] = { 0 };
      auto addr_family       = (p_unicast_adapter_addr->Address.lpSockaddr->sa_family == AF_INET) ? IpAddrFamily::IPv4 : IpAddrFamily::IPv6;
			InterfaceAddress *if_addr = nullptr;

      if( addr_family == IpAddrFamily::IPv4)
      {
        auto p_ipv4_addr = reinterpret_cast<struct sockaddr_in *>( p_unicast_adapter_addr->Address.lpSockaddr );
				if_addr = new InterfaceAddress{ (uint32_t)p_cur_adapter_addr->IfIndex, IFF_UP, addr_family, (const char *)&p_ipv4_addr->sin_addr };
        inet_ntop( AF_INET, static_cast<void *>( &p_ipv4_addr->sin_addr ), (char*)&temp_addr_buf[0], sizeof( temp_addr_buf ) );
      }
      else if( addr_family == IpAddrFamily::IPv6)
      {
        auto p_ipv6_addr = reinterpret_cast<struct sockaddr_in6 *>( p_unicast_adapter_addr->Address.lpSockaddr );
				if_addr = new InterfaceAddress{ (uint32_t)p_cur_adapter_addr->IfIndex, IFF_UP, addr_family, (const char *)&p_ipv6_addr->sin6_addr };
        inet_ntop( AF_INET6, static_cast<void *>( &p_ipv6_addr->sin6_addr ), (char*)&temp_addr_buf[0], sizeof( temp_addr_buf ) );
      }

      const auto is_addr_valid = is_valid_addr( p_unicast_adapter_addr );

      DBG_INFO( "InterfaceMonitorImplWindowsData::get_if_addr_list_for_index:%d# Found address %s, valid[%d]", __LINE__, &temp_addr_buf[0], is_addr_valid );

      if( !is_addr_valid )
      {
        continue;
      }
      if_addr_ptr_list.push_back(if_addr);
    }
  }

exit_label_:
  DBG_INFO( "InterfaceMonitorImplWindowsData::get_if_addr_list_for_index:%d# EXIT", __LINE__ );

  return ( if_addr_ptr_list );
}

inline bool InterfaceMonitorImplWindowsData::register_for_addr_change()
{
  auto ret_status = true;

  DBG_INFO( "InterfaceMonitorImplWindowsData::register_for_addr_change:%d# ENTER", __LINE__ );

  shutdown_event_ = CreateEvent( nullptr, TRUE, FALSE, nullptr );

  if( shutdown_event_ == nullptr )
  {
    DBG_ERROR( "InterfaceMonitorImplWindowsData::register_for_addr_change:%d# FAILED to create shutdown event",
      __LINE__ );
    ret_status = false;
    goto exit_label_;
  }

  monitor_thread_handle_ = CreateThread( nullptr, 0, InterfaceMonitorThread, this, 0, nullptr );

  if( monitor_thread_handle_ == nullptr )
  {
    CloseHandle( shutdown_event_ );
    shutdown_event_ = nullptr;
    DBG_ERROR( "InterfaceMonitorImplWindowsData::register_for_addr_change:%d# FAILED to create monitor thread",
      __LINE__ );
    ret_status = false;
    goto exit_label_;
  }

  handle_interface_addr_change();

exit_label_:
  DBG_INFO( "InterfaceMonitorImplWindowsData::register_for_addr_change:%d# EXIT status %d", __LINE__, !ret_status );
  return ( true );
}

inline bool InterfaceMonitorImplWindowsData::unregister_for_addr_change()
{
  DBG_INFO( "InterfaceMonitorImplWindowsData::unregister_for_addr_change:%d# ENTER", __LINE__ );

  if( shutdown_event_ != nullptr )
  {
    DBG_INFO( "InterfaceMonitorImplWindowsData::unregister_for_addr_change:%d# closing shutdown event", __LINE__ );
    SetEvent( shutdown_event_ );
    WaitForSingleObject( monitor_thread_handle_, INFINITE );
    CloseHandle( shutdown_event_ );
    shutdown_event_ = nullptr;
  }

  if( monitor_thread_handle_ != nullptr )
  {
    DBG_INFO( "InterfaceMonitorImplWindowsData::unregister_for_addr_change:%d# stopping monitor thread", __LINE__ );
    CloseHandle( monitor_thread_handle_ );
    monitor_thread_handle_ = nullptr;
  }

  DBG_INFO( "InterfaceMonitorImplWindowsData::unregister_for_addr_change:%d# EXIT", __LINE__ );

  return ( true );
}

inline void InterfaceMonitorImplWindowsData::reset()
{
  DBG_INFO( "InterfaceMonitorImplWindowsData::reset:%d# ENTER", __LINE__ );

  for( auto &if_addr : new_if_addr_ptr_list_ )
  {
    delete if_addr;
  }

  new_if_addr_ptr_list_.clear();

  for( auto &if_addr : curr_if_addr_ptr_list_ )
  {
    delete if_addr;
  }

  curr_if_addr_ptr_list_.clear();

  if( access_mutex_ != nullptr )
  {
    OsalMgr::Inst()->FreeMutex( access_mutex_ );
    access_mutex_ = nullptr;
  }

  DBG_INFO( "enclosing_method:%d# EXIT", __LINE__ );
}

inline void InterfaceMonitorImplWindowsData::run_if_monitor_thread()
{
  {
    OVERLAPPED overlapped = { 0 };
    WSADATA wsa_data{};

    if( WSAStartup( MAKEWORD( 2, 2 ), &wsa_data ) != NO_ERROR )
    {
      return;
    }

    const auto nwm_socket = WSASocketW( AF_INET6, SOCK_DGRAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED );

    if( INVALID_SOCKET == nwm_socket )
    {
      WSACleanup();

      return;
    }

    // Put socket into dual IPv4/IPv6 mode.
    auto ipv6_only = FALSE;

    if( SOCKET_ERROR == setsockopt( nwm_socket, IPPROTO_IPV6, IPV6_V6ONLY, reinterpret_cast<char *>( &ipv6_only ),
      sizeof ipv6_only ) )
    {
      closesocket( nwm_socket );
      WSACleanup();

      return;
    }

    overlapped.hEvent = CreateEvent( nullptr, TRUE, FALSE, nullptr );

    if( nullptr == overlapped.hEvent )
    {
      closesocket( nwm_socket );

      WSACleanup();
      return;
    }

    WSAEVENT eventList[2]  = { overlapped.hEvent, shutdown_event_, };
    DWORD    bytesReturned = 0;

    for(;; )
    {
      if( SOCKET_ERROR == WSAIoctl( nwm_socket, SIO_ADDRESS_LIST_CHANGE, nullptr, 0, nullptr, 0, &bytesReturned,
        &overlapped, nullptr ) )
      {
        const auto err = WSAGetLastError();

        if( err != ERROR_IO_PENDING )
        {
          break;
        }

        // Wait for an address change or a request to cancel the thread.
        const auto wait_status = WSAWaitForMultipleEvents(
          _countof( eventList ), eventList, FALSE, WSA_INFINITE, FALSE );

        if( wait_status != WSA_WAIT_EVENT_0 )
        {
          // The cancel event was signaled.  There is no need to call CancelIo
          // here, because we will close the socket handle below, causing any
          // pending I/O to be canceled then.
          break;
        }

        WSAResetEvent( overlapped.hEvent );
      }

      // We have a change to process.  The address change callback ignores the parameters, so we just pass default values.
      handle_interface_addr_change();
    }

    closesocket( nwm_socket );
    CloseHandle( overlapped.hEvent );
    WSACleanup();
  }
}

static DWORD WINAPI InterfaceMonitorThread( const PVOID context )
{
  auto impl_data = static_cast<InterfaceMonitorImplWindowsData *>( context );

  if( impl_data != nullptr )
  {
    impl_data->run_if_monitor_thread();
  }

  return ( 1 );
}

inline void InterfaceMonitorImplWindowsData::handle_interface_addr_change()
{
  DBG_INFO( "InterfaceMonitorImplWindowsData::handle_interface_addr_change:%d# ENTER", __LINE__ );
  ScopedMutex lock( access_mutex_ );

  std::vector<InterfaceAddress *> temp_old_if_addr_list{};

  /* copy the old interface addresses to temp */
  temp_old_if_addr_list.swap( curr_if_addr_ptr_list_ );
  curr_if_addr_ptr_list_.clear();     // clear it to get new addresses

  curr_if_addr_ptr_list_ = get_if_addr_list_for_index( 0 );

  // old - 5 6 7 8 9
  // new - 5 6 7 8 9 - case 1
  // new - 2 6 7 8 9 - case 2

  is_old_if_addr_lost_ = false;

  // check for the old address mising
  for( auto it = temp_old_if_addr_list.cbegin(); it != temp_old_if_addr_list.cend(); ++it )
  {
    is_old_if_addr_lost_ = true;

    for( auto it2 = curr_if_addr_ptr_list_.cbegin(); it2 != curr_if_addr_ptr_list_.cend(); ++it2 )
    {
      if( *it == *it2 )
      {
        is_old_if_addr_lost_ = false;
        break;
      }
    }

    if( is_old_if_addr_lost_ )
    {
      break;
    }
  }

  // new - 5 6 7 8
  // old -          - case 1
  // old - 1 2 3	  - case 2
  // old - 5 6 7 8  - case 3
  // check for any new address addition

  for( auto it2 = curr_if_addr_ptr_list_.cbegin(); it2 != curr_if_addr_ptr_list_.cend(); ++it2 )
  {
    bool found{ false };

    for( auto it = temp_old_if_addr_list.cbegin(); it != temp_old_if_addr_list.cend(); ++it )
    {
      if( *it == *it2 )
      {
        found = true;
        break;
      }
    }

    if( !found )
    {
      auto newIfAddr = new InterfaceAddress( **it2 );
      new_if_addr_ptr_list_.push_back( newIfAddr );
    }
  }

  if( !new_if_addr_ptr_list_.empty() )
  {
    notify_if_modified();
  }

  for( auto &if_addr : temp_old_if_addr_list )
  {
    delete if_addr;
  }

  temp_old_if_addr_list.clear();

  DBG_INFO( "InterfaceMonitorImplWindowsData::handle_interface_addr_change:%d# EXIT", __LINE__ );
}
}
}

#endif// #ifdef _OS_WINDOWS_
