#ifndef __PNG_INTERNAL_H
#define __PNG_INTERNAL_H

/*=======================================
 * Required header files
 *=======================================*/

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

#include <png.h>

#if defined(__cplusplus)
} /* extern "C" */
#endif

/*================================================
 * Function pointer and other definitions
 *===============================================*/

typedef png_voidp PNGUErrorPtr;

//Callback function types for error and warning
typedef void (*PNGUErrorFuncType)   (png_structp png_ptr, png_const_charp error_msg);
typedef void (*PNGUWarningFuncType) (png_structp png_ptr, png_const_charp warning_msg);

//Number of bytes read to chechk file signature
#define PNGU_SIG_BYTES 8


/*===================================================
 *
 * Use the following error handling in C programs
 *
 *===================================================*/

/*
   By default libpng invokes a long jump back to the
   point where its jump buffer was set. The libpng jump
   buffer is obtained via a call to png_jmpbuf(...) and
   should be then passed to setjmp funtion. This function
   returns 0 when the buffer is set and 1 when the
   control was returned back to the same point by a
   longjmp call. The provided macro is just a wrapper to
   such call to hide the setjmp call behind a more
   readable alias and should be used like this:

   png_structp ps;
   if (png_jmp_error_caught(ps)) {
      //handle the error
   }

 */

#include <setjmp.h>

#define pngu_jmp_error_caught(ps) \
  (setjmp(png_jmpbuf(ps)))


/*===================================================
 *
 * Use the following error handling in C++ programs
 *
 *===================================================*/

/*
  Instead of the default long jump error handling
  provided by the libpng library, we want to raise
  a C++ error. This is much safer since all the
  objects get properly deconstructed. The custom
  error handling function should be passed to the
  png_create_read_struct(...) function as the third
  argument:

  pngu_create_read_struct(..., ..., png_throw_error_fn, ...);

  Every png could can than be normally wrapped into a
  try() - catch() pair.

  */

#if defined(__cplusplus)

/*void pngu_throw_error_fn(png_structp ps, png_const_charp msg) throw(...)
{
  throw msg;
}*/

#endif /* defined(__cplusplus) */


#endif /* __PNG_INTERNAL_H */
