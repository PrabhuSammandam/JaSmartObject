/*
 * InterfaceMonitorImplLinux.cpp
 *
 *  Created on: 17-Sep-2017
 *      Author: prabhu
 */

#ifdef _OS_LINUX_
#include <port/linux/inc/InterfaceMonitorImplLinux.h>
#include <OsalMgr.h>
#include <Mutex.h>
#include <ScopedMutex.h>
#include <SimpleList.h>
#include <common/inc/logging_network.h>

using namespace ja_iot::base;
using namespace ja_iot::osal;

namespace ja_iot {
namespace network {

using InterfaceAddressList = SimpleList<InterfaceAddress, MAX_NO_OF_INTERFACE_ADDRESS>;
using InterfaceAddressPtrList = StaticPtrArray<InterfaceAddress *, MAX_NO_OF_INTERFACE_ADDRESS>;
using InterfaceEventHandlerPtrArray = StaticPtrArray<IInterfaceEventHandler *, MAX_NO_OF_INTERFACE_EVENT_HANDLER>;

class InterfaceMonitorImplLinuxData {
public:
	InterfaceMonitorImplLinuxData(InterfaceMonitorImplLinux* host) :
			host_ { host } {
	}

	void get_if_addr_list_for_index(uint16_t index,
			PtrArray<InterfaceAddress *> &if_addr_ptr_list);
	bool register_for_addr_change();
	bool unregister_for_addr_change();
	void reset();
	void run_if_monitor_thread();
	void handle_interface_addr_change();
	void notify_if_modified();
	void notify_if_state_changed(InterfaceStatusFlag interface_status);

public:
	InterfaceMonitorImplLinux* host_;
	Mutex* access_mutex_ = nullptr;
	bool is_old_if_addr_lost_ = false;
	InterfaceAddressPtrList curr_if_addr_ptr_list_;
	InterfaceAddressPtrList new_if_addr_ptr_list_;
	InterfaceAddressList if_addr_store_;
	InterfaceEventHandlerPtrArray if_event_handler_ptr_array_;

};

InterfaceMonitorImplLinux::InterfaceMonitorImplLinux() {
	this->pimpl_ = new InterfaceMonitorImplLinuxData { this };
}

ErrCode InterfaceMonitorImplLinux::StartMonitor(AdapterType adapter_type) {
	ErrCode ret_status = ErrCode::OK;

	DBG_INFO( "InterfaceMonitorImplLinux::StartMonitor:%d# ENTER adapter_type[%x]", __LINE__, (int) adapter_type );

	pimpl_->access_mutex_ = OsalMgr::Inst()->AllocMutex();

	if (pimpl_->access_mutex_ == nullptr) {
		DBG_ERROR( "InterfaceMonitorImplLinux::StartMonitor:%d# AllocMutex FAILED", __LINE__ );
		ret_status = ErrCode::ERR;
		goto exit_label_;
	}

	pimpl_->curr_if_addr_ptr_list_.Clear();

	if (!pimpl_->register_for_addr_change()) {
		DBG_ERROR( "InterfaceMonitorImplLinux::StartMonitor:%d# FAILED to register for addr change", __LINE__ );
		pimpl_->reset();
		ret_status = ErrCode::ERR;
		goto exit_label_;
	}

	exit_label_: DBG_INFO( "InterfaceMonitorImplLinux::StartMonitor:%d# EXIT status %d", __LINE__, (int)ret_status );

	return (ret_status);
}

ErrCode InterfaceMonitorImplLinux::StopMonitor(AdapterType adapter_type) {
	ErrCode ret_status = ErrCode::OK;

	DBG_INFO( "InterfaceMonitorImplLinux::StopMonitor:%d# ENTER AdapterType %x", __LINE__, (int) adapter_type );

	pimpl_->unregister_for_addr_change();
	pimpl_->reset();

	DBG_INFO( "InterfaceMonitorImplLinux::StopMonitor:%d# EXIT %d", __LINE__, (int)ret_status );

	return (ret_status);
}

ErrCode InterfaceMonitorImplLinux::GetInterfaceAddrList(
		InterfaceAddressPtrArray& if_ptr_array, bool skip_if_down) {
	ErrCode ret_status = ErrCode::OK;

	DBG_INFO( "InterfaceMonitorImplLinux::GetInterfaceAddrList:%d# ENTER skip_if_down %d", __LINE__, skip_if_down );

	ScopedMutex lock(pimpl_->access_mutex_);

	DBG_INFO( "InterfaceMonitorImplLinux::GetInterfaceAddrList:%d# Current no of if address %d", __LINE__, pimpl_->curr_if_addr_ptr_list_.Count() );

	if (pimpl_->curr_if_addr_ptr_list_.Count() > 0) {
		for (int i = 0; i < pimpl_->curr_if_addr_ptr_list_.Count(); i++) {
			auto if_addr = pimpl_->curr_if_addr_ptr_list_.GetItem(i);

			if (if_addr != nullptr) {
				if (skip_if_down
						&& ((if_addr->getFlags() & IFF_UP) != IFF_UP)) {
					continue;
				}

				DBG_INFO( "InterfaceMonitorImplLinux::GetInterfaceAddrList:%d# Adding if_addr idx[%d], family[%d], flags[%x], addr[%s]", __LINE__, if_addr->getIndex(), (int)if_addr->getFamily(), (int)if_addr->getFlags(), if_addr->get_addr() );
				auto newIfAddr = new InterfaceAddress(*if_addr);
				if_ptr_array.Add(newIfAddr);
			}
		}
	}

	DBG_INFO( "InterfaceMonitorImplLinux::GetInterfaceAddrList:%d# EXIT status %d", __LINE__, (int)ret_status );

	return (ret_status);

}

ErrCode InterfaceMonitorImplLinux::GetNewlyFoundInterface(
		InterfaceAddressPtrArray& newly_found_if_addr_ptr_list) {
	ErrCode ret_status = ErrCode::OK;

	DBG_INFO( "InterfaceMonitorImplLinux::GetNewlyFoundInterface:%d# ENTER", __LINE__ );

	pimpl_->access_mutex_->Lock();

	pimpl_->is_old_if_addr_lost_ = false;

	DBG_INFO( "InterfaceMonitorImplLinux::GetNewlyFoundInterface:%d# Newly found if_addr count[%d]", __LINE__, pimpl_->new_if_addr_ptr_list_.Count() );

	if (pimpl_->new_if_addr_ptr_list_.Count() > 0) {
		for (int i = 0; i < pimpl_->new_if_addr_ptr_list_.Count(); i++) {
			auto if_addr = pimpl_->new_if_addr_ptr_list_.GetItem(i);

			if (if_addr != nullptr) {
				DBG_INFO( "InterfaceMonitorImplLinux::GetNewlyFoundInterface:%d# Adding if_addr idx[%d], family[%d], flags[%x]", __LINE__, if_addr->getIndex(), (int)if_addr->getFamily(), (int)if_addr->getFlags() );

				newly_found_if_addr_ptr_list.Add(
						new InterfaceAddress { *if_addr });

				/* free the interface address */
				pimpl_->if_addr_store_.Free(if_addr);
			}
		}
	}

	pimpl_->new_if_addr_ptr_list_.Clear();

	bool is_old_if_addr_lost { pimpl_->is_old_if_addr_lost_ };
	bool is_new_if_addr_found { pimpl_->new_if_addr_ptr_list_.Count() > 0 };

	DBG_INFO( "InterfaceMonitorImplLinux::GetNewlyFoundInterface:%d# old if_addr lost [%d], new if_addr_found[%d]", __LINE__, is_old_if_addr_lost, is_new_if_addr_found );

	pimpl_->access_mutex_->Unlock();

	if (is_old_if_addr_lost) {
		DBG_INFO( "InterfaceMonitorImplLinux::GetNewlyFoundInterface:%d# Notifying IF_DOWN", __LINE__ );
		pimpl_->notify_if_state_changed(InterfaceStatusFlag::DOWN);
	}

	if (is_new_if_addr_found) {
		DBG_INFO( "InterfaceMonitorImplLinux::GetNewlyFoundInterface:%d# Notifying IF_UP", __LINE__ );
		pimpl_->notify_if_state_changed(InterfaceStatusFlag::UP);
	}

	DBG_INFO( "InterfaceMonitorImplLinux::GetNewlyFoundInterface:%d# EXIT status %d", __LINE__, (int)ret_status );

	return (ret_status);

}

void InterfaceMonitorImplLinux::AddInterfaceEventHandler(
		IInterfaceEventHandler* interface_event_handler) {
	if (interface_event_handler != nullptr) {
		DBG_INFO( "InterfaceMonitorImplLinux::AddInterfaceEventHandler:%d# Added if_handler %p", __LINE__, interface_event_handler );
		pimpl_->if_event_handler_ptr_array_.Add(interface_event_handler);
	}

}

void InterfaceMonitorImplLinux::RemoveInterfaceEventHandler(
		IInterfaceEventHandler* interface_event_handler) {
	if (interface_event_handler != nullptr) {
		DBG_INFO( "InterfaceMonitorImplLinux::RemoveInterfaceEventHandler:%d# Removed if_handler %p", __LINE__, interface_event_handler );
		pimpl_->if_event_handler_ptr_array_.Remove(interface_event_handler);
	}
}

/*****************************************************************************/
/***********************InterfaceMonitorImplLinuxData*************************/
/*****************************************************************************/

void InterfaceMonitorImplLinuxData::get_if_addr_list_for_index(uint16_t index,
		PtrArray<InterfaceAddress*>& if_addr_ptr_list) {
}

bool InterfaceMonitorImplLinuxData::register_for_addr_change() {
}

bool InterfaceMonitorImplLinuxData::unregister_for_addr_change() {
}

void InterfaceMonitorImplLinuxData::reset() {
}

void InterfaceMonitorImplLinuxData::run_if_monitor_thread() {
}

void InterfaceMonitorImplLinuxData::handle_interface_addr_change() {
	DBG_INFO("InterfaceMonitorImplLinuxData::handle_interface_addr_change:%d# ENTER", __LINE__);
	InterfaceAddressPtrList temp_old_if_addr_list { };
	ScopedMutex lock(access_mutex_);

	if (curr_if_addr_ptr_list_.Count() > 0) {
		/* copy the old interface addresses to temp */
		for (int i = 0; i < curr_if_addr_ptr_list_.Count(); i++) {
			auto curr_if = curr_if_addr_ptr_list_.GetItem(i);

			if (curr_if == nullptr) {
				continue;
			}

			if (!temp_old_if_addr_list.Add(curr_if)) {
				break;
			}
		}
	}

	curr_if_addr_ptr_list_.Clear();   // clear it to get new addresses

	get_if_addr_list_for_index(0, curr_if_addr_ptr_list_);

	// old - 5 6 7 8 9
	// new - 5 6 7 8 9 - case 1
	// new - 2 6 7 8 9 - case 2

	is_old_if_addr_lost_ = false;

	// check for the old address mising
	if (temp_old_if_addr_list.Count() > 0) {
		for (int i = 0; i < temp_old_if_addr_list.Count(); i++) {
			auto old_if = temp_old_if_addr_list.GetItem(i);

			if (old_if == nullptr) {
				continue;
			}

			is_old_if_addr_lost_ = true;

			if (curr_if_addr_ptr_list_.Count() > 0) {
				for (int i = 0; i < curr_if_addr_ptr_list_.Count(); i++) {
					auto new_if = curr_if_addr_ptr_list_.GetItem(i);

					if (new_if == nullptr) {
						continue;
					}

					if (old_if == new_if) {
						is_old_if_addr_lost_ = false;
						break;
					}
				}
			}

			if (is_old_if_addr_lost_) {
				break;
			}
		}
	}

	// new - 5 6 7 8
	// old -          - case 1
	// old - 1 2 3	  - case 2
	// old - 5 6 7 8  - case 3
	// check for any new address addition
	if (curr_if_addr_ptr_list_.Count() > 0) {
		for (int i = 0; i < curr_if_addr_ptr_list_.Count(); i++) {
			auto new_if = curr_if_addr_ptr_list_.GetItem(i);

			if (new_if == nullptr) {
				continue;
			}

			bool found { false };

			if (temp_old_if_addr_list.Count() > 0) {
				for (int i = 0; i < temp_old_if_addr_list.Count(); i++) {
					auto oldAddr = temp_old_if_addr_list.GetItem(i);

					if (oldAddr == nullptr) {
						continue;
					}

					if (oldAddr == new_if) {
						found = true;
						break;
					}
				}
			}

			if (!found) {
				auto newIfAddr = if_addr_store_.Alloc();
				*newIfAddr = *new_if;

				new_if_addr_ptr_list_.Add(newIfAddr);
			}
		}
	}

	if (new_if_addr_ptr_list_.Count() > 0) {
		notify_if_modified();
	}

	if (temp_old_if_addr_list.Count() > 0) {
		for (int i = 0; i < temp_old_if_addr_list.Count(); i++) {
			auto oldAddr = temp_old_if_addr_list.GetItem(i);

			if (oldAddr == nullptr) {
				continue;
			}

			if_addr_store_.Free(oldAddr);
		}
	}DBG_INFO("InterfaceMonitorImplLinuxData::handle_interface_addr_change:%d# EXIT", __LINE__);

}

void InterfaceMonitorImplLinuxData::notify_if_modified() {
	if (if_event_handler_ptr_array_.Count() > 0) {
		InterfaceEvent interface_event { InterfaceEventType::kInterfaceModified };
		interface_event.setAdapterType(AdapterType::IP);

		for (int i = 0; i < if_event_handler_ptr_array_.Count(); i++) {
			auto if_modified_handler = if_event_handler_ptr_array_.GetItem(i);

			if (if_modified_handler != nullptr) {
				DBG_INFO( "InterfaceMonitorImplLinuxData::notify_if_modified:%d# Calling the HandleEvent for handler[%p]", __LINE__, if_modified_handler );
				if_modified_handler->HandleInterfaceEvent(&interface_event);
			}
		}
	}
}

void InterfaceMonitorImplLinuxData::notify_if_state_changed(
		InterfaceStatusFlag interface_status) {
	if (if_event_handler_ptr_array_.Count() > 0) {
		InterfaceEvent interface_event {
				InterfaceEventType::kInterfaceStateChanged };
		interface_event.setInterfaceStatusFlag(interface_status);

		for (int i = 0; i < if_event_handler_ptr_array_.Count(); i++) {
			auto if_modified_handler = if_event_handler_ptr_array_.GetItem(i);

			if (if_modified_handler != nullptr) {
				DBG_INFO( "InterfaceMonitorImplLinuxData::notify_if_state_changed:%d# Calling the HandleEvent for handler[%p]", __LINE__, if_modified_handler );
				if_modified_handler->HandleInterfaceEvent(&interface_event);
			}
		}
	}

}

}  // namespace network
}  // namespace ja_iot

#endif /* _OS_LINUX_ */
