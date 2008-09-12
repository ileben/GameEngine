#ifndef __GEGLHEADERS_H
#define __GEGLHEADERS_H

#if defined(__APPLE__)
#  include <OpenGL/gl.h>
#  include <OpenGL/glu.h>
#  include <GLUT/glut.h>
#
#else
#
#  define GL_GLEXT_LEGACY //dont define extensions for us
#  include <GL/gl.h>
#  include <GL/glu.h>
#  include <GL/glut.h>
#
#  if !defined (WIN32)
#    include <GL/glx.h>
#  endif
#
#endif

#ifndef APIENTRY
#  define APIENTRY
#endif

typedef void (*PFVOID) ();

/*******************************************************
GLX_VERSION_1_4
********************************************************/

#if !defined(WIN32) && !defined(__APPLE__)
#include <dlfcn.h>
typedef PFVOID (*GE_PFGLXGETPROCADDRESS) (const GLubyte *);
#endif

/*******************************************************
GL_EXT_separate_specular_color
********************************************************/

#ifndef GL_EXT_separate_specular_color
#define GL_LIGHT_MODEL_COLOR_CONTROL      0x81F8
#define GL_SEPARATE_SPECULAR_COLOR        0x81FA
#endif

/*******************************************************
GL_VERSION_1_2
********************************************************/

#ifndef GL_VERSION_1_2
#define GL_MAX_ELEMENTS_VERTICES          0x80E8
#define GL_MAX_ELEMENTS_INDICES           0x80E9
#endif

#ifndef GL_VERSION_1_2
typedef void
  (APIENTRY* GE_PFGLDRAWRANGEELEMENTS)
  (GLenum, GLuint, GLuint, GLsizei, GLenum, const GLvoid *);
#endif

/*******************************************************
GL_VERSION_1_3
********************************************************/

#ifndef GL_VERSION_1_3
#define GL_MULTISAMPLE                    0x809D
#define GL_TEXTURE0                       0x84C0
#define GL_TEXTURE1                       0x84C1
#endif

#ifndef GL_VERSION_1_3

typedef void
  (APIENTRY* GE_PFGLACTIVETEXTURE)
  (GLenum);

typedef void
  (APIENTRY* GE_PFGLMULTITEXCOORD2F)
  (GLenum, GLfloat, GLfloat);

#endif

/*******************************************************
GL_ARB_shader_objects
********************************************************/

#ifndef GL_ARB_shader_objects
#define GL_OBJECT_COMPILE_STATUS_ARB      0x8B81
#define GL_OBJECT_LINK_STATUS_ARB         0x8B82
#define GL_OBJECT_INFO_LOG_LENGTH_ARB     0x8B84
typedef char                              GLcharARB;
typedef unsigned int                      GLhandleARB;
#endif

#ifndef GL_VERSION_2_0
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
typedef char                              GLchar;
#endif

/*******************************************************
GL_VERSION_2_0
********************************************************/

#ifndef GL_VERSION_2_0

typedef GLuint
  (APIENTRY* GE_PFGLCREATEPROGRAM)
  (void);

typedef GLuint
  (APIENTRY* GE_PFGLCREATESHADER)
  (GLenum);

typedef void
  (APIENTRY* GE_PFGLATTACHSHADER)
  (GLuint, GLuint);

typedef void
  (APIENTRY* GE_PFGLSHADERSOURCE)
  (GLuint, GLsizei, const GLchar* *, const GLint *);

typedef void
  (APIENTRY* GE_PFGLCOMPILESHADER)
  (GLuint);

typedef void
  (APIENTRY* GE_PFGLLINKPROGRAM)
  (GLuint);

typedef void
  (APIENTRY* GE_PFGLUSEPROGRAM)
  (GLuint);

typedef void
  (APIENTRY* GE_PFGLDETACHSHADER)
  (GLuint, GLuint);

typedef void
  (APIENTRY* GE_PFGLDELETESHADER)
  (GLuint);

typedef void
  (APIENTRY* GE_PFGLDELETEPROGRAMSARB)
  (GLsizei, const GLuint *);

typedef void
  (APIENTRY* GE_PFGLGETINFOLOGARB)
  (GLhandleARB, GLsizei, GLsizei *, GLcharARB *);

typedef void
  (APIENTRY* GE_PFGLGETOBJECTPARAMETERIVARB)
  (GLhandleARB, GLenum, GLint *);

typedef GLint
  (APIENTRY* GE_PFGLGETUNIFORMLOCATION)
  (GLuint, const GLchar *);

typedef void
  (APIENTRY* GE_PFGLUNIFORM1I)
  (GLint, GLint);

typedef void
  (APIENTRY* GE_PFGLUNIFORM2I)
  (GLint, GLint, GLint);

typedef void
  (APIENTRY* GE_PFGLUNIFORM3I)
  (GLint, GLint, GLint, GLint);

typedef void
  (APIENTRY* GE_PFGLUNIFORM4I)
  (GLint, GLint, GLint, GLint, GLint);

typedef void
  (APIENTRY* GE_PFGLUNIFORM1F)
  (GLint, GLfloat);

typedef void
  (APIENTRY* GE_PFGLUNIFORM2F)
  (GLint, GLfloat, GLfloat);

typedef void
  (APIENTRY* GE_PFGLUNIFORM3F)
  (GLint, GLfloat, GLfloat, GLfloat);

typedef void
  (APIENTRY* GE_PFGLUNIFORM4F)
  (GLint, GLfloat, GLfloat, GLfloat, GLfloat);

#endif

/*******************************************************
Function re-routing
********************************************************/

#ifndef GL_VERSION_1_2
extern GE_PFGLDRAWRANGEELEMENTS         GE_glDrawRangeElements;
#endif

#ifndef GL_VERSION_1_3
extern GE_PFGLACTIVETEXTURE             GE_glActiveTexture;
extern GE_PFGLMULTITEXCOORD2F           GE_glMultiTexCoord2f;
#endif

#ifndef GL_VERSION_2_0
extern GE_PFGLCREATEPROGRAM             GE_glCreateProgram;
extern GE_PFGLCREATESHADER              GE_glCreateShader;
extern GE_PFGLATTACHSHADER              GE_glAttachShader;
extern GE_PFGLSHADERSOURCE              GE_glShaderSource;
extern GE_PFGLCOMPILESHADER             GE_glCompileShader;
extern GE_PFGLLINKPROGRAM               GE_glLinkProgram;
extern GE_PFGLUSEPROGRAM                GE_glUseProgram;
extern GE_PFGLDETACHSHADER              GE_glDetachShader;
extern GE_PFGLDELETESHADER              GE_glDeleteShader;
extern GE_PFGLDELETEPROGRAMSARB         GE_glDeleteProgramsARB;
extern GE_PFGLGETINFOLOGARB             GE_glGetInfoLogARB;
extern GE_PFGLGETOBJECTPARAMETERIVARB   GE_glGetObjectParameterivARB;
extern GE_PFGLGETUNIFORMLOCATION        GE_glGetUniformLocation;
extern GE_PFGLUNIFORM1I                 GE_glUniform1i;
extern GE_PFGLUNIFORM2I                 GE_glUniform2i;
extern GE_PFGLUNIFORM3I                 GE_glUniform3i;
extern GE_PFGLUNIFORM4I                 GE_glUniform4i;
extern GE_PFGLUNIFORM1F                 GE_glUniform1f;
extern GE_PFGLUNIFORM2F                 GE_glUniform2f;
extern GE_PFGLUNIFORM3F                 GE_glUniform3f;
extern GE_PFGLUNIFORM4F                 GE_glUniform4f;
#endif


#ifndef GE_NO_EXTENSION_ROUTING

#ifndef GL_VERSION_1_2
#define glDrawRangeElements          GE_glDrawRangeElements
#endif

#ifndef GL_VERSION_1_3
#define glActiveTexture              GE_glActiveTexture
#define glMultiTexCoord2f            GE_glMultiTexCoord2f
#endif

#ifndef GL_VERSION_2_0
#define glCreateProgram              GE_glCreateProgram
#define glCreateShader               GE_glCreateShader
#define glAttachShader               GE_glAttachShader
#define glShaderSource               GE_glShaderSource
#define glCompileShader              GE_glCompileShader
#define glLinkProgram                GE_glLinkProgram
#define glUseProgram                 GE_glUseProgram
#define glDetachShader               GE_glDetachShader
#define glDeleteShader               GE_glDeleteShader
#define glDeleteProgramsARB          GE_glDeleteProgramsARB
#define glDeleteProgram(p)          {GE_glDeleteProgramsARB(1, &p);}
#define glGetInfoLogARB              GE_glGetInfoLogARB
#define glGetObjectParameterivARB    GE_glGetObjectParameterivARB
#define glGetUniformLocation         GE_glGetUniformLocation
#define glUniform1i                  GE_glUniform1i
#define glUniform2i                  GE_glUniform2i
#define glUniform3i                  GE_glUniform3i
#define glUniform4i                  GE_glUniform4i
#define glUniform1f                  GE_glUniform1f
#define glUniform2f                  GE_glUniform2f
#define glUniform3f                  GE_glUniform3f
#define glUniform4f                  GE_glUniform4f
#endif

#endif//!GE_NO_EXTENSION_ROUTING



/*******************************************************
Vertical sync control
/*******************************************************/

typedef void (APIENTRY *GE_PFGLSWAPINTERVAL) (GLint);
extern GE_PFGLSWAPINTERVAL glSwapInterval;


/*******************************************************
Internal GL renderer parameters
/*******************************************************/

#define _GL_MAX_SHININESS 128


#endif /* __GEGLHEADERS_H */
