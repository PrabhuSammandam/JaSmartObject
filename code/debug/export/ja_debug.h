/*
 * ja_debug.h
 *
 *  Created on: Oct 7, 2017
 *      Author: psammand
 */

#pragma once

#define JA_ERR_GO( expression ) \
  do { \
    if( !( expression ) ){ \
      goto exit_label_; \
    } \
  } while( 0 )

#define JA_ERR_CHECK_RAISE( expression, stat, err ) \
  do { \
    if( !( expression ) ){ \
      stat = ( err ); \
      goto exit_label_; \
    } \
  } while( 0 )

#define JA_ERR_RAISE( stat, err ) \
  do { \
    stat = ( err ); \
    goto __XDEBUG_EXIT_LABEL; \
  } while( 0 )

#define JA_ERR_OK_PRINT_GO( stat, ... )   do { \
    if( ( stat ) != ErrCode::OK ){ \
      DBG_ERROR( __VA_ARGS__ ); \
      goto exit_label_; \
    } \
  } while( 0 )
