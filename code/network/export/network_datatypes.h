/*
 * network_datatypes.h
 *
 *  Created on: Dec 8, 2018
 *      Author: psammand
 */

#pragma once

#include <vector>

namespace ja_iot {
namespace network {
class Endpoint;
class InterfaceAddress;

typedef std::vector<Endpoint *> EndpointList;
typedef std::vector<InterfaceAddress *> InterfaceAddressList;
}
}