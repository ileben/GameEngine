#include "geShaders.h"
#include "geGLHeaders.h"

namespace GE
{
  DEFINE_CLASS (GLShader);
  DEFINE_CLASS (GLProgram);
  
  /*** Obsolete

  char* getObjectInfoLog (GLhandleARB handle)
  {
    GLint infoSize = 0;
    glGetObjectParameterivARB ((GLhandleARB)handle,
                               GL_OBJECT_INFO_LOG_LENGTH_ARB, &infoSize);
    
    char *infoLog = NULL;
    if (infoSize > 0) {
      infoLog = new char[infoSize+1];
      memset(infoLog, 0 ,infoSize+1);
      glGetInfoLogARB ((GLhandleARB)handle, infoSize, NULL, (GLchar*) infoLog);
    }
    
    return infoLog;
  }
  */

  /*** Obsolete

  char* loadShaderFromFile (const char *filename)
  {
    FILE *f = std::fopen( filename, "rb" );
    if (f == NULL) return NULL;
    
    std::fseek( f, 0, SEEK_END );
    int size = std::ftell( f );
    std::fseek( f, 0, SEEK_SET );
    
    char *shaderString = new char[size + 1];
    std::fread( shaderString, 1, size, f );
    shaderString[ size ] = 0;
    
    return shaderString;
  }
  */

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
