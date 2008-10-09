#ifndef __JPEG_INTERNAL_H
#define __JPEG_INTERNAL_H

/*=========================================
 * Required header files
 *=========================================*/

#include <stdio.h>
#include <sys/types.h>

#if defined(WIN32)
#  include <malloc.h>
#else
#  include <stdlib.h>
#endif

#if defined(__cplusplus)
extern "C" {
#endif

#include <jpeglib.h>
#include <jerror.h>
  
#if defined(__cplusplus)
} /* extern "C" */
#endif


/*============================================
 * Structure definitions
 *============================================*/

typedef   struct jpeg_error_mgr            jpeg_error_mgr;
typedef   struct jpeg_source_mgr           jpeg_source_mgr;
typedef   struct jpeg_decompress_struct    jpeg_decompress_struct;
typedef   struct jpeg_compress_struct      jpeg_compress_struct;



/*======================================================
 *
 * Use the following error management in C programs
 *
 *======================================================*/

/*
  The idea is to declare a custom error manager
  instead of the default one like this:

  struct jpeg_jmp_error_mgr jerrmgr;

  Custom error handling would be setup by passing an
  instance of the custom error manager to the jpeg
  decompressor structure instead of the default one:

  struct jpeg_decompress_struct jdc;
  jdc.err = jpeg_jmp_error(&jerrmgr);

  Prior to initializing the rest of the jdc structure
  we would setup the event handler for the OnExit event.
  The custom error manager overrides this event so that
  instead of calling exit() like the default manager does,
  the control is returned via an explicit long jump back
  to the point in code where jpeg_my_error_caught(...)
  was called. When this happens the function returns 1(true),
  while 0(false) is returned the first time jpeg_jmp_error_caught
  is called:

  if (jpeg_jmp_error_caught(&jdc)) {
    jpeg_destroy_decompress(&jdc);
    // other stuff related to handling the error...
  }
*/

#include <setjmp.h>

/*------------------------------------------------
 * Longjmp error manager structure
 *------------------------------------------------*/

struct jpeg_jmp_error_mgr {
  /* Include 'base class' members */
  struct jpeg_error_mgr base;
  /* Where to return control instead of exiting */
  jmp_buf return_point;

};

/*-------------------------------------------------
 * Structure and pointer typedefs
 *-------------------------------------------------*/

typedef   struct jpeg_jmp_error_mgr     jpeg_jmp_error_mgr;
typedef   jpeg_jmp_error_mgr*           jpeg_jmp_error_mgr_ptr;

/*----------------------------------------------------
 * Longjmp manager initializer function
 *----------------------------------------------------*/

jpeg_error_mgr* jpeg_jmp_error(jpeg_jmp_error_mgr_ptr errmgr);

/*----------------------------------------------------------
 * This macro sets onexit return-point where called and
 * returns 1 if longjmp sent control back to the same point
 *----------------------------------------------------------*/

#define jpeg_jmp_error_caught(jdc) \
  setjmp( ((jpeg_jmp_error_mgr*)(jdc)->err)->return_point )




/*======================================================
 *
 * Use the following error management in C++ programs
 *
 *======================================================*/

/*
  Here we don't need any custom error manager structure
  to store additional variables as in the C variant.
  We just have to define a custom onexit event handler
  which throws a C++ exception instead of calling exit()
  like the default event handler function does. The
  error manager should then be initialized via custom
  initialization function instead of default one
  (called jpeg_std_error):

  struct jpeg_error_mgr jerrmgr;
  jpeg_throw_error(&jerrmgr)

  Any jpeg function related to the decompressor structure
  with this error manager can then be wrapped in standard
  C++ try() catch() calls.
*/

#if defined(__cplusplus)

/*-------------------------------------------------
 * Exception-throwing manager initializer function
 *-------------------------------------------------*/

jpeg_error_mgr* jpeg_throw_error(jpeg_error_mgr *mgr);

#endif /* defined(__cplusplus) */

#endif /* __JPEG_INTERNAL_H */
