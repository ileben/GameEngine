#include "util/geUtil.h"
#include "image/geImage.h"

#include "jpeg_internal.h"
#include "jpeg_external.h"

/*======================================================
 *
 * Use the following error management in C programs
 *
 *======================================================*/


/*------------------------------------------------------
 * OnExit error handler callback (happens when
 * the default error manager would call exit())
 *------------------------------------------------------*/

void jpeg_jmp_error_onexit(j_common_ptr jdc)
{
  /* We know its our manager actually */
  jpeg_jmp_error_mgr_ptr errmgr =
     (jpeg_jmp_error_mgr_ptr)jdc->err;
  /* Return control to saved return-point */
  longjmp(errmgr->return_point, 1);
}

/*------------------------------------------------------
 * OnEmitMsg error handler callback (happens when
 * the default error manager would like to emit any
 * kind of warning or error message)
 *------------------------------------------------------*/

void jpeg_jmp_error_onemitmsg(j_common_ptr jdc, int msg_level)
{
  /* Emitting messages in jpeg seems to be broken on Windows
     as it always segfalts in sprintf (bad file pointer). We
     just override the function to do nothing */
}

/*------------------------------------------------
 * Longjmp error manager initializer
 *------------------------------------------------*/

struct jpeg_error_mgr* jpeg_jmp_error(jpeg_jmp_error_mgr_ptr errmgr)
{
  /* Initialize base class part */
  jpeg_std_error(&errmgr->base);
  /* Override with custom onexit handler */
  errmgr->base.error_exit = jpeg_jmp_error_onexit;
  errmgr->base.emit_message = jpeg_jmp_error_onemitmsg;
  /* Return pointer to initialized handler */
  return (struct jpeg_error_mgr*)errmgr;
}

/*======================================================
 *
 * Use the following error management in C++ programs
 *
 *======================================================*/


#if defined(__cplusplus)

/*------------------------------------------------------
 * OnExit error handler callback (happens when
 * the default error manager would call exit())
 *------------------------------------------------------*/

/*void jpeg_throw_error_onexit(j_common_ptr jdc) throw(...)
{
  //Throw exception
  throw "jpeg decompression error";
}*/

/*------------------------------------------------------
 * OnEmitMsg error handler callback (happens when
 * the default error manager would like to emit any
 * kind of warning or error message)
 *------------------------------------------------------*/

/*void jpeg_throw_error_onemitmsg(j_common_ptr jdc, int msg_level)
{
  //Emitting messages in jpeg seems to be broken on Windows
  //as it always segfalts in sprintf (bad file pointer). We
  //just override the function to do nothing
}*/

/*------------------------------------------------------
 * Exception-throwing error manager initializer
 *------------------------------------------------------*/

/*struct jpeg_error_mgr* jpeg_throw_error(jpeg_error_mgr * mgr)
{
  //Initialize standard error management
  jpeg_std_error(mgr);
  //Override onexit event handler
  mgr->error_exit = jpeg_throw_error_onexit;
  mgr->emit_message = jpeg_throw_error_onemitmsg;

  return mgr;
}*/

#endif /* defined(__cplusplus) */
