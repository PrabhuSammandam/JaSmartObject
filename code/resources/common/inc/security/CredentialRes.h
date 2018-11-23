/*
 * CredentialRes.h
 *
 *  Created on: Nov 19, 2018
 *      Author: psammand
 */

#pragma once
#include <vector>
#include "BaseResource.h"
#include "Uuid.h"
#include "Credential.h"

namespace ja_iot {
namespace resources {
using namespace stack;
using namespace base;

class CredObject
{
  public:
    CredObject () {}
    ~CredObject ();
    void encode_to_cbor( CborEncoder &cz_cbor_encoder );
    void encode_to_cbor( CborEncoder &cz_cbor_encoder, std::vector<int> &aceid_list );

    Uuid & get_resource_owner_uuid() { return ( _resource_owner_uuid ); }
    void set_resource_owner_uuid( const Uuid &resource_owner_uuid ) { _resource_owner_uuid = resource_owner_uuid; }

    std::vector<Credential *>& get_credentials_array() { return ( _credentials_array ); }
    uint8_t                  remove_credential( std::vector<int> &cred_id_list );
    uint8_t                  remove_credential( const std::string &subject );
    uint8_t                  remove_credential( int credid );
    uint8_t                  remove_all_credential();

  private:
    std::vector<Credential *>   _credentials_array;
    Uuid                        _resource_owner_uuid;
};

class CredentialRes : public BaseResource
{
  public:

  public:
    CredentialRes ();

    bool is_method_supported( uint8_t method ) override;

    Uuid & get_resource_owner_uuid() { return ( _cred_obj.get_resource_owner_uuid() ); }
    void set_resource_owner_uuid( const Uuid &resource_owner_uuid ) { _cred_obj.set_resource_owner_uuid( resource_owner_uuid ); }

  private:
    uint8_t handle_get( QueryContainer &query_container, Interaction *interaction ) override;
    uint8_t handle_post( QueryContainer &query_container, Interaction *interaction ) override;
    uint8_t handle_post_intl( QueryContainer &query_container, Interaction *interaction );
    uint8_t handle_delete( QueryContainer &query_container, Interaction *interaction ) override;
    uint8_t handle_delete_intl( QueryContainer &query_container, Interaction *interaction );
    void    init();

    CredObject   _cred_obj;
};
}
}
