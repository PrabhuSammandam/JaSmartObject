/*
 * AccessControlEntry.h
 *
 *  Created on: Nov 20, 2018
 *      Author: psammand
 */

#pragma once
#include <cstdint>
#include <vector>
#include <string>
#include "Uuid.h"
#include "security/SecurityDataTypes.h"
#include "cbor/CborEncoder.h"
#include "ResPropValue.h"

namespace ja_iot {
namespace resources {
using namespace base;
using namespace stack;
class AceResource
{
  public:
    AceResource () {}
    AceResource( const std::string &href ) : _href{ href } {}

    std::string& get_href() { return ( _href ); }
    void       set_href( const std::string &href ) { _href = href; }

    uint8_t get_wildcard() { return ( _wildcard ); }
    void    set_wildcard( uint8_t wildcard ) { _wildcard = wildcard; }

    bool is_all_discoverable() { return ( _wildcard == ACL_WILDCARD_ALL_DISCOVERABLE ); }
    bool is_all_non_discoverable() { return ( _wildcard == ACL_WILDCARD_ALL_NON_DISCOVERABLE ); }
    bool is_all_resources() { return ( _wildcard == ACL_WILDCARD_ALL_RESOURCES ); }

		bool is_matched(const std::string &href, bool is_discoverable);
		bool is_matched(const std::string &href, const std::vector<std::string> &rt_array, const std::vector<std::string> &if_array, bool is_discoverable);

    std::vector<std::string>& get_types() { return ( _types ); }
    std::vector<std::string>& get_interfaces() { return ( _interfaces ); }

    void encode_to_cbor( CborEncoder &cz_cbor_encoder );

  private:
    std::string                _href;
    std::vector<std::string>   _types;
    std::vector<std::string>   _interfaces;
    uint8_t                    _wildcard = ACL_WILDCARD_NONE;
};

class AceTimePattern
{
  public:
    bool operator == ( const AceTimePattern &other );

  private:
    std::string                _period;
    std::vector<std::string>   _recurrence_array;
};

class AceSubjectBase
{
  public:
    AceSubjectBase( uint8_t type ) : _type{ type } {}
    virtual ~AceSubjectBase () {}
    uint8_t getType() const { return ( _type ); }
    void    setType( uint8_t type ) { _type = type; }
    void    encode_to_cbor( CborEncoder &cz_cbor_encoder );

    virtual void do_encode_to_cbor( CborEncoder &cz_cbor_encoder ) = 0;

  private:
    uint8_t   _type;
};

class AceSubjectUuid : public AceSubjectBase
{
  public:
    AceSubjectUuid () : AceSubjectBase{ ACE_SUBJECT_TYPE_UUID } {}

    bool operator == ( const AceSubjectUuid &other ) { return ( _uuid == other._uuid ); }

    Uuid & get_uuid() { return ( _uuid ); }
    void do_encode_to_cbor( CborEncoder &cz_cbor_encoder ) override;

  private:
    Uuid   _uuid;
};

class AceSubjectRole : public AceSubjectBase
{
  public:
    AceSubjectRole () : AceSubjectBase{ ACE_SUBJECT_TYPE_ROLE } {}

    std::string& get_authority() { return ( _authority ); }
    void       set_authority( const std::string &authority ) { _authority = authority; }
    std::string& get_role() { return ( _role ); }
    void       set_role( const std::string &role ) { _role = role; }

    bool operator == ( const AceSubjectRole &other ) { return ( _authority == other._authority && _role == other._role ); }
    void          do_encode_to_cbor( CborEncoder &cz_cbor_encoder ) override;

  private:
    std::string   _authority;
    std::string   _role;
};

class AceSubjectConnection : public AceSubjectBase
{
  public:
    AceSubjectConnection( uint8_t connection_type ) : AceSubjectBase{ ACE_SUBJECT_TYPE_CONNECTION }, _connection{ connection_type } {}

    uint8_t getConnection() const { return ( _connection ); }
    void    setConnection( uint8_t connection ) { _connection = connection; }

    bool operator == ( const AceSubjectConnection &other ) { return ( _connection == other._connection ); }
    void          do_encode_to_cbor( CborEncoder &cz_cbor_encoder ) override;

  private:
    uint8_t   _connection;
};

typedef std::vector<AceResource *> AceResourceList;
typedef std::vector<AceTimePattern *> AceTimePatternList;

class AccessControlEntry
{
  public:
    AccessControlEntry( uint16_t id );
    ~AccessControlEntry ();

    bool operator == ( const AccessControlEntry &other );
    bool          is_same( const AccessControlEntry &other );

    uint16_t get_id() const { return ( _id ); }

    void    set_permission( uint8_t permission ) { _permission = permission; }
    uint8_t get_permission() { return ( _permission ); }

    void            set_subject( AceSubjectBase *subject ) { _subject = subject; }
    AceSubjectBase* get_subject() { return ( _subject ); }

    AceResourceList   & get_resources_array() { return ( _resources_array ); }
    AceTimePatternList& get_validities_array() { return ( _validities_array ); }

		bool has_resource(const std::string &href, bool is_discoverable);
    bool has_resource( const std::string &href, const std::vector<std::string> &rt_array, const std::vector<std::string> &if_array, bool is_discoverable );

    void    encode_to_cbor( CborEncoder &cz_cbor_encoder );
    uint8_t decode_from_cbor( ResRepresentation &cbor_ace_obj );

  private:
    uint16_t             _id = 0;
    AceResourceList      _resources_array;
    uint8_t              _permission = CRUDN_PERMISSION_NONE;
    AceSubjectBase *     _subject    = nullptr;
    AceTimePatternList   _validities_array;
};

typedef std::vector<AccessControlEntry *> AccessControlList;
}
}