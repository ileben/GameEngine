#define GE_API_EXPORT
#include "geEngine.h"

#define GE_NO_EXTENSION_ROUTING
#include "geGLHeaders.h"

#if !defined(WIN32) && !defined(__APPLE__)
#  include <X11/Xlib.h>
#  include <X11/Xutil.h>
#  include <X11/Xos.h>
#  include <X11/Xatom.h>
#  include <X11/keysym.h>
#  include <X11/Xmu/StdCmap.h>
#endif

#include <stdio.h>
using namespace OCC;

/*========================================
 *
 * Vertex shader - Per pixel specularity
 *
 *========================================*/

const char *vshaderSpecularity = ""; /*TODO: instead from file */

/*==========================================
 *
 * Fragment shader - Per pixel specularity
 *
 *==========================================*/

const char *fshaderSpecularity = ""; /*TODO: instead from file */

/*=================================================
 *
 * Function pointers for undefined extensions
 *
 *=================================================*/

#if !defined(WIN32) && !defined(__APPLE__)
GE_PFGLXGETPROCADDRESS           GE_glXGetProcAddress = NULL;
#endif

#ifndef GL_VERSION_1_2
GE_PFGLDRAWRANGEELEMENTS         GE_glDrawRangeElements = NULL;
#endif

#ifndef GL_VERSION_1_3
GE_PFGLACTIVETEXTURE             GE_glActiveTexture = NULL;
GE_PFGLMULTITEXCOORD2F           GE_glMultiTexCoord2f = NULL;
#endif

#ifndef GL_VERSION_2_0
GE_PFGLCREATEPROGRAM             GE_glCreateProgram = NULL;
GE_PFGLCREATESHADER              GE_glCreateShader = NULL;
GE_PFGLATTACHSHADER              GE_glAttachShader = NULL;
GE_PFGLSHADERSOURCE              GE_glShaderSource = NULL;
GE_PFGLCOMPILESHADER             GE_glCompileShader = NULL;
GE_PFGLLINKPROGRAM               GE_glLinkProgram = NULL;
GE_PFGLUSEPROGRAM                GE_glUseProgram = NULL;
GE_PFGLDETACHSHADER              GE_glDetachShader = NULL;
GE_PFGLDELETESHADER              GE_glDeleteShader = NULL;
GE_PFGLDELETEPROGRAMSARB         GE_glDeleteProgramsARB = NULL;
GE_PFGLGETINFOLOGARB             GE_glGetInfoLogARB = NULL;
GE_PFGLGETOBJECTPARAMETERIVARB   GE_glGetObjectParameterivARB = NULL;
GE_PFGLGETUNIFORMLOCATION        GE_glGetUniformLocation = NULL;
GE_PFGLUNIFORM1I                 GE_glUniform1i = NULL;
GE_PFGLUNIFORM2I                 GE_glUniform2i = NULL;
GE_PFGLUNIFORM3I                 GE_glUniform3i = NULL;
GE_PFGLUNIFORM4I                 GE_glUniform4i = NULL;
GE_PFGLUNIFORM1F                 GE_glUniform1f = NULL;
GE_PFGLUNIFORM2F                 GE_glUniform2f = NULL;
GE_PFGLUNIFORM3F                 GE_glUniform3f = NULL;
GE_PFGLUNIFORM4F                 GE_glUniform4f = NULL;
#endif

GE_PFGLSWAPINTERVAL glSwapInterval = NULL;

/*=================================================
 *
 * Kernel loads extensions and shader programs
 *
 *=================================================*/

namespace GE
{
  DEFINE_CLASS (Kernel);
  
  Kernel* Kernel::Instance = NULL;
    
  /*------------------------------------------------
  Returns address for a named procedure natively
  --------------------------------------------------*/
  
  PFVOID Kernel::getProcAddress(const char *name)
  {
    #if defined(_WIN32)
    return (PFVOID) wglGetProcAddress (name);
    #elif defined(__APPLE__)
    //TODO: Mac OS glGetProcAddress implementation
    return (PFVOID) NULL;
    #else
    return GE_glXGetProcAddress ((GLubyte*) name);
    #endif
  }
  
  /*------------------------------------------------
  Returns true if named extension is supported
  --------------------------------------------------*/
  
  int Kernel::checkExtension(const char *extensions, const char *name)
  {
    int nlen = (int)strlen(name);
    int elen = (int)strlen(extensions);
    const char *e = extensions;
    ASSERT(nlen > 0);

    while (1) {

      //Try to find sub-string
      e = strstr(e, name);
      if (e == NULL) return 0;
      //Check if last
      if (e == extensions + elen - nlen)
        return 1;
        //Check if space follows (avoid same names with a suffix)
      if (*(e + nlen) == ' ')
        return 1;

      e += nlen;
    }

    return 0;
  }
  
  /*-------------------------------------------------------
  Loads all the used OpenGL extensions. The extension
  names are first searched for in the opengl extensions
  string to check whether they are supported by the
  hardware. The missing procedure addresses are then
  queried for if not defined in the opengl headers.
  ---------------------------------------------------------*/
  
  void Kernel::loadExtensions()
  {
    /*
    Check for glXGetProcAddress
    *****************************************/
    
    #if !defined(WIN32) && !defined(__APPLE__)
    
    void *dlhandle = dlopen (NULL, RTLD_LAZY);
    ASSERT (dlhandle != NULL);
    
    GE_glXGetProcAddress =
      (GE_PFGLXGETPROCADDRESS) dlsym (dlhandle, "glXGetProcAddress");
    
    if (GE_glXGetProcAddress == NULL)
    {
      GE_glXGetProcAddress =
        (GE_PFGLXGETPROCADDRESS) dlsym (dlhandle, "glXGetProcAddressARB");
      
      ASSERT (GE_glXGetProcAddress != NULL);
    }
    
    #endif
    
    
    /*
    Obtain supported extension string
    *****************************************/
    
    const char *ext = (const char*) glGetString (GL_EXTENSIONS);
    
    /*
    Check elements range
    *****************************************/
    
    if (checkExtension(ext, "GL_EXT_draw_range_elements")) {
      hasRangeElements = true;
      
      #ifndef GL_VERSION_1_2
      GE_glDrawRangeElements = (GE_PFGLDRAWRANGEELEMENTS)
        getProcAddress ("glDrawRangeElementsEXT");
      
      if (GE_glDrawRangeElements==NULL)
        hasRangeElements = false;
      #endif
      
      glGetIntegerv(GL_MAX_ELEMENTS_INDICES, &maxElementsIndices);
      glGetIntegerv(GL_MAX_ELEMENTS_VERTICES, &maxElementsVertices);
      
    }else{ hasRangeElements = false; }
    
    /*
    Check multitexturing
    *****************************************/
    
    if (checkExtension(ext, "GL_ARB_multitexture")) {
      hasMultitexture = true;

      #ifndef GL_VERSION_1_3
      GE_glActiveTexture = (GE_PFGLACTIVETEXTURE)
        getProcAddress ("glActiveTextureARB");
      GE_glMultiTexCoord2f = (GE_PFGLMULTITEXCOORD2F)
        getProcAddress ("glMultiTexCoord2fARB");
      
      if (GE_glActiveTexture==NULL || GE_glMultiTexCoord2f==NULL)
        hasMultitexture = false;
      #endif

    }else{ hasMultitexture = false; }
    
    /*
    Check shaders
    *****************************************/
    
    if (checkExtension(ext, "GL_ARB_shader_objects") &&
        checkExtension(ext, "GL_ARB_vertex_program") &&
        checkExtension(ext, "GL_ARB_fragment_program")) {
      hasShaders = true;
      
      #ifndef GL_VERSION_2_0
      GE_glCreateProgram = (GE_PFGLCREATEPROGRAM)
        getProcAddress ("glCreateProgramObjectARB");
      GE_glCreateShader = (GE_PFGLCREATESHADER)
        getProcAddress ("glCreateShaderObjectARB");
      GE_glAttachShader = (GE_PFGLATTACHSHADER)
        getProcAddress ("glAttachObjectARB");
      GE_glShaderSource = (GE_PFGLSHADERSOURCE)
        getProcAddress ("glShaderSourceARB");
      GE_glCompileShader = (GE_PFGLCOMPILESHADER)
        getProcAddress ("glCompileShaderARB");
      GE_glLinkProgram = (GE_PFGLLINKPROGRAM)
        getProcAddress ("glLinkProgramARB");
      GE_glUseProgram = (GE_PFGLUSEPROGRAM)
        getProcAddress ("glUseProgramObjectARB");
      GE_glDetachShader = (GE_PFGLDETACHSHADER)
        getProcAddress ("glDetachObjectARB");
      GE_glDeleteShader = (GE_PFGLDELETESHADER)
        getProcAddress ("glDeleteObjectARB");
      GE_glDeleteProgramsARB = (GE_PFGLDELETEPROGRAMSARB)
        getProcAddress ("glDeleteProgramsARB");
      GE_glGetInfoLogARB = (GE_PFGLGETINFOLOGARB)
        getProcAddress ("glGetInfoLogARB");
      GE_glGetObjectParameterivARB = (GE_PFGLGETOBJECTPARAMETERIVARB)
        getProcAddress ("glGetObjectParameterivARB");
      GE_glGetUniformLocation = (GE_PFGLGETUNIFORMLOCATION)
        getProcAddress ("glGetUniformLocationARB");
      GE_glUniform1i = (GE_PFGLUNIFORM1I)
        getProcAddress ("glUniform1iARB");
      GE_glUniform2i = (GE_PFGLUNIFORM2I)
        getProcAddress ("glUniform2iARB");
      GE_glUniform3i = (GE_PFGLUNIFORM3I)
        getProcAddress ("glUniform3iARB");
      GE_glUniform4i = (GE_PFGLUNIFORM4I)
        getProcAddress ("glUniform4iARB");
      GE_glUniform1f = (GE_PFGLUNIFORM1F)
        getProcAddress ("glUniform1fARB");
      GE_glUniform2f = (GE_PFGLUNIFORM2F)
        getProcAddress ("glUniform2fARB");
      GE_glUniform3f = (GE_PFGLUNIFORM3F)
        getProcAddress ("glUniform3fARB");
      GE_glUniform4f = (GE_PFGLUNIFORM4F)
        getProcAddress ("glUniform4fARB");
      
      if (GE_glCreateProgram==NULL || GE_glCreateShader==NULL ||
          GE_glCreateShader==NULL || GE_glAttachShader==NULL ||
          GE_glShaderSource==NULL || GE_glCompileShader==NULL ||
          GE_glLinkProgram==NULL || GE_glUseProgram==NULL ||
          GE_glUseProgram==NULL || GE_glDetachShader==NULL ||
          GE_glDeleteShader==NULL || GE_glDeleteProgramsARB==NULL ||
          GE_glGetInfoLogARB==NULL || GE_glGetObjectParameterivARB==NULL ||
          GE_glGetUniformLocation==NULL ||
          GE_glUniform1i==NULL || GE_glUniform2i==NULL ||
          GE_glUniform3i==NULL || GE_glUniform4i==NULL ||
          GE_glUniform1f==NULL || GE_glUniform2f==NULL ||
          GE_glUniform3f==NULL || GE_glUniform4f==NULL)
        hasShaders = false;
      #endif
      
    }else{ hasShaders = false; }
    
    /*
    Check vertical sync control
    *****************************************/
    
    #if defined(WIN32)
    glSwapInterval = (GE_PFGLSWAPINTERVAL)
      getProcAddress("wglSwapIntervalEXT");
    #endif
  }
  
  /*------------------------------------------------
  Constructor for a single instance
  --------------------------------------------------*/
  
  Kernel::Kernel()
  {
    //Only allow one kernel
    assert(Kernel::Instance == NULL);
    Kernel::Instance = this;
    
    //Load extensions
    this->loadExtensions();
  }
  
  Kernel::~Kernel()
  {
  }
  
  Object* Kernel::spawn (ClassName cn)
  {
    Object *obj = (Object*) New (cn);
    //obj->initProperties ();
    objects.pushBack (obj);
    return obj;
  }
  
  Object* Kernel::spawn (const char *classString)
  {
    ClassName cn = ClassFromString (classString);
    if (cn == NULL) return NULL;
    return spawn (cn);
  }
  
  void Kernel::enableVerticalSync(bool on)
  {
    if (glSwapInterval)
      glSwapInterval(on ? 1 : 0);
  }

}//namespace GE
