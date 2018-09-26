/*
 * logging_stack.h
 *
 *  Created on: Mar 17, 2018
 *      Author: psammand
 */

#pragma once

#if defined(_DEBUG_) && defined(_STACK_DEBUG_)
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

#define DBG_INFO2( format, ... )
#define DBG_WARN2( format, ... )
#define DBG_ERROR2( format, ... )
#define DBG_FATAL2( format, ... )

#endif

