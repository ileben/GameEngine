#include "geShader.h"
#include "geShaders.h"
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
      if (vertex->compile ())
        printf ("Vertex shader compiled.\n");
      else printf ("Failed compiling vertex shader!");

      CharString infoLog = vertex->getInfoLog ();
      if (infoLog.length() > 0)
        printf ("Info Log:\n%s\n", infoLog.buffer());
    }
    
    //Load fragment source
    if (!fragment->fromFile( fileFragment )) {
      printf( "Failed loading fragment shader from '%s'\n",
        fileFragment.toCSTR().buffer() );
    }
    else
    {
      //Compile fragment shader
      if (fragment->compile())
        printf ("Fragment shader compiled.\n");
      else printf ("Failed compiling fragment shader!\n");

      CharString infoLog = fragment->getInfoLog ();
      if (infoLog.length() > 0)
        printf ("Info Log:\n%s\n", infoLog.buffer());
    }

    //Link shading program
    program->attach( vertex );
    program->attach( fragment );
    
    if (program->link())
      printf ("Shading program linked.\n");
    else printf ("Failed linking shading program!\n");

    CharString infoLog = program->getInfoLog ();
    if (infoLog.length() > 0)
      printf ("Info Log:\n%s\n", infoLog.buffer());

    //Query attribute locations
    for (UintSize a=0; a<attribs.size(); ++a)
      attribs[a].ID = program->getAttribute( attribs[a].name.buffer());

    //Query uniform locations
    for (UintSize u=0; u<uniforms.size(); ++u)
      uniforms[u].ID = program->getUniform( uniforms[u].name.buffer() );
  }

  void Shader::registerVertexAttrib (const CharString &name)
  {
    attribs.pushBack( VertexAttrib( name ));
  }
  
  void Shader::registerUniform (const CharString &name,
                                UniformType type,
                                int count)
  {
    uniforms.pushBack( Uniform( name, type, count ));
  }

  Int32 Shader::getVertexAttribID (UintSize index)
  {
    if (index > attribs.size()) return -1;
    return attribs[ index ].ID;
  }

  Int32 Shader::getVertexAttribID (const CharString &name)
  {
    for (UintSize a=0; a<attribs.size(); ++a)
      if (attribs[a].name == name)
        return attribs[a].ID;

    return -1;
  }

  Int32 Shader::getUniformID (UintSize index)
  {
    if (index > uniforms.size()) return -1;
    return uniforms[ index ].ID;
  }

  Int32 Shader::getUniformID (const CharString &name)
  {
    for (UintSize u=0; u<uniforms.size(); ++u)
      if (uniforms[u].name == name)
        return uniforms[u].ID;

    return -1;
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

  const GLProgram* Shader::getGLProgram()
  {
    return program;
  }
  
}//namespace GE
