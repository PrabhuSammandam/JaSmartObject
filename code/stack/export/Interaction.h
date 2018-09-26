/*
 * Interaction.h
 *
 *  Created on: Feb 14, 2018
 *      Author: psammand
 */

#pragma once

#include "StackMessage.h"

namespace ja_iot {
namespace stack {
class Interaction
{
  public:
    virtual ~Interaction () {}
    virtual ServerRequest* get_server_request()                                    = 0;
    virtual void           set_server_request( ServerRequest *pcz_server_request ) = 0;

    virtual ServerResponse* get_server_response()                                      = 0;
    virtual void            set_server_response( ServerResponse *pcz_server_response ) = 0;
};
}
}