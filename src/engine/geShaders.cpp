#include "geShaders.h"
#include "geGLHeaders.h"

namespace GE
{
  DEFINE_CLASS (GLShader);
  DEFINE_CLASS (GLProgram);

  /*==============================================
   *
   * Vertex or fragment shader
   *
   *==============================================*/

  GLShader::GLShader ()
  {
    type = ShaderType::Any;
    handle = 0;
  }

  GLShader::~GLShader ()
  {
    glDeleteShader( handle );
  }
  
  void GLShader::create (ShaderType::Enum type)
  {
    if (handle != 0) return;
    
    switch (type)
    {
    case ShaderType::Vertex:
      handle = glCreateShader( GL_VERTEX_SHADER );
      break;
    case ShaderType::Fragment:
      handle = glCreateShader( GL_FRAGMENT_SHADER );
      break;
    }
    
    this->type = type;
  }

  bool GLShader::compile (const CharString &source)
  {
    const GLchar *src = source.buffer();
    glShaderSource (handle, 1, &src, NULL);

    GLint status = 0;
    glCompileShader( handle );
    glGetShaderiv( handle, GL_COMPILE_STATUS, &status );
    return (status == GL_TRUE);
  }

  CharString GLShader::getInfoLog ()
  {
    GLint len = 0;
    glGetShaderiv( handle, GL_INFO_LOG_LENGTH, &len);
    if (len == 0) return CharString();

    char *clog = new char[len+1];
    glGetShaderInfoLog( handle, len+1, NULL, clog );
    clog[len] = '\0';
    
    CharString log( clog );
    delete[] clog;
    return log;
  }

  /*==========================================
   *
   * Shading program
   *
   *==========================================*/

  GLProgram::GLProgram ()
  {
  }

  GLProgram::~GLProgram ()
  {
    glDeleteProgram( handle );
  }
  
  void GLProgram::create ()
  {
    handle = glCreateProgram ();
  }

  void GLProgram::attach (GLShader *s)
  {
    switch (s->type)
    {
    case ShaderType::Vertex:
      glAttachShader( handle, s->handle );
      break;

    case ShaderType::Fragment:
      glAttachShader( handle, s->handle );
      break;
    }
  }

  void GLProgram::detach (GLShader *s)
  {
    switch (s->type)
    {
    case ShaderType::Vertex:
      glDetachShader( handle, s->handle );
      break;
      
    case ShaderType::Fragment:
      glDetachShader( handle, s->handle );
      break;
    }
  }
  
  bool GLProgram::link ()
  {
    GLint status;
    glLinkProgram (handle);
    glGetProgramiv( handle, GL_LINK_STATUS, &status );
    return (status == GL_TRUE);
  }
  
  CharString GLProgram::getInfoLog ()
  {
    GLint len = 0;
    glGetProgramiv( handle, GL_INFO_LOG_LENGTH, &len);
    if (len == 0) return CharString();

    char *clog = new char[len+1];
    glGetProgramInfoLog( handle, len+1, NULL, clog );
    clog[len] = '\0';

    return CharString(clog);
  }
  
  void GLProgram::use ()
  {
    glUseProgram( handle );
  }
  
  void GLProgram::UseFixed ()
  {
    glUseProgram( 0 );
  }

  void GLProgram::bindAttribute (Uint32 index, const char *name)
  {
    glBindAttribLocation( handle, index, name );
  }

  Int32 GLProgram::getAttribute (const char *name) const
  {
    return GE_glGetAttribLocation( handle, name );
  }
  
  Int32 GLProgram::getUniform (const char *name) const
  {
    return glGetUniformLocation( handle, name );
  }
  
}/* namespace GE */
