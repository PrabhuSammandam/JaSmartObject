/*
 * Credential.h
 *
 *  Created on: Nov 19, 2018
 *      Author: psammand
 */

#pragma once
#include <vector>
#include <string>
#include <cstdint>
#include "Uuid.h"
#include "cbor/CborEncoder.h"
#include "ResPropValue.h"
#include "security/SecurityDataTypes.h"
namespace ja_iot {
namespace resources {
using namespace base;
using namespace stack;
class CredentialRole
{
  public:
    CredentialRole () {}

    void encode_to_cbor( CborEncoder &cz_cbor_encoder );

    void       set_authority( const std::string &autority ) { _authority = autority; }
    std::string& get_authority() { return ( _authority ); }
    void       set_role( const std::string &role ) { _role = role; }
    std::string& get_role() { return ( _role ); }

  private:
    std::string   _authority;
    std::string   _role;
};

enum class CredentialDataType : uint8_t
{
  PUBLIC,
  PRIVATE,
  OPTIONAL
};

class BaseCredentialData
{
  public:
    BaseCredentialData( CredentialDataType type ) : _type{ type } {}
    virtual ~BaseCredentialData () {}

    virtual void encode_to_cbor( CborEncoder &cz_cbor_encoder ) { (void) cz_cbor_encoder; }
    virtual void decode_from_cbor( ResRepresentation &cbor_rep );

    void encode_encoding_to_cbor( CborEncoder &cz_cbor_encoder );
    void decode_encoding_from_cbor( ResRepresentation &cbor_rep );

  protected:
    CredentialDataType   _type;
    std::string          _data;
    uint8_t              _encoding = CREDENTIAL_ENCODING_TYPE_JWT;
};

class CredentialPublicData : public BaseCredentialData
{
  public:
    CredentialPublicData () : BaseCredentialData{ CredentialDataType::PUBLIC } {}
    void encode_to_cbor( CborEncoder &cz_cbor_encoder ) override;
};

class CredentialPrivateData : public BaseCredentialData
{
  public:
    CredentialPrivateData () : BaseCredentialData{ CredentialDataType::PRIVATE } {}
    void encode_to_cbor( CborEncoder &cz_cbor_encoder ) override;
    void decode_from_cbor( ResRepresentation &cbor_rep ) override;

  private:
    uint16_t   _handle = 0;
};

class CredentialOptionalData : public BaseCredentialData
{
  public:
    CredentialOptionalData () : BaseCredentialData{ CredentialDataType::OPTIONAL } {}
    void encode_to_cbor( CborEncoder &cz_cbor_encoder ) override;
    void decode_from_cbor( ResRepresentation &cbor_rep ) override;

  private:
    bool   _revocation_status = false;
};

class Credential
{
  public:
    Credential( uint16_t cred_id ) : _id{ cred_id } {}
    ~Credential ();

    uint8_t  decode_from_cbor( ResRepresentation &cred_rep_obj );
    void     encode_to_cbor( CborEncoder &cz_cbor_encoder );
    void     encode_cred_usage_to_cbor( CborEncoder &cz_cbor_encoder );
    void     decode_cred_usage_from_cbor( ResRepresentation &cbor_rep );
    Uuid     & get_subject_uuid() { return ( _subject_uuid ); }
    uint16_t get_id() { return ( _id ); }
    bool     is_same_subject( const Uuid &subject_uuid ) { return ( _subject_uuid == subject_uuid ); }

  private:
    uint16_t               _id;
    Uuid                   _subject_uuid;
    CredentialRole *       _role          = nullptr;
    uint8_t                _type          = CREDENTIAL_TYPE_NONE;
    uint8_t                _usage_type    = CREDENTIAL_USAGE_TYPE_NONE;
    BaseCredentialData *   _public_data   = nullptr;
    BaseCredentialData *   _private_data  = nullptr;
    BaseCredentialData *   _optional_data = nullptr;
    std::string            _period;
    std::vector<uint8_t>   _credential_refresh_method_array;
};
}
}
