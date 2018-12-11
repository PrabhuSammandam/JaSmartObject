/*
 * SecureContextMgr.cpp
 *
 *  Created on: Dec 8, 2018
 *      Author: psammand
 */

#include "SecureContextMgr.h"

namespace ja_iot {
namespace network {
/* singleton instance of SecureContextMgr */
SecureContextMgr *SecureContextMgr::_pcz_instance{};

SecureContextMgr & SecureContextMgr::inst()
{
  if( _pcz_instance == nullptr )
  {
    static SecureContextMgr _instance{};
    _pcz_instance = &_instance;
  }

  return ( *_pcz_instance );
}
SecureContextMgr::SecureContextMgr ()
{
}
SecureContextMgr::~SecureContextMgr ()
{
}
}
}



