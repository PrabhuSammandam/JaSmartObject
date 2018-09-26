#pragma once

#include <cstdint>
#include "end_point.h"
#include "ip_addr.h"
#include "StackMessage.h"

/* Required params for request
 *
 * 1. IP addr
 * 2. Port
 * 3. Observe
 * 4. Message Type
 * 5. Content-Format
 * 6. Timeout
 * 7. Block2 Size
 * 8. Accept
 * 9. Path
 * 10. Query
 *
 *
 * Following are the operations done by client
 * 1. DISCOVER
 * 2. GET
 * 3. PUT
 * 4. POST
 * 5. DELETE
 * 6. OBSERVE
 *
 * Input needed for each operations
 *
 * Common for all operations.
 * a. IP addr,
 * b. Port
 * c. Timeout
 * d. Message Type
 * e. Path
 * f. Query
 *
 * 1. GET
 *              a. Accept
 *              b. Block2 Size
 *
 * 2. PUT
 *              a. Accept
 *              b. Content-Format
 *              c. Block2 Size
 *              d. Payload
 *
 * 3. POST
 *              a. Accept
 *              b. Content-Format
 *              c. Block2 Size
 *              d. Payload
 *
 * 4. DELETE
 *
 * 5. OBSERVE
 *              a. Accept
 *
 * 6. DISCOVER
 *
 */

class BaseRequestInfo
{
  public:
    BaseRequestInfo( ja_iot::network::Endpoint &endpoint, std::string uri_path, client_response_cb response_cb );

  public:
		ja_iot::network::Endpoint _endpoint;
    uint8_t                   _msg_type = COAP_MSG_TYPE_CON;
    std::string               _uri_path;
    std::string               _uri_query;
    client_response_cb        _response_cb = nullptr;
};

class GetRequestInfo : public BaseRequestInfo
{
public:
	GetRequestInfo(ja_iot::network::Endpoint &endpoint, std::string uri_path, client_response_cb response_cb);
public:
    uint16_t   _accept_format = COAP_CONTENT_FORMAT_JSON;
    uint16_t   _block2_size = 0;
};

class PutRequestInfo : public BaseRequestInfo
{
public:
	PutRequestInfo(ja_iot::network::Endpoint &endpoint, std::string uri_path, client_response_cb response_cb);

	void set_payload(std::string & payload);

  public:
    uint16_t   _accept_format;
    uint16_t   _content_format;
    uint16_t   _block2_size;
    int8_t *   _payload_buf;
    uint16_t   _payload_buf_len;
};

typedef class PutRequestInfo PostRequestInfo;
typedef class BaseRequestInfo DeleteRequestInfo;
typedef class GetRequestInfo ObserveRequestInfo;
typedef class GetRequestInfo DiscoverRequestInfo;

class Client
{
  public:
    uint8_t get( GetRequestInfo &get_request_info );
    uint8_t put( PutRequestInfo &put_request_info );
    uint8_t post( PostRequestInfo &post_request_info );
    uint8_t del( DeleteRequestInfo &delete_request_info );
    uint8_t observe( ObserveRequestInfo &observe_request_info );
    uint8_t discover( DiscoverRequestInfo &discover_request_info );
};