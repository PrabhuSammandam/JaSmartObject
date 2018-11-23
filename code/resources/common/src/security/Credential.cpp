#include "common/inc/security/Credential.h"
#include "common/inc/security/SecurityDataTypes.h"
#include "ResPropValue.h"
#include "base_utils.h"

namespace ja_iot{
namespace resources {
using namespace stack;
using namespace base;

Credential::~Credential()
{
	delete_and_clear(_role);
	delete_and_clear(_public_data);
	delete_and_clear(_private_data);
	delete_and_clear(_optional_data);
}

uint8_t Credential::decode_from_cbor( ResRepresentation &cred_rep_obj )
{
  std::string temp_str;

  if( cred_rep_obj.get_prop( "subjectuuid", temp_str ) )
  {
    _subject_uuid << temp_str;
  }

  ResRepresentation temp_cbor_obj;

  if( cred_rep_obj.get_prop( "roleid", temp_cbor_obj ) )
  {
    auto cred_role = new CredentialRole{};

    temp_cbor_obj.get_prop( "role", cred_role->get_role() );
    temp_cbor_obj.get_prop( "authority", cred_role->get_authority() );
    temp_cbor_obj.~ResRepresentation ();
  }

  _type = (uint8_t)cred_rep_obj.get_prop<long>( "credtype", _type );
  decode_cred_usage_from_cbor( cred_rep_obj );

  if( cred_rep_obj.get_prop( "publicdata", temp_cbor_obj ) )
  {
    _public_data = new CredentialPublicData{};
    _public_data->decode_from_cbor( temp_cbor_obj );
    temp_cbor_obj.~ResRepresentation ();
  }

  if( cred_rep_obj.get_prop( "privatedata", temp_cbor_obj ) )
  {
    _private_data = new CredentialPublicData{};
    _private_data->decode_from_cbor( temp_cbor_obj );
    temp_cbor_obj.~ResRepresentation ();
  }

  if( cred_rep_obj.get_prop( "optionaldata", temp_cbor_obj ) )
  {
    _optional_data = new CredentialPublicData{};
    _optional_data->decode_from_cbor( temp_cbor_obj );
    temp_cbor_obj.~ResRepresentation ();
  }

  cred_rep_obj.get_prop( "period", _period );

  std::vector<long> crms;

  if( cred_rep_obj.get_prop( "crms", crms ) )
  {
    _credential_refresh_method_array.reserve( crms.size() );

    for( auto v : crms )
    {
      _credential_refresh_method_array.push_back( (uint8_t)v );
    }
  }

  return ( uint8_t() );
}

void Credential::encode_to_cbor( CborEncoder &cz_cbor_encoder )
{
  cz_cbor_encoder.start_map();
  cz_cbor_encoder.write_map_entry( "credid", _id );
  cz_cbor_encoder.write_map_entry( "subjectuuid", _subject_uuid.to_string() );

  if( _role != nullptr )
  {
    _role->encode_to_cbor( cz_cbor_encoder );
  }

  cz_cbor_encoder.write_map_entry( "credtype", _type );
  encode_cred_usage_to_cbor( cz_cbor_encoder );

  if( _public_data != nullptr )
  {
    _public_data->encode_to_cbor( cz_cbor_encoder );
  }

  if( _private_data != nullptr )
  {
    _private_data->encode_to_cbor( cz_cbor_encoder );
  }

  if( _optional_data != nullptr )
  {
    _optional_data->encode_to_cbor( cz_cbor_encoder );
  }

  cz_cbor_encoder.write_map_entry( "period", _period );
  cz_cbor_encoder.write_map_entry( "crms", _credential_refresh_method_array );
  cz_cbor_encoder.end_map();
}

void Credential::encode_cred_usage_to_cbor( CborEncoder &cz_cbor_encoder )
{
  if( _usage_type == CREDENTIAL_USAGE_TYPE_NONE )
  {
    return;
  }

  cz_cbor_encoder.write_string( "credusage" );

  switch( _usage_type )
  {
    case CREDENTIAL_USAGE_TYPE_TRUST_CA:
      cz_cbor_encoder.write_string( "oic.sec.cred.trustca" );
      break;
    case CREDENTIAL_USAGE_TYPE_CERT:
      cz_cbor_encoder.write_string( "oic.sec.cred.cert" );
      break;
    case CREDENTIAL_USAGE_TYPE_ROLE_CERT:
      cz_cbor_encoder.write_string( "oic.sec.cred.rolecert" );
      break;
    case CREDENTIAL_USAGE_TYPE_MFG_TRUST_CA:
      cz_cbor_encoder.write_string( "oic.sec.cred.mfgtrustca" );
      break;
    case CREDENTIAL_USAGE_TYPE_MFG_CERT:
      cz_cbor_encoder.write_string( "oic.sec.cred.mfgcert" );
      break;
  }
}

void Credential::decode_cred_usage_from_cbor( ResRepresentation &cbor_rep )
{
}

void CredentialRole::encode_to_cbor( CborEncoder &cz_cbor_encoder )
{
  if( _authority.empty() && _role.empty() )
  {
    return;
  }

  cz_cbor_encoder.write_string( "roleid" );
  cz_cbor_encoder.start_map();
  cz_cbor_encoder.write_map_entry( "authority", _authority );
  cz_cbor_encoder.write_map_entry( "role", _role );
  cz_cbor_encoder.end_map();
}

void BaseCredentialData::decode_from_cbor( ResRepresentation &cbor_rep )
{
	cbor_rep.get_prop("data", _data);
	decode_encoding_from_cbor(cbor_rep);
}

void BaseCredentialData::encode_encoding_to_cbor( CborEncoder &cz_cbor_encoder )
{
  cz_cbor_encoder.write_string( "encoding" );

  switch( _encoding )
  {
    case CREDENTIAL_ENCODING_TYPE_JWT:
      cz_cbor_encoder.write_string( "oic.sec.encoding.jwt" );
      break;
    case CREDENTIAL_ENCODING_TYPE_CWT:
      cz_cbor_encoder.write_string( "oic.sec.encoding.cwt" );
      break;
    case CREDENTIAL_ENCODING_TYPE_BASE64:
      cz_cbor_encoder.write_string( "oic.sec.encoding.base64" );
      break;
    case CREDENTIAL_ENCODING_TYPE_URI:
      cz_cbor_encoder.write_string( "oic.sec.encoding.uri" );
      break;
    case CREDENTIAL_ENCODING_TYPE_PEM:
      cz_cbor_encoder.write_string( "oic.sec.encoding.pem" );
      break;
    case CREDENTIAL_ENCODING_TYPE_DER:
      cz_cbor_encoder.write_string( "oic.sec.encoding.der" );
      break;
    case CREDENTIAL_ENCODING_TYPE_RAW:
      cz_cbor_encoder.write_string( "oic.sec.encoding.raw" );
      break;
    case CREDENTIAL_ENCODING_TYPE_HANDLE:
      cz_cbor_encoder.write_string( "oic.sec.encoding.handle" );
      break;
  }
}

void BaseCredentialData::decode_encoding_from_cbor(ResRepresentation & cbor_rep)
{
	std::string encoding;

	if (cbor_rep.get_prop("encoding", encoding))
	{
		if (encoding == "oic.sec.encoding.jwt")
		{
			_encoding = CREDENTIAL_ENCODING_TYPE_JWT;
		}
		else if (encoding == "oic.sec.encoding.cwt")
		{
			_encoding = CREDENTIAL_ENCODING_TYPE_CWT;
		}
		else if (encoding == "oic.sec.encoding.base64")
		{
			_encoding = CREDENTIAL_ENCODING_TYPE_BASE64;
		}
		else if (encoding == "oic.sec.encoding.uri")
		{
			_encoding = CREDENTIAL_ENCODING_TYPE_URI;
		}
		else if (encoding == "oic.sec.encoding.pem")
		{
			_encoding = CREDENTIAL_ENCODING_TYPE_PEM;
		}
		else if (encoding == "oic.sec.encoding.der")
		{
			_encoding = CREDENTIAL_ENCODING_TYPE_DER;
		}
		else if (encoding == "oic.sec.encoding.raw")
		{
			_encoding = CREDENTIAL_ENCODING_TYPE_RAW;
		}
		else if (encoding == "oic.sec.encoding.handle")
		{
			_encoding = CREDENTIAL_ENCODING_TYPE_HANDLE;
		}
	}
}

void CredentialPublicData::encode_to_cbor( CborEncoder &cz_cbor_encoder )
{
  cz_cbor_encoder.write_string( "publicdata" );
  cz_cbor_encoder.start_map();
  cz_cbor_encoder.write_map_entry( "data", _data );
  encode_encoding_to_cbor( cz_cbor_encoder );
  cz_cbor_encoder.end_map();
}

void CredentialPrivateData::encode_to_cbor( CborEncoder &cz_cbor_encoder )
{
  cz_cbor_encoder.write_string( "privatedata" );
  cz_cbor_encoder.start_map();
  cz_cbor_encoder.write_map_entry( "data", _data );
  encode_encoding_to_cbor( cz_cbor_encoder );
  cz_cbor_encoder.write_map_entry( "handle", _handle );
  cz_cbor_encoder.end_map();
}

void CredentialPrivateData::decode_from_cbor(ResRepresentation & cbor_rep)
{
	BaseCredentialData::decode_from_cbor(cbor_rep);
	_handle = (uint16_t)cbor_rep.get_prop<long>("handle");
}

void CredentialOptionalData::encode_to_cbor( CborEncoder &cz_cbor_encoder )
{
  cz_cbor_encoder.write_string( "optionaldata" );
  cz_cbor_encoder.start_map();
  cz_cbor_encoder.write_map_entry( "data", _data );
  encode_encoding_to_cbor( cz_cbor_encoder );
  cz_cbor_encoder.write_map_entry( "revstat", _revocation_status );
  cz_cbor_encoder.end_map();
}

void CredentialOptionalData::decode_from_cbor(ResRepresentation & cbor_rep)
{
	BaseCredentialData::decode_from_cbor(cbor_rep);
	cbor_rep.get_prop("revstat", _revocation_status);
}
}
}
