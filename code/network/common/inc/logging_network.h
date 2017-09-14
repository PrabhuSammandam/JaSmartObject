/*
 * logging_network.h
 *
 *  Created on: Sep 12, 2017
 *      Author: psammand
 */

#ifndef NETWORK_COMMON_INC_LOGGING_NETWORK_H_
#define NETWORK_COMMON_INC_LOGGING_NETWORK_H_

#ifdef _DEBUG_
#ifdef _NETWORK_DEBUG_

#define DBG_INFO( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_WARN( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_ERROR( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_FATAL( format, ... ) printf( format "\n", ## __VA_ARGS__ )

#else

#define DBG_INFO( format, ... )
#define DBG_WARN( format, ... )
#define DBG_ERROR( format, ... )
#define DBG_FATAL( format, ... )

#endif

#else

#define DBG_INFO( format, ... )
#define DBG_WARN( format, ... )
#define DBG_ERROR( format, ... )
#define DBG_FATAL( format, ... )

#endif /* _NETWORK_DEBUG_ */

#endif /* NETWORK_COMMON_INC_LOGGING_NETWORK_H_ */
