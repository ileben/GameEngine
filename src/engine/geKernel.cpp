#include <iostream>
#include "util/geUtil.h"
#include "io/geFile.h"
#include "image/geImage.h"
#include "engine/geTexture.h"
#include "engine/geTriMesh.h"
#include "engine/geSkinMesh.h"
#include "engine/geSkinPose.h"
#include "engine/geCharacter.h"
#include "engine/geKernel.h"
#include "engine/geRenderer.h"
#include "engine/geScene.h"
#include "engine/actors/geTriMeshActor.h"

#define GE_NO_EXTENSION_ROUTING
#include "geGLHeaders.h"

#include <cstdio>

#if !defined(WIN32) && !defined(__APPLE__)
#  include <X11/Xlib.h>
#  include <X11/Xutil.h>
#  include <X11/Xos.h>
#  include <X11/Xatom.h>
#  include <X11/keysym.h>
#endif


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

#ifndef GL_VERSION_1_5
GE_PFGLGENBUFFERS                GE_glGenBuffers = NULL;
GE_PFGLBINDBUFFER                GE_glBindBuffer = NULL;
GE_PFGLBUFFERDATA                GE_glBufferData = NULL;
GE_PFGLBUFFERSUBDATA             GE_glBufferSubData = NULL;
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
GE_PFGLUNIFORM1FV                GE_glUniform1fv = NULL;
GE_PFGLUNIFORM2FV                GE_glUniform2fv = NULL;
GE_PFGLUNIFORM3FV                GE_glUniform3fv = NULL;
GE_PFGLUNIFORM4FV                GE_glUniform4fv = NULL;
GE_PFGLUNIFORMMATRIX4FV          GE_glUniformMatrix4fv = NULL;
GE_PFGLBINDATTRIBLOCATION        GE_glBindAttribLocation = NULL;
GE_PFGLGETATTRIBLOCATION         GE_glGetAttribLocation = NULL;
GE_PFGLVERTEXATTRIBPOINTER       GE_glVertexAttribPointer = NULL;
GE_PFGLDISABLEVERTEXATTRIBARRAY  GE_glDisableVertexAttribArray = NULL;
GE_PFGLENABLEVERTEXATTRIBARRAY   GE_glEnableVertexAttribArray = NULL;
GE_PFGLGETSHADERIV               GE_glGetShaderiv = NULL;
GE_PFGLGETSHADERINFOLOG          GE_glGetShaderInfoLog = NULL;
GE_PFGLGETPROGRAMIV              GE_glGetProgramiv = NULL;
GE_PFGLGETPROGRAMINFOLOG         GE_glGetProgramInfoLog = NULL;
#endif

#ifndef GL_EXT_framebuffer_object
GE_PFGLGENFRAMEBUFFERS           GE_glGenFramebuffers = NULL;
GE_PFGLBINDFRAMEBUFFER           GE_glBindFramebuffer = NULL;
GE_PFGLDELETEFRAMEBUFFERS        GE_glDeleteFramebuffers = NULL;
GE_PFGLCHECKFRAMEBUFFERSTATUS    GE_glCheckFramebufferStatus = NULL;
GE_PFGLFRAMEBUFFERTEXTURE2D      GE_glFramebufferTexture2D = NULL;
GE_PFGLFRAMEBUFFERRENDERBUFFER   GE_glFramebufferRenderbuffer = NULL;
GE_PFGLISRENDERBUFFER            GE_glIsRenderbuffer = NULL;
GE_PFGLBINDRENDERBUFFER          GE_glBindRenderbuffer = NULL;
GE_PFGLDELETERENDERBUFFERS       GE_glDeleteRenderbuffers = NULL;
GE_PFGLGENRENDERBUFFERS          GE_glGenRenderbuffers = NULL;
GE_PFGLRENDERBUFFERSTORAGE       GE_glRenderbufferStorage = NULL;
#endif

#ifndef GL_ARB_draw_buffers
GE_PFGLDRAWBUFFERS               GE_glDrawBuffers = NULL;
#endif

#ifndef GL_ARB_occlusion_query
GE_PFGLGENQUERIES                GE_glGenQueries = NULL;
GE_PFGLDELETEQUERIES             GE_glDeleteQueries = NULL;
GE_PFGLISQUERY                   GE_glIsQuery = NULL;
GE_PFGLBEGINQUERY                GE_glBeginQuery = NULL;
GE_PFGLENDQUERY                  GE_glEndQuery = NULL;
GE_PFGLGETQUERYIV                GE_glGetQueryiv = NULL;
GE_PFGLGETQUERYOBJECTIV          GE_glGetQueryObjectiv = NULL;
GE_PFGLGETQUERYOBJECTUIV         GE_glGetQueryObjectuiv = NULL;
#endif

#ifndef GL_ARB_vertex_array_object
GE_PFGLBINDVERTEXARRAY           GE_glBindVertexArray = NULL;
GE_PFGLDELETEVERTEXARRAYS        GE_glDeleteVertexArrays = NULL;
GE_PFGLGENVERTEXARRAYS           GE_glGenVertexArrays = NULL;
GE_PFGLISVERTEXARRAY             GE_glIsVertexArray = NULL;
#endif

GE_PFGLSWAPINTERVAL glSwapInterval = NULL;


/*=================================================
 *
 * Kernel loads extensions and shader programs
 *
 *=================================================*/

namespace GE
{

  /*
  -------------------------------------
  Singleton
  -------------------------------------*/

  Kernel* Kernel::Instance = NULL;

  /*
  -------------------------------------
  Kernel-managed memory
  -------------------------------------*/

  KernelBuffer::KernelBuffer (int newSize)
  {
    refcount = 0;
    size = newSize;
    data = malloc (newSize);
  }
  
  KernelBuffer::~KernelBuffer ()
  {
    free (data);
  }
  
  void KernelBuffer::reference ()
  {
    refcount++;
  }
  
  void KernelBuffer::dereference ()
  {
    refcount--;
  }
  
  KernelBuffer* Kernel::createBuffer (int size)
  {
    KernelBuffer *buf = new KernelBuffer (size);
    buffers.add (buf);
    return buf;
  }

  void Kernel::refBuffer (KernelBuffer *buf)
  {
    buf->reference ();
  }

  void Kernel::derefBuffer (KernelBuffer* buf)
  {
    buf->dereference ();
    if (buf->refcount <= 0)
    {
      buffers.remove (buf);
      delete buf;
    }
  }
  
  /*
  --------------------------------------------------
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
  
  /*
  --------------------------------------------------
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
  
  /*
  --------------------------------------------------------
  Loads all the used OpenGL extensions. The extension
  names are first searched for in the opengl extensions
  string to check whether they are supported by the
  hardware. The missing procedure addresses are then
  queried for if not defined in the opengl headers.
  --------------------------------------------------------*/
  
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
    Check vertex buffer objects
    *****************************************/

    if (checkExtension(ext, "GL_ARB_vertex_buffer_object")) {
      hasVertexBufferObjects = true;

      #ifndef GL_VERSION_1_5
      GE_glGenBuffers = (GE_PFGLGENBUFFERS)
        getProcAddress("glGenBuffersARB");
      GE_glBindBuffer = (GE_PFGLBINDBUFFER)
        getProcAddress("glBindBufferARB");
      GE_glBufferData = (GE_PFGLBUFFERDATA)
        getProcAddress("glBufferDataARB");
      GE_glBufferSubData = (GE_PFGLBUFFERSUBDATA)
        getProcAddress("glBufferSubDataARB");

      if (GE_glGenBuffers==NULL || GE_glBindBuffer==NULL ||
          GE_glBufferData==NULL || GE_glBufferSubData==NULL)
        hasVertexBufferObjects = false;
      #endif
    
    }else{ hasVertexBufferObjects = false; }
    
    /*
    Check shader objects
    *****************************************/
    
    if (checkExtension(ext, "GL_ARB_shader_objects") &&
        checkExtension(ext, "GL_ARB_vertex_program") &&
        checkExtension(ext, "GL_ARB_fragment_program")) {
      hasShaderObjects = true;
      
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
      GE_glUniform1fv = (GE_PFGLUNIFORM1FV)
        getProcAddress ("glUniform1fvARB");
      GE_glUniform2fv = (GE_PFGLUNIFORM2FV)
        getProcAddress ("glUniform2fvARB");
      GE_glUniform3fv = (GE_PFGLUNIFORM3FV)
        getProcAddress ("glUniform3fvARB");
      GE_glUniform4fv = (GE_PFGLUNIFORM4FV)
        getProcAddress ("glUniform4fvARB");
      GE_glUniformMatrix4fv = (GE_PFGLUNIFORMMATRIX4FV)
        getProcAddress ("glUniformMatrix4fvARB");
      GE_glBindAttribLocation = (GE_PFGLBINDATTRIBLOCATION)
        getProcAddress ("glBindAttribLocationARB");
      GE_glGetAttribLocation = (GE_PFGLGETATTRIBLOCATION)
        getProcAddress ("glGetAttribLocationARB");
      GE_glVertexAttribPointer = (GE_PFGLVERTEXATTRIBPOINTER)
        getProcAddress ("glVertexAttribPointerARB");
      GE_glDisableVertexAttribArray = (GE_PFGLDISABLEVERTEXATTRIBARRAY)
        getProcAddress ("glDisableVertexAttribArrayARB");
      GE_glEnableVertexAttribArray = (GE_PFGLENABLEVERTEXATTRIBARRAY)
        getProcAddress ("glEnableVertexAttribArrayARB");
      GE_glGetShaderiv = (GE_PFGLGETSHADERIV)
        getProcAddress ("glGetShaderiv");
      GE_glGetShaderInfoLog = (GE_PFGLGETSHADERINFOLOG)
        getProcAddress ("glGetShaderInfoLog");
      GE_glGetProgramiv = (GE_PFGLGETPROGRAMIV)
        getProcAddress ("glGetProgramiv");
      GE_glGetProgramInfoLog = (GE_PFGLGETPROGRAMINFOLOG)
        getProcAddress ("glGetProgramInfoLog");
        
      
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
          GE_glUniform3f==NULL || GE_glUniform4f==NULL ||
          GE_glUniform1fv==NULL || GE_glUniform2fv==NULL ||
          GE_glUniform3fv==NULL || GE_glUniform4fv==NULL ||
          GE_glUniformMatrix4fv==NULL || GE_glBindAttribLocation==NULL ||
          GE_glGetAttribLocation==NULL || GE_glVertexAttribPointer==NULL ||
          GE_glDisableVertexAttribArray==NULL || GE_glEnableVertexAttribArray==NULL)
        hasShaderObjects = false;
      #endif
      
    }else{ hasShaderObjects = false; }

    /*
    Check framebuffer objects
    *****************************************/

    if (checkExtension(ext, "GL_EXT_framebuffer_object")) {
      hasFramebufferObjects = true;
      
      #ifndef GL_EXT_framebuffer_object
      GE_glGenFramebuffers = (GE_PFGLGENFRAMEBUFFERS)
        getProcAddress ("glGenFramebuffersEXT");
      GE_glBindFramebuffer = (GE_PFGLBINDFRAMEBUFFER)
        getProcAddress ("glBindFramebufferEXT");
      GE_glDeleteFramebuffers = (GE_PFGLDELETEFRAMEBUFFERS)
        getProcAddress ("glDeleteFramebuffersEXT");
      GE_glCheckFramebufferStatus = (GE_PFGLCHECKFRAMEBUFFERSTATUS)
        getProcAddress ("glCheckFramebufferStatusEXT");
      GE_glFramebufferTexture2D = (GE_PFGLFRAMEBUFFERTEXTURE2D)
        getProcAddress ("glFramebufferTexture2DEXT");
      GE_glFramebufferRenderbuffer = (GE_PFGLFRAMEBUFFERRENDERBUFFER)
        getProcAddress ("glFramebufferRenderbufferEXT");
      GE_glIsRenderbuffer = (GE_PFGLISRENDERBUFFER)
        getProcAddress ("glIsRenderbufferEXT");
      GE_glBindRenderbuffer = (GE_PFGLBINDRENDERBUFFER)
        getProcAddress ("glBindRenderbufferEXT");
      GE_glDeleteRenderbuffers = (GE_PFGLDELETERENDERBUFFERS)
        getProcAddress ("glDeleteRenderbuffersEXT");
      GE_glGenRenderbuffers = (GE_PFGLGENRENDERBUFFERS)
        getProcAddress ("glGenRenderbuffersEXT");
      GE_glRenderbufferStorage = (GE_PFGLRENDERBUFFERSTORAGE)
        getProcAddress ("glRenderbufferStorageEXT");

      if (GE_glGenFramebuffers==NULL || GE_glBindFramebuffer==NULL ||
          GE_glBindFramebuffer==NULL || GE_glDeleteFramebuffers==NULL ||
          GE_glCheckFramebufferStatus==NULL || GE_glFramebufferTexture2D==NULL ||
          GE_glFramebufferRenderbuffer==NULL || GE_glIsRenderbuffer==NULL ||
          GE_glBindRenderbuffer==NULL || GE_glDeleteRenderbuffers==NULL ||
          GE_glGenRenderbuffers==NULL || GE_glRenderbufferStorage==NULL)
        hasFramebufferObjects = false;
      #endif

    }else{ hasFramebufferObjects = false; }

    /*
    Check multiple render targets
    *****************************************/

    if (checkExtension( ext, "GL_ARB_draw_buffers" )) {
      hasMultipleRenderTargets = true;
      glGetIntegerv( GL_MAX_DRAW_BUFFERS, &maxRenderTargets );

      #ifndef GL_ARB_draw_buffers
      GE_glDrawBuffers = (GE_PFGLDRAWBUFFERS)
        getProcAddress( "glDrawBuffersARB" );

      if (GE_glDrawBuffers==NULL)
        hasMultipleRenderTargets = false;
      #endif

    }else{ hasMultipleRenderTargets = false; }

    /*
    Check for packed depth-stencil format
    *****************************************/

    if (checkExtension( ext, "GL_EXT_packed_depth_stencil" )) {
      hasDepthStencilFormat = true;
    }else{ hasDepthStencilFormat = false; }

    /*
    Check for occlusion query
    *****************************************/

    if (checkExtension( ext, "GL_ARB_occlusion_query" )) {
      hasOcclusionQuery = true;

      #ifndef GL_ARB_occlusion_query
      GE_glGenQueries = (GE_PFGLGENQUERIES)
        getProcAddress( "glGenQueriesARB" );
      GE_glDeleteQueries = (GE_PFGLDELETEQUERIES)
        getProcAddress( "glDeleteQueriesARB" );
      GE_glIsQuery = (GE_PFGLISQUERY)
        getProcAddress( "glIsQueryARB" );
      GE_glBeginQuery = (GE_PFGLBEGINQUERY)
        getProcAddress( "glBeginQueryARB" );
      GE_glEndQuery = (GE_PFGLENDQUERY)
        getProcAddress( "glEndQueryARB" );
      GE_glGetQueryiv = (GE_PFGLGETQUERYIV)
        getProcAddress( "glGetQueryivARB" );
      GE_glGetQueryObjectiv = (GE_PFGLGETQUERYOBJECTIV)
        getProcAddress( "glGetQueryObjectivARB" );
      GE_glGetQueryObjectuiv = (GE_PFGLGETQUERYOBJECTUIV)
        getProcAddress( "glGetQueryObjectuivARB" );

      if (GE_glGenQueries==NULL || GE_glDeleteQueries==NULL ||
          GE_glIsQuery==NULL || GE_glBeginQuery==NULL ||
          GE_glEndQuery==NULL || GE_glGetQueryiv==NULL ||
          GE_glGetQueryObjectiv==NULL || GE_glGetQueryObjectuiv==NULL)
        hasOcclusionQuery = false;
      #endif

      maxOcclusionBits = 0;
      if (hasOcclusionQuery)
        GE_glGetQueryiv( GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS, &maxOcclusionBits );

    }else{ hasOcclusionQuery = false; }

    /*
    Check for vertex array object
    *****************************************/

    if (checkExtension( ext, "GL_ARB_vertex_array_object" )) {
      hasVertexArrayObjects = true;

      #ifndef GL_ARB_vertex_array_object
      GE_glBindVertexArray = (GE_PFGLBINDVERTEXARRAY)
        getProcAddress( "glBindVertexArray" );
      GE_glDeleteVertexArrays = (GE_PFGLDELETEVERTEXARRAYS)
        getProcAddress( "glDeleteVertexArrays" );
      GE_glGenVertexArrays = (GE_PFGLGENVERTEXARRAYS)
        getProcAddress( "glGenVertexArrays" );
      GE_glIsVertexArray = (GE_PFGLISVERTEXARRAY)
        getProcAddress( "glIsVertexArray" );

      if (GE_glBindVertexArray==NULL || GE_glIsVertexArray==NULL ||
          GE_glGenVertexArrays==NULL || GE_glDeleteVertexArrays==NULL)
        hasVertexArrayObjects = false;
      #endif

    }else{ hasVertexArrayObjects = false; }

    /*
    Check vertical sync control
    *****************************************/
    
    #if defined(WIN32)
    glSwapInterval = (GE_PFGLSWAPINTERVAL)
      getProcAddress("wglSwapIntervalEXT");
    #endif
    
    /*
    Report
    *****************************************/
    printf( "hasMultitexture: %s\n", (hasMultitexture ? "true" : "false" ));
    printf( "hasShaderObjects: %s\n", (hasShaderObjects ? "true" : "false" ));
    printf( "hasFramebufferObjects: %s\n", (hasFramebufferObjects ? "true" : "false" ));
    printf( "hasVertexBufferObjects: %s\n", (hasVertexBufferObjects ? "true" : "false" ));
    printf( "hasMultipleRenderTargets: %s\n", (hasMultipleRenderTargets ? "true" : "false" ));
    printf( "hasDepthStencilFormat: %s\n", (hasDepthStencilFormat ? "true" : "false" ));
    printf( "hasRangeElements: %s\n", (hasRangeElements ? "true" : "false" ));
    printf( "hasOcclusionQuery: %s\n", (hasOcclusionQuery ? "true" : "false" ));
    printf( "maxRenderTargets: %d\n", maxRenderTargets );
    printf( "maxElementsVertices: %d\n", maxElementsVertices );
    printf( "maxElementsIndices: %d\n", maxElementsIndices );
    printf( "maxOcclusionBits: %d\n", maxOcclusionBits);
  }
  
  /*
  --------------------------------------------------
  Constructor for the single instance
  --------------------------------------------------*/
  
  Kernel::Kernel()
  {
    //Only allow one kernel
    assert(Kernel::Instance == NULL);
    Kernel::Instance = this;
    
    //Load extensions
    this->loadExtensions();
    
    //Create renderer
    renderer = new Renderer;

    //Time
    timeInit = false;
    time = 0.0f;
    dtime = 0.0f;
  }
  
  Kernel::~Kernel()
  {
    delete renderer;
  }
  /*
  void* Kernel::spawnFromPackage (ClassPtr cn)
  {
    IClass::FromFile (cn);
  }
  */
  void* Kernel::spawn (Class cn)
  {
    void *obj = cn->instantiate();
    objects.pushBack( (Object*) obj );
    return obj;
  }
  
  void* Kernel::spawn (const char *classString)
  {/*
    Class cn = ClassFromString (classString);
    if (cn == NULL) return NULL;
    return spawn (cn);
    */
    return NULL;
  }
  
  void Kernel::enableVerticalSync(bool on)
  {
    if (glSwapInterval)
      glSwapInterval(on ? 1 : 0);
  }

  Renderer* Kernel::getRenderer ()
  {
    return renderer;
  }

  void Kernel::tick (Float t)
  {
    if (timeInit)
      dtime = t - time;

    time = t;
    timeInit = true;
  }

  Float Kernel::getTime ()
  {
    return time;
  }

  Float Kernel::getInterval()
  {
    return dtime;
  }

  
  /*
  --------------------------------------------------
  Scene handling
  --------------------------------------------------*/

  bool loadPackageFile (ByteString &data, const CharString &filename)
  {
    //Open the file
    File file( filename );
    if (!file.open( FileAccess::Read, FileCondition::MustExist )) {
      std::cout << "Failed opening file " << filename.buffer() << "!" << std::endl;
      return false; }

    //Check the file signature
    Serializer s;
    if (file.getSize() < s.getSignatureSize()) {
      file.close(); std::cout << "Invalid file " << filename.buffer() << "!" << std::endl;
      return false; }

    ByteString sig = file.read( s.getSignatureSize() );
    if ((UintSize) sig.length() < s.getSignatureSize()) {
      file.close(); std::cout << "Invalid file " << filename.buffer() << "!" << std::endl;
      return false; }

    if ( ! s.checkSignature( sig.buffer() )) {
      file.close(); std::cout << "Invalid file " << filename.buffer() << "!" << std::endl;
      return false; }

    //Read the rest of the file
    file.read( data, file.getSize() - s.getSignatureSize() );
    if (data.length() == 0) {
      file.close(); std::cout << "Invalid file " << filename.buffer() << "!" << std::endl;
      return false; }

    //Close the file
    file.close();
    return true;
  }

  void Kernel::cacheResource (Resource *res, const CharString &name)
  {
    resources[ name.buffer() ] = res;
    res->setResourceName( name );
  }
  
  Resource* Kernel::getResource (const CharString &name)
  {
    //Search for the resource in the cache
    ResourceIter iter = resources.find( name );
    if (iter != resources.end()) return iter->second;

    //Load missing resource
    std::cout << "Loading resource " << name.buffer() << "..." << std::endl;

    if (name.right(3) == "jpg" ||
        name.right(3) == "png")
    {
      //Load image
      Image img;
      if (img.readFile( name ) != IMAGE_NO_ERROR) {
        std::cout << "Failed loading texture '" << name.buffer() << "'!" << std::endl;
        return NULL;
      }

      //Create texture resource
      Texture *tex = new Texture;
      tex->fromImage( &img );

      //Store resource in cache
      cacheResource( tex, name );
      return tex;
    }
    else
    {
      //Load file
      ByteString data;
      if (!loadPackageFile( data, "Meshes\\" + name ))
        return NULL;

      //Deserialize resource
      Serializer s;
      Resource *res = (Resource*) s.deserialize( data.buffer(), data.length() );

      //Send meshes to GPU
      TriMesh *mesh = Class::SafeCast< TriMesh >( res );
      if (mesh != NULL) mesh->sendToGpu();

      //Send character meshes to GPU
      Character *character = Class::SafeCast< Character >( res );
      if (character != NULL) {
        for (UintSize m=0; m<character->meshes.size(); ++m)
          character->meshes[ m ]->sendToGpu();
      }

      //Store resource in cache
      cacheResource( res, name );
      return res;
    }
  }


  struct MeshOutput
  {
    TriMesh *mesh;
    TriMeshActor *actor;
    MultiMaterial *material;
  };

  struct MergeVertex
  {
    Vector3 *normal;
    Vector3 *coord;

    void bind (VertexBinding<MergeVertex> *b)
    {
      b->bind( &normal, ShaderData::Normal );
      b->bind( &coord, ShaderData::Coord3 );
    }
  };

  void mergeMeshes (Scene3D *scene)
  {
    scene->updateChanges();

    //Output meshes and actors
    ArrayList< MeshOutput > outputs;
    
    //Walk the list of mesh actors
    ArrayList< Actor* > actors;
    scene->findActorsByClass( ClassName( TriMeshActor ), actors );
    for (UintSize a=0; a<actors.size(); ++a)
    {
      //Get source mesh from the actor
      TriMeshActor *srcActor = (TriMeshActor*) actors[ a ];
      TriMesh *srcMesh = srcActor->getMesh();
      if (srcMesh == NULL) continue;

      //Get material from the actor
      Material *srcMat = srcActor->getMaterial();
      if (srcMat == NULL) continue;

      //Find an existing output with matching mesh format
      MeshOutput out; bool found = false;
      for (UintSize o=0; o<outputs.size(); ++o) {
        if (*outputs[ o ].mesh->getFormat() == *srcMesh->getFormat()) {
          out = outputs[ o ];
          found = true;
          break;
        }}

      //Create new one if not found
      if (!found)
      {
        out.mesh = new TriMesh;
        out.actor = new TriMeshActor;
        out.material = new MultiMaterial;
        outputs.pushBack( out );

        //Match format to source mesh;
        out.mesh->setFormat( *srcMesh->getFormat() );
        out.actor->setMesh( out.mesh );
        out.actor->setMaterial( out.material );
      }

      //Copy materials
      MaterialID startMat = (MaterialID) out.material->getNumSubMaterials();
      if (ClassOf( srcMat ) == ClassName( MultiMaterial )) {

        MultiMaterial* multiMat = (MultiMaterial*) srcMat;
        out.material->setNumSubMaterials( startMat + multiMat->getNumSubMaterials() );

        for (MaterialID m=0; m<multiMat->getNumSubMaterials(); ++m) {
          Material *subMat = multiMat->getSubMaterial( m );
          out.material->setSubMaterial( startMat + m, subMat );
        }
      }else
      {
        out.material->setNumSubMaterials( startMat + 1 );
        out.material->setSubMaterial( startMat, srcMat );
      }

      //Copy index groups, offsetting by last existing index and material
      VertexID startIndex = (VertexID) out.mesh->indices.size();
      for (UintSize g=0; g<srcMesh->groups.size(); ++g)
      {
        TriMesh::IndexGroup grp = srcMesh->groups[ g ];
        grp.start += startIndex;
        grp.materialID += startMat;
        out.mesh->groups.pushBack( grp );
      }

      //Copy mesh indices, offsetting by last existing vertex
      VertexID startVertex = (VertexID) out.mesh->getVertexCount();
      for (UintSize i=0; i<srcMesh->indices.size(); ++i)
        out.mesh->indices.pushBack( startVertex + srcMesh->indices[ i ] );

      //Get the world transform from the actor
      //(TODO: should exclude the root transform as the new actor
      //will be placed under root too. For now we know all the
      //actors are direct children of root.
      Matrix4x4 worldMat = srcActor->getMatrix();

      //Walk the list of vertices
      VertexBinding< MergeVertex > vertBind;
      vertBind.init( srcMesh->getFormat() );
      for (UintSize v=0; v<srcMesh->getVertexCount(); ++v)
      {
        //Init vertex by copying all the data
        void *vertData = srcMesh->getVertex(v);
        MergeVertex srcVert = vertBind( vertData  );
        MergeVertex outVert = vertBind( out.mesh->addVertex( vertData ) );
        
        //Modify normal and coord by multiplying with the world transform
        if (srcVert.coord == NULL || outVert.coord == NULL) continue;
        if (srcVert.normal == NULL || outVert.normal == NULL) continue;
        *outVert.coord = worldMat.transformPoint( *srcVert.coord );
        *outVert.normal = worldMat.transformVector( *srcVert.normal );
      }

      //Remove the actor from the scene
      srcActor->setParent( NULL );
    }

    //Walk the list of output meshes
    for (UintSize o=0; o<outputs.size(); ++o)
    {
      //Add output actors to the scene
      MeshOutput &out = outputs[ o ];
      scene->getRoot()->addChild( out.actor );

      //Send mesh to GPU
      out.mesh->sendToGpu();
    }
  }

  Scene3D* Kernel::loadSceneData (const void *data, UintSize size)
  {
    //Deserialize data
    Serializer s;
    Object *obj = s.deserialize( data, size );
    
    //Check if Scene3D loaded
    Scene3D *scene = Class::SafeCast< Scene3D >( obj );
    if (scene == NULL) {
      std::cout << "Invalid file content!" << std::endl;
      return NULL;
    }
/*
    //DEBUG: Replace textured materials with simple ones
    ArrayList< Actor* > actors;
    scene->updateChanges();
    scene->findActorsByClass( Class(TriMeshActor), actors );
    for (UintSize a=0; a<actors.size(); ++a)
    {
      StandardMaterial *m = new StandardMaterial;
      m->setDiffuseColor( Vector3( .6, .6, .6 ) );
      
      TriMeshActor *meshActor = (TriMeshActor*) actors[a];
      meshActor->setMaterial( m );
    }
*/
    //Cache resources loaded with the scene
    for (UintSize r=0; r<scene->resources.size(); ++r)
    {
      //Store resource in cache
      Resource *res = scene->resources[ r ];
      cacheResource( res, res->getResourceName() );

      //Send meshes to GPU
      TriMesh *mesh = Class::SafeCast< TriMesh >( res );
      if (mesh != NULL) mesh->sendToGpu();

      //Send character meshes to GPU
      Character *character = Class::SafeCast< Character >( res );
      if (character != NULL) {
        for (UintSize m=0; m<character->meshes.size(); ++m)
          character->meshes[ m ]->sendToGpu();
      }
    }

    //Assign resources / load missing
    const ArrayList< Object* > &objects = s.getObjects();
    for (UintSize o=0; o<objects.size(); ++o)
    {
      Object *obj = objects.at(o);
      if (ClassOf( obj ) == ClassName( ResourceRef ))
      {
        ResourceRef *ref = (ResourceRef*) obj;
        ref->ptr = getResource( ref->name );
      }
    }

    //Invoke loaded events
    for (UintSize o=0; o<objects.size(); ++o) {
      Actor3D *actor = Class::SafeCast< Actor3D >( objects.at(o) );
      if (actor != NULL) actor->onResourcesLoaded();
    }

    //mergeMeshes( scene );

    //Update the scene
    scene->updateChanges();
    return scene;
  }

  Scene3D* Kernel::loadSceneFile (const CharString &filename)
  {
    //Load from file
    ByteString data;
    if (!loadPackageFile( data, filename ))
      return NULL;
    
    //Process data
    return loadSceneData( data.buffer(), data.length() );
  }

}//namespace GE
