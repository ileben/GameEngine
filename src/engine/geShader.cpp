#include "geShader.h"
#include "geShaders.h"
#include "geGLHeaders.h"
#include "io/geFile.h"
#include "engine/embedit/shadenode.Shader.embedded"

namespace GE
{
  DEFINE_CLASS (Shader);
  
  
  Shader::Shader ()
  {
    program = NULL;
    newShaderNode = NULL;
  }
  
  Shader::~Shader ()
  {
    detachShaders();
    deleteShaders();
    deleteProgram();
  }
  
  void Shader::deleteProgram ()
  {
    if (program != NULL)
    { 
      delete program;
      program = NULL;
    }
  }

  void Shader::deleteShaders ()
  {
    for (UintSize v=0; v<vertShaders.size(); ++v)
      delete vertShaders[v];
    vertShaders.clear();

    for (UintSize f=0; f<fragShaders.size(); ++f)
      delete fragShaders[f];
    fragShaders.clear();
  }

  void Shader::attachShaders ()
  {
    if (program == NULL) return;

    for (UintSize v=0; v<vertShaders.size(); ++v)
      program->attach( vertShaders[v] );
    for (UintSize f=0; f<fragShaders.size(); ++f)
      program->attach( fragShaders[f] );
  }

  void Shader::detachShaders ()
  {
    if (program == NULL) return;

    for (UintSize v=0; v<vertShaders.size(); ++v)
      program->detach( vertShaders[v] );
    for (UintSize f=0; f<fragShaders.size(); ++f)
      program->detach( fragShaders[f] );
  }

  bool Shader::compile (ShaderType::Enum target, const CharString &source)
  {
    //Make sure old shaders are detached
    detachShaders();

    //Create a new shader
    GLShader *shader = new GLShader;
    shader->create( target );

    //Add to shader list
    switch (target) {
    case ShaderType::Vertex:
      vertShaders.pushBack( shader ); break;
    case ShaderType::Fragment:
      fragShaders.pushBack( shader ); break; }

    //Compile the source
    bool status = shader->compile( source );

    if (status)
      printf( "%s shader compiledd.\n",
        target == ShaderType::Vertex ? "Vertex" : "Fragment" );
    else
      printf( "Failed compiling %s shader!",
        target == ShaderType::Vertex ? "vertex" : "fragment" );

    CharString infoLog = shader->getInfoLog ();
    if (infoLog.length() > 0)
      printf ("Info Log:\n%s\n", infoLog.buffer());

    return status;
  }

  bool Shader::link()
  {
    //Create a new program
    deleteProgram();
    program = new GLProgram;
    program->create();

    //Attach all the shaders and link the program
    attachShaders();
    bool status = program->link();

    if (status)
      printf ("Shading program linkedd.\n");
    else printf ("Failed linking shading program!\n");

    CharString infoLog = program->getInfoLog ();
    if (infoLog.length() > 0)
      printf ("Info Log:\n%s\n", infoLog.buffer());

    //Query attribute locations
    for (UintSize a=0; a<attribs.size(); ++a)
      attribs[a].ID = program->getAttribute( attribs[a].name.buffer() );

    //Query uniform locations
    for (UintSize u=0; u<uniforms.size(); ++u)
      uniforms[u].ID = program->getUniform( uniforms[u].name.buffer() );

    return status;
  }

  bool Shader::fromString (const CharString &strVertex,
                           const CharString &strFragment)
  {
    //Compile and link the source

    if (!compile( ShaderType::Vertex, strVertex ))
      return false;

    if (!compile( ShaderType::Fragment, strFragment ))
      return false;

    if (!link())
      return false;

    return true;
  }

  bool Shader::fromFile (const String &fileNameVertex,
                         const String &fileNameFragment)
  {
    //Open the vertex file
    File fileVertex( fileNameVertex );
    if (!fileVertex.open( FileAccess::Read, FileCondition::MustExist )) {
      printf( "Failed loading vertex shader from '%s'\n",
        fileNameVertex.toCSTR().buffer() );
      return false;
    }

    //Open the fragment file
    File fileFragment( fileNameFragment );
    if (!fileFragment.open( FileAccess::Read, FileCondition::MustExist )) {
      printf( "Failed loading vertex shader from '%s'\n",
        fileNameVertex.toCSTR().buffer() );
      return false;
    }

    //Load vertex and fragment source
    CharString srcVertex = fileVertex.read( fileVertex.getSize() );
    CharString srcFragment = fileFragment.read( fileFragment.getSize() );
    fileVertex.close();
    fileFragment.close();

    //Init from string
    return fromString( srcVertex, srcFragment );
  }

  /*
  --------------------------------------
  Shader composing
  --------------------------------------*/

  DataUnit DataUnit::Float( DataType::Float, 1 );
  DataUnit DataUnit::Vec2( DataType::Float, 2 );
  DataUnit DataUnit::Vec3( DataType::Float, 3 );
  DataUnit DataUnit::Vec4( DataType::Float, 4 );

  DataUnit DataUnit::Int( DataType::Int, 1 );
  DataUnit DataUnit::IVec2( DataType::Int, 2 );
  DataUnit DataUnit::IVec3( DataType::Int, 3 );
  DataUnit DataUnit::IVec4( DataType::Int, 4 );

  DataUnit DataUnit::Uint( DataType::Uint, 1 );
  DataUnit DataUnit::UVec2( DataType::Uint, 2 );
  DataUnit DataUnit::UVec3( DataType::Uint, 3 );
  DataUnit DataUnit::UVec4( DataType::Uint, 4 );

  DataUnit DataUnit::Mat3( DataType::Matrix, 3 );
  DataUnit DataUnit::Mat4( DataType::Matrix, 4 );
  DataUnit DataUnit::Sampler2D( DataType::Sampler2D, 1 );

  CharString DataUnit::toString()
  {
    if (count > 1)
    {
      switch (type) {
      case DataType::Int: return "ivec" + CharString::FInt(count);
      case DataType::Float: return "vec" + CharString::FInt(count);
      case DataType::Matrix: return "mat" + CharString::FInt(count);
      default: return "invalidtype";
      }
    }
    else
    {
      switch (type) {
      case DataType::Int: return "int";
      case DataType::Float: return "float";
      case DataType::Sampler2D: return "sampler2D";
      }
    }
    
    return "invalidtype";
  }

  void Shader::Socket::resolveKnownData()
  {
    builtInAccess[ ShaderType::Vertex ] = false;
    builtInAccess[ ShaderType::Fragment ] = false;

    switch (data)
    {
    case ShaderData::Coord:
      name = "Coord";
      unit = DataUnit::Vec4;
      builtInAccess[ ShaderType::Vertex ] = true;
      break;
    case ShaderData::TexCoord:
      name = "TexCoord" + CharString::FInt(index);
      unit = DataUnit::Vec4;
      builtInAccess[ ShaderType::Vertex ] = true;
      break;
    case ShaderData::Normal:
      name = "Normal";
      unit = DataUnit::Vec3;
      builtInAccess[ ShaderType::Vertex ] = true;
      break;
    case ShaderData::Diffuse:
      name = "Diffuse";
      unit = DataUnit::Vec4;
      builtInAccess[ ShaderType::Vertex ] = true;
      builtInAccess[ ShaderType::Fragment ] = true;
      break;
    case ShaderData::Specular:
      name = "Specular";
      unit = DataUnit::Vec4;
      builtInAccess[ ShaderType::Vertex ] = true;
      builtInAccess[ ShaderType::Fragment ] = true;
      break;
    case ShaderData::SpecularExp:
      name = "SpecularExp";
      unit = DataUnit::Float;
      builtInAccess[ ShaderType::Vertex ] = true;
      builtInAccess[ ShaderType::Fragment ] = true;
      break;
    default:
      break;
    }
  }

  CharString Shader::Socket::getInitString()
  {
    CharString strIndex = CharString::FInt(index);
    switch (data)
    {
    case ShaderData::Coord:
      return "= gl_Vertex";
    case ShaderData::TexCoord:
      return "= gl_MultiTexCoord" + strIndex;
    case ShaderData::Normal:
      return "= gl_Normal";
    case ShaderData::Diffuse:
      return "= gl_FrontMaterial.diffuse";
    case ShaderData::Specular:
      return "= gl_FrontMaterial.specular";
    case ShaderData::SpecularExp:
      return "= gl_FrontMaterial.shininess";
    default:
      return "";
    }
  }

  void Shader::composeNodeNew (ShaderType::Enum location)
  {
    if (newShaderNode != NULL)
      delete newShaderNode;

    newShaderNode = new Node;
    newShaderNode->location = location;
  }

  void Shader::composeNodeCode (const CharString &code)
  {
    if (newShaderNode == NULL) return;
    newShaderNode->code = code;
  }

  void Shader::composeNodeSocket (SocketFlow::Enum flow,
                                  ShaderData::Enum data,
                                  DataSource::Enum source,
                                  const DataUnit &unit,
                                  const CharString &name,
                                  int index)
  {
    if (newShaderNode == NULL) return;

    switch (flow)
    {
    case SocketFlow::In:
      newShaderNode->inSocks.pushBack( Socket( data, source, unit, name, index ) );
      break;
    case SocketFlow::Out:
      newShaderNode->outSocks.pushBack( Socket( data, source, unit, name, index ) );
      break;
    }
  }

  void Shader::composeNodeSocket (SocketFlow::Enum flow,
                                  ShaderData::Enum data,
                                  DataSource::Enum source,
                                  int index)
  {
    if (newShaderNode == NULL) return;

    switch (flow)
    {
    case SocketFlow::In:
      newShaderNode->inSocks.pushBack( Socket( data, source, index ) );
      break;
    case SocketFlow::Out:
      newShaderNode->outSocks.pushBack( Socket( data, source, index ) );
      break;
    }
  }

  void Shader::composeNodeEnd ()
  {
    if (newShaderNode == NULL) return;
    
    switch (newShaderNode->location)
    {
    case ShaderType::Vertex:
      vertShaderNodes.pushFront( newShaderNode );
      break;
    case ShaderType::Fragment:
      fragShaderNodes.pushFront( newShaderNode );
      break;
    }
    
    newShaderNode = NULL;
  }

  Int32 Shader::registerVertexAttrib (const DataUnit &unit,
                                      const CharString &name)
  {
    attribs.pushBack( VertexAttrib( unit, name ));
    return (Int32) attribs.size()-1;
  }

  Int32 Shader::registerUniform (ShaderType::Enum location,
                                 const DataUnit &unit,
                                 const CharString &name,
                                 Uint32 count)
  {
    uniforms.pushBack( Uniform( location, unit, name, count ));
    return (Int32) uniforms.size()-1;
  }

  void output (CharString &code, int indent, const CharString &str)
  {
    for (int i=0; i<indent; ++i)
      code += "  ";
    code += str;
  }

  void outputLines (CharString &code, int indent, const CharString &str)
  {
    ArrayList<CharString> lines;
    str.tokenize( "\n", &lines );
    for (UintSize l=0; l<lines.size(); ++l)
      output( code, indent, lines[l]+"\n" );
  }

  CharString Shader::outputShader (LinkedList<Socket> *varying,
                                   LinkedList<Socket> *init,
                                   LinkedList<Socket> *done,
                                   LinkedList<Node*> *nodes,
                                   const CharString &code,
                                   ShaderType::Enum target)
  {
    int indent = 0;
    typedef LinkedList<Socket>::Iterator SocketIter;
    typedef LinkedList<Node*>::Iterator NodeIter;
    CharString shaderStr;
    int nID = 0;

    //Output uniforms
    for (UintSize u=0; u<uniforms.size(); ++u) {
      Uniform &su = uniforms[u];
      if (su.loc != target) continue;
      output( shaderStr, indent, "uniform " + su.unit.toString() + " " + su.name );
      if (su.count > 1) shaderStr += "[" + CharString::FInt( su.count ) + "];\n";
      else shaderStr += ";\n";
    }

    //Output attributes
    if (target == ShaderType::Vertex) {
      for (UintSize a=0; a<attribs.size(); ++a) {
        VertexAttrib &va = attribs[a];
        output( shaderStr, indent, "attribute " + va.unit.toString() + " " + va.name + ";\n" );
      }}

    //Output varying for each remaining socket requiring varying input
    if (varying != NULL) {
      for (SocketIter v=varying->begin(); v!=varying->end(); ++v)
        output( shaderStr, indent, "varying " + v->unit.toString() + " v" + v->name + ";\n");
    }

    //Output function definition for each node used
    for (NodeIter n=nodes->begin(); n!=nodes->end(); ++n)
    {
      UintSize arg=0;
      Node *node = *n;
      shaderStr += "\n";
      output( shaderStr, indent, "void node" + CharString::FInt(nID++) + " (" );
      for (UintSize i=0; i<node->inSocks.size(); ++i, ++arg)
      {
        if (arg>0) shaderStr += ", ";
        Socket &sock = node->inSocks[i];
        shaderStr += "in " + sock.unit.toString() + " in" + sock.name;
      }
      for (UintSize o=0; o<node->outSocks.size(); ++o, ++arg)
      {
        if (arg>0) shaderStr += ", ";
        Socket &sock = node->outSocks[o];
        shaderStr += "out " + sock.unit.toString() + " out" + sock.name;
      }
      shaderStr += ")\n";
      output( shaderStr, indent, "{\n" );
      outputLines( shaderStr, indent+1, node->code );
      output( shaderStr, indent, "}\n" );
    }

    //Start main
    shaderStr += "\n";
    output( shaderStr, indent, "void main()\n{\n" );
    indent += 1;

    //Ouput a simple declaration for satisfied sockets
    for (SocketIter d=done->begin(); d!=done->end(); ++d)
      output( shaderStr, indent, d->unit.toString() + " t" + d->name + ";\n");

    //Init unsatisfied socket variables with builtin, attribute or varying data
    if (init != NULL) {
      for (SocketIter i=init->begin(); i!=init->end(); ++i) {
        if (i->source == DataSource::BuiltIn && i->builtInAccess[ target ])
          output( shaderStr, indent, i->unit.toString() + " t" + i->name + " " + i->getInitString() + ";\n" );
        else if (i->source == DataSource::Attribute && target==ShaderType::Vertex)
          output( shaderStr, indent, i->unit.toString() + " t" + i->name + " = " + i->name + ";\n");
        else
          output( shaderStr, indent, i->unit.toString() + " t" + i->name + " = v" + i->name  + ";\n");
      }}

    shaderStr+= "\n";
    nID = 0;

    //Output function calls for each node used
    for (NodeIter n=nodes->begin(); n!=nodes->end(); ++n)
    {
      UintSize arg=0;
      Node *node = *n;
      output( shaderStr, indent, "node" + CharString::FInt(nID++) + "( " );
      for (UintSize i=0; i<node->inSocks.size(); ++i, ++arg)
      {
        if (arg>0) shaderStr += ", ";
        shaderStr += "t" + node->inSocks[i].name;
      }
      for (UintSize o=0; o<node->outSocks.size(); ++o, ++arg)
      {
        if (arg>0) shaderStr += ", ";
        shaderStr += "t" + node->outSocks[o].name;
      }
      shaderStr += " );\n";
    }

    //Insert additional code
    outputLines( shaderStr, indent, code );

    //Assign varying data to variables
    shaderStr += "\n";
    if (target != ShaderType::Fragment) {
      if (varying != NULL) {
        for (SocketIter v=varying->begin(); v!=varying->end(); ++v)
          output( shaderStr, indent, "v" + v->name + " = t" + v->name + ";\n");
      }}

    //End main
    indent -= 1;
    output( shaderStr, indent, "}\n" );

    return shaderStr;
  }

  void Shader::findNodesForSockets (LinkedList<Socket> *requiredSocks,
                                    LinkedList<Node*> *availableNodes,
                                    LinkedList<Socket> *satisfiedSocks,
                                    LinkedList<Node*> *usedNodes)
  {
    typedef LinkedList<Socket>::Iterator SocketIter;
    typedef LinkedList<Node*>::Iterator NodeIter;

    //Walk the available nodes (should be in reverse order of dependency)
    for (NodeIter n=availableNodes->begin(); n!=availableNodes->end(); ++n)
    {
      bool nodeUsed = false;

      //Walk the output sockets of this node
      for (UintSize o=0; o<(*n)->outSocks.size(); ++o)
      {
        //Walk the list of sockets that require data
        for (SocketIter s=requiredSocks->begin(); s!=requiredSocks->end(); ) {

          //Check if requirement satisfied
          if ((*n)->outSocks[o] == *s) {

            //Remove this socket requirement
            s = requiredSocks->removeAt( s );
            //Mark used
            nodeUsed = true;
          }
          else ++s;
        }
      }

      //Check if any node output used
      if (nodeUsed)
      {
        //Add the node to used list
        usedNodes->pushFront( *n );

        //Add new input sockets to required list
        for (UintSize i=0; i<(*n)->inSocks.size(); ++i)
          if (!requiredSocks->contains( (*n)->inSocks[i] ))
            requiredSocks->pushBack( (*n)->inSocks[i] );
      }
    }

    //Find all satisfied output sockets (i.e. not required anymore)
    for (NodeIter n=usedNodes->begin(); n!=usedNodes->end(); ++n) {
      for (UintSize o=0; o<(*n)->outSocks.size(); ++o)
        if (!requiredSocks->contains( (*n)->outSocks[o] ))
          satisfiedSocks->pushBack( (*n)->outSocks[o] );
    }
  }

  bool Shader::compose (RenderTarget::Enum target)
  {
    typedef LinkedList<Socket>::Iterator SocketIter;
    typedef LinkedList<Node*>::Iterator NodeIter;
    
    LinkedList<Socket> varying;

    LinkedList<Socket> fragSocks;
    LinkedList<Node*> fragUsedNodes;
    LinkedList<Socket> fragDoneSocks;

    LinkedList<Socket> vertSocks;
    LinkedList<Node*> vertUsedNodes;
    LinkedList<Socket> vertDoneSocks;

    //A default node that transforms normals with the normal matrix
    composeNodeNew( ShaderType::Vertex );
    composeNodeSocket( SocketFlow::In, ShaderData::Normal );
    composeNodeSocket( SocketFlow::Out, ShaderData::Normal );
    composeNodeCode( "outNormal = normalize( gl_NormalMatrix * inNormal );\n" );
    composeNodeEnd();

    //A default node that transforms vertices with the modelview matrix
    composeNodeNew( ShaderType::Vertex );
    composeNodeSocket( SocketFlow::In, ShaderData::Coord );
    composeNodeSocket( SocketFlow::Out, ShaderData::Coord );
    composeNodeCode( "outCoord = gl_ModelViewMatrix * inCoord;\n" );
    composeNodeEnd();

    //Final vertex code applies projection matrix to vertices
    CharString vertCode = "gl_Position  = gl_ProjectionMatrix * tCoord;\n";
    CharString fragCode;

    //Define the final data required by the renderer
    switch (target)
    {
    case RenderTarget::GBuffer:
      fragSocks.pushBack( Socket( ShaderData::Coord ));
      fragSocks.pushBack( Socket( ShaderData::Normal ));
      fragSocks.pushBack( Socket( ShaderData::Diffuse ));
      fragSocks.pushBack( Socket( ShaderData::Specular ));
      fragSocks.pushBack( Socket( ShaderData::SpecularExp ));
      fragCode = fragEndCodeGBuffer;

      break;
    case RenderTarget::ShadowMap:
      vertSocks.pushBack( Socket( ShaderData::Coord ));
      fragCode = fragEndCodeShadowMap;
      break;
    };

    //Find the fragment nodes that produce data for sockets
    findNodesForSockets( &fragSocks, &fragShaderNodes, &fragDoneSocks, &fragUsedNodes );

    //Pass required sockets to vertex if not accessible from fragment shader
    for (SocketIter fs=fragSocks.begin(); fs!=fragSocks.end(); ++fs) {
      if (!fs->builtInAccess[ ShaderType::Fragment ]) {
        if (!vertSocks.contains( *fs ))
          vertSocks.pushBack( *fs );
        if (!varying.contains( *fs))
          varying.pushBack( *fs );
      }}

    //Find the vertex nodes that produce data for sockets
    findNodesForSockets( &vertSocks, &vertShaderNodes, &fragDoneSocks, &vertUsedNodes );

    CharString vertShaderStr = outputShader
      ( &varying, &vertSocks, &fragDoneSocks, &vertUsedNodes, vertCode, ShaderType::Vertex );
    CharString fragShaderStr = outputShader
      ( &varying, &fragSocks, &fragDoneSocks, &fragUsedNodes, fragCode, ShaderType::Fragment );

    vertShaderNodes.clear();
    fragShaderNodes.clear();
    
    
    //printf( "-----------------------\nVertexShader:\n-----------------------\n");
    //printf( "%s\n", vertShaderStr.buffer() );
    //printf( "-----------------------\nFragmentShader:\n-----------------------\n");
    //printf( "%s\n", fragShaderStr.buffer() );
    

    return fromString( vertShaderStr, fragShaderStr );
  }

  UintSize Shader::getNumVertexShaders () {
    return vertShaders.size();
  }

  UintSize Shader::getNumFragmentShaders () {
    return fragShaders.size();
  }

  const GLShader* Shader::getVertex (UintSize index) {
    if (index > vertShaders.size()) return NULL;
    return vertShaders[ index ];
  }

  const GLShader* Shader::getFragment (UintSize index) {
    if (index > fragShaders.size()) return NULL;
    return fragShaders[ index ];
  }

  const GLProgram* Shader::getProgram() {
    return program;
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
  
}//namespace GE
