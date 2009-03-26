#include "geShaders.h"
#include "geGLHeaders.h"

namespace GE
{
  DEFINE_CLASS (GLShader);
  DEFINE_CLASS (GLProgram);
  
  
  char* loadShaderFromFile (const char *filename)
  {
    FILE *f = fopen (filename, "rb");
    if (f == NULL) return NULL;
    
    fseek (f, 0, SEEK_END);
    int size = ftell (f);
    fseek (f, 0, SEEK_SET);
    
    char *shaderString = new char[size + 1];
    fread (shaderString, 1, size, f);
    shaderString[size] = 0;
    
    return shaderString;
  }
  
  char* getObjectInfoLog (GLhandleARB handle)
  {
    /*
    int infoSize = 0;
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &infoSize);
    
    char *infoLog = new char[infoSize+1];
    memset(infoLog, 0, infoSize+1);
    glGetShaderInfoLog(handle, infoSize, NULL, infoLog);
    return infoLog;*/
    
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

  /*==============================================
   *
   * Vertex or fragment shader
   *
   *==============================================*/

  GLShader::GLShader ()
  {
    this->type = GE_SHADER_INVALID;
    this->handle = 0;
  }

  GLShader::~GLShader ()
  {
    glDeleteShader (handle);
  }
  
  void GLShader::create (ShaderType type)
  {
    if (handle != 0) return;
    
    switch (type)
    {
    case GE_SHADER_VERTEX:
      handle = glCreateShader (GL_VERTEX_SHADER);
      break;
    case GE_SHADER_FRAGMENT:
      handle = glCreateShader (GL_FRAGMENT_SHADER);
      break;
    }
    
    this->type = type;
  }

  void GLShader::fromString (const char *code)
  {
    glShaderSource (handle, 1, &code, NULL);
  }

  bool GLShader::fromFile (const String &filename)
  {
    const char *source = loadShaderFromFile (filename.toCSTR().buffer());
    if (source == NULL) return false;
    fromString (source);
    delete source;
    return true;
  }

  bool GLShader::compile ()
  {
    GLint status = 0;
    glCompileShader (handle);
    
    //return true; //TODO: glGetObjectParameterivARB doesn't work on linux
    //glGetObjectParameterivARB ((GLhandleARB)handle,
    //                           GL_OBJECT_COMPILE_STATUS_ARB, &status);
    //return (status == 1 ? true : false);

    glGetShaderiv( handle, GL_COMPILE_STATUS, &status );
    return (status == GL_TRUE);
  }

  CharString GLShader::getInfoLog ()
  {
    //return getObjectInfoLog ((GLhandleARB)handle);

    GLint len = 0;
    glGetShaderiv( handle, GL_INFO_LOG_LENGTH, &len);
    if (len == 0) return CharString();

    char *clog = new char[len+1];
    glGetShaderInfoLog( handle, len+1, NULL, clog );
    clog[len] = '\0';

    return CharString(clog);
  }

  /*==========================================
   *
   * Shading program
   *
   *==========================================*/

  GLProgram::GLProgram ()
  {
    vertex = NULL;
    fragment = NULL;
  }

  GLProgram::~GLProgram ()
  {
    detach (GE_SHADER_VERTEX);
    detach (GE_SHADER_FRAGMENT);
    glDeleteProgram (handle);
  }
  
  void GLProgram::create ()
  {
    handle = glCreateProgram ();
  }

  void GLProgram::detach (ShaderType which)
  {
    switch (which)
    {
    case GE_SHADER_VERTEX:
      if (vertex != NULL)
        glDetachShader (handle, vertex->handle);
      break;
      
    case GE_SHADER_FRAGMENT:
      if (fragment != NULL)
        glDetachShader (handle, fragment->handle);
      break;
    }
  }

  void GLProgram::attach (GLShader *s)
  {
    detach (s->type);
    
    switch (s->type)
    {
    case GE_SHADER_VERTEX:
      glAttachShader (handle, s->handle);
      vertex = s;
      break;

    case GE_SHADER_FRAGMENT:
      glAttachShader (handle, s->handle);
      fragment = s;
      break;
    }
  }
  
  GLShader* GLProgram::getVertex ()
  {
    return vertex;
  }
  
  GLShader* GLProgram::getFragment ()
  {
    return fragment;
  }
  
  bool GLProgram::link ()
  {
    GLint status;
    glLinkProgram (handle);

    //return true; //TODO: glGetObjectParameterivARB doesn't work on linux
    //glGetObjectParameterivARB ((GLhandleARB) handle,
    //                          GL_OBJECT_LINK_STATUS_ARB, &status);    
    //return (status1 == 1 ? true : false);
    
    glGetProgramiv( handle, GL_LINK_STATUS, &status );
    return (status == GL_TRUE);
  }
  
  CharString GLProgram::getInfoLog ()
  {
    //return getObjectInfoLog( (GLhandleARB)handle );

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
    //TODO: the following gl call throws an exception if the
    //uniform with given name is not found in the program
    return glGetUniformLocation( handle, name );
  }
  
}/* namespace GE */
