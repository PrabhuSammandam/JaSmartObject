/*
 * SecureContextMgr.h
 *
 *  Created on: Dec 8, 2018
 *      Author: psammand
 */

#pragma once

#include "SecureContext.h"

namespace ja_iot {
namespace network {
class SecureContextMgr
{
  public:
    static SecureContextMgr& inst();
    SecureContext          & get_context() { return ( _pcz_context ); }

  private:
    SecureContextMgr ();
    ~SecureContextMgr ();
    SecureContextMgr( const SecureContextMgr & )               = delete;// prevent copy constructor
    SecureContextMgr & operator = ( const SecureContextMgr & ) = delete;// prevent assignment operator
    static SecureContextMgr * _pcz_instance;       // static class variable cannot be instantiated here.
    SecureContext             _pcz_context;
};
}
}