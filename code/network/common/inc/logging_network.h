/*
 * logging_network.h
 *
 *  Created on: Sep 12, 2017
 *      Author: psammand
 */

#pragma once

#include <cstdio>

#ifdef _DEBUG_
#ifdef _NETWORK_DEBUG_

#define DBG_INFO( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_WARN( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_ERROR( format, ... ) printf( format "\n", ## __VA_ARGS__ )
#define DBG_FATAL( format, ... ) printf( format "\n", ## __VA_ARGS__ )

#ifdef __GNUC__
#define DBG_INFO2( format, ... ) printf( "INF:%s::%s:%d# " format "\n", __FILE_NAME__, __FUNCTION__, __LINE__, ## __VA_ARGS__ )
#define DBG_WARN2( format, ... ) printf( "WRN:%s::%s:%d# " format "\n", __FILE_NAME__, __FUNCTION__, __LINE__, ## __VA_ARGS__ )
#define DBG_ERROR2( format, ... ) printf( "ERR:%s::%s:%d# " format "\n", __FILE_NAME__, __FUNCTION__, __LINE__, ## __VA_ARGS__ )
#define DBG_FATAL2( format, ... ) printf( "FTL:%s::%s:%d# " format "\n", __FILE_NAME__, __FUNCTION__, __LINE__, ## __VA_ARGS__ )
#else
#define DBG_INFO2( format, ... ) printf( "INF:%s::%s:%d# " format "\n", __FILE_NAME__,__func__, __LINE__, ## __VA_ARGS__ )
#define DBG_WARN2( format, ... ) printf( "WRN:%s::%s:%d# " format "\n", __FILE_NAME__,__func__, __LINE__, ## __VA_ARGS__ )
#define DBG_ERROR2( format, ... ) printf( "ERR:%s::%s:%d# " format "\n", __FILE_NAME__,__func__, __LINE__, ## __VA_ARGS__ )
#define DBG_FATAL2( format, ... ) printf( "FTL:%s::%s:%d# " format "\n", __FILE_NAME__,__func__, __LINE__, ## __VA_ARGS__ )
#endif

#define EXIT_LABEL_ exit_label_;

#else

#define DBG_INFO( format, ... )
#define DBG_WARN( format, ... )
#define DBG_ERROR( format, ... )
#define DBG_FATAL( format, ... )

#define DBG_INFO2( format, ... )
#define DBG_WARN2( format, ... )
#define DBG_ERROR2( format, ... )
#define DBG_FATAL2( format, ... )

#endif

#else

#define DBG_INFO( format, ... )
#define DBG_WARN( format, ... )
#define DBG_ERROR( format, ... )
#define DBG_FATAL( format, ... )

#define DBG_INFO2( format, ... )
#define DBG_WARN2( format, ... )
#define DBG_ERROR2( format, ... )
#define DBG_FATAL2( format, ... )

#define EXIT_LABEL_

#endif /* _NETWORK_DEBUG_ */
