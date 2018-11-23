/*
 * ResModuleInterface.h
 *
 *  Created on: Nov 23, 2018
 *      Author: psammand
 */

#include <cstdint>
#include "ErrCode.h"

namespace ja_iot {
namespace resources {
using namespace base;

ErrCode resource_module_init_core_resources();
ErrCode resource_module_init_secure_virtual_resources();

}
}
