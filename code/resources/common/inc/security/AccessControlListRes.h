/*
 * AccessControlListRes.h
 *
 *  Created on: Nov 20, 2018
 *      Author: psammand
 */

#pragma once
#include <vector>
#include "BaseResource.h"
#include "Uuid.h"
#include "security/AccessControlEntry.h"

namespace ja_iot {
namespace resources {
using namespace stack;
using namespace base;

class Acl2Object
{
  public:
    Acl2Object () {}
    ~Acl2Object ();
    void    encode_to_cbor( CborEncoder &cz_cbor_encoder );
    void    encode_to_cbor( CborEncoder &cz_cbor_encoder, std::vector<int> &aceid_list );
    uint8_t remove_ace( int ace_id );
    uint8_t remove_all_ace();

    AccessControlList& get_ace_list() { return ( _ace_list ); }
    Uuid             & get_resource_owner_id() { return ( _resource_owner_uuid ); }
    void             set_resource_owner_id( const Uuid &uuid ) { _resource_owner_uuid = uuid; }

  private:
    AccessControlList   _ace_list;
    Uuid                _resource_owner_uuid;
};

class AccessControlListRes : public BaseResource
{
  public:
    AccessControlListRes ();

    bool is_method_supported( uint8_t method ) override;

    AccessControlList find_ace_by_connection( uint8_t connection_type );
    AccessControlList& get_access_control_list() { return ( _acl_obj.get_ace_list() ); }
    Uuid             & get_resource_owner_uuid() { return ( _acl_obj.get_resource_owner_id() ); }
    void             set_resource_owner_uuid( const Uuid &uuid );
    uint16_t         count() { return ( (uint16_t) _acl_obj.get_ace_list().size() ); }
		bool             is_allowed(const uint8_t connection_type, IResource* resource, uint8_t method);
		bool             is_allowed(const Uuid &subject_uuid, IResource* resource, uint8_t method);
		bool             is_allowed( const uint8_t connection_type, const std::string &href, uint8_t permission, const bool is_discoverable );
    bool             is_allowed( const Uuid &subject_uuid, const std::string &href, uint8_t permission, const bool is_discoverable );

  private:
    uint8_t handle_get( QueryContainer &query_container, Interaction *interaction ) override;
    uint8_t handle_post( QueryContainer &query_container, Interaction *interaction ) override;
    uint8_t handle_delete( QueryContainer &query_container, Interaction *interaction ) override;

    uint8_t handle_post_intl( QueryContainer &query_container, Interaction *interaction );
    uint8_t handle_delete_intl( QueryContainer &query_container, Interaction *interaction );

    void init();

    Acl2Object   _acl_obj;
};
}
}