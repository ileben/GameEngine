#define GE_API_EXPORT
#include "geEngine.h"
#include "geGLHeaders.h"

namespace GE
{
  DEFINE_CLASS (Shader);
  
  
  Shader::Shader ()
  {
    vertex = NULL;
    fragment = NULL;
    program = NULL;
  }
  
  Shader::~Shader ()
  {
    freeProgram ();
  }
  
  void Shader::freeProgram ()
  {
    if (program != NULL)
    {
      program->detach (GE_SHADER_VERTEX);
      program->detach (GE_SHADER_FRAGMENT);
      delete program;
    }
    
    if (vertex != NULL)
      delete vertex;
    
    if (fragment != NULL)
      delete fragment;
  }
  
  void Shader::fromFile (const String &fileVertex,
                         const String &fileFragment)
  {
    freeProgram ();
    
    //Create program and shader objects
    program = new GLProgram;
    vertex = new GLShader;
    fragment = new GLShader;
    
    program->create ();
    vertex->create (GE_SHADER_VERTEX);
    fragment->create (GE_SHADER_FRAGMENT);
    
    //Load vertex source
    if (!vertex->fromFile( fileVertex )) {
      printf( "Failed loading vertex shader from '%s'\n",
        fileVertex.toCSTR().buffer() );
    }
    else
    {
      //Compile vertex shader
      if (vertex->compile ()) {
        printf ("Vertex shader compiled.\n");
      }else{
        printf ("Failed compiling vertex shader!");
        char *infoLog = vertex->getInfoLog ();
        if (infoLog != NULL) {
          printf ("Info Log:\n%s\n", infoLog);
          delete[] infoLog; }
      }
    }
    
    //Load fragment source
    if (!fragment->fromFile( fileFragment )) {
      printf( "Failed loading fragment shader from '%s'\n",
        fileFragment.toCSTR().buffer() );
    }
    else
    {
      //Compile fragment shader
      if (fragment->compile ()) {
        printf ("Fragment shader compiled.\n");
      }else{
        printf ("Failed compiling fragment shader!\n");
        char *infoLog = fragment->getInfoLog ();
        if (infoLog != NULL) {
          printf ("Info Log:\n%s\n", infoLog);
          delete[] infoLog; }
      }
    }

    //Link shading program
    program->attach (vertex);
    program->attach (fragment);
    if (program->link ()) {
      printf ("Shading program linked.\n");
    }else{
      printf ("Failed linking shading program!\n");
      char *infoLog = program->getInfoLog ();
      if (infoLog != NULL) {
        printf ("Info Log:\n%s\n", infoLog);
        delete[] infoLog; }
    }
  }
  
  void Shader::registerUniform (const String &name,
                                UniformType type,
                                int count)
  {
    uniforms.pushBack (Uniform (name, type, count));
  }
  
  UintSize Shader::getUniformCount()
  {
    return uniforms.size();
  }
  
  Shader::Uniform& Shader::getUniform( UintSize index )
  {
    return uniforms[ index ];
  }
  
  void Shader::use ()
  {
    if (program != NULL)
      program->use ();
  }
  
}//namespace GE
