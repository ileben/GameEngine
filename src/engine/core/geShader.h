#ifndef __GE_SHADER_H
#define __GE_SHADER_H

#include "util/geUtil.h"
#include "geResource.h"
#include "geShaders.h"
#include "geRenderer.h"

#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{
  /*
  ------------------------------------
  Forward declarations
  ------------------------------------*/
  class GLShader;
  class GLProgram;

  /*
  ------------------------------------
  Shader
  ------------------------------------*/
  
  namespace DataType {
    enum Enum
    {
      Uint,
      Int,
      Float,
      Matrix,
      Sampler2D
    };};

  struct DataUnit
  {
    DataType::Enum type;
    int count;

    DataUnit ()
      { }
    DataUnit (DataType::Enum t, int c)
      { type=t; count=c; }
    bool operator== (const DataUnit &u) const
      { return (type == u.type && count == u.count); }
    CharString toString();

    static DataUnit Float;
    static DataUnit Vec2;
    static DataUnit Vec3;
    static DataUnit Vec4;

    static DataUnit Int;
    static DataUnit IVec2;
    static DataUnit IVec3;
    static DataUnit IVec4;

    static DataUnit Uint;
    static DataUnit UVec2;
    static DataUnit UVec3;
    static DataUnit UVec4;

    static DataUnit Mat3;
    static DataUnit Mat4;
    static DataUnit Sampler2D;
  };

  namespace DataSource {
    enum Enum
    {
      None,
      BuiltIn,
      Attribute
    };}

  namespace SocketFlow {
    enum Enum
    {
      In,
      Out
    };}

  namespace ShaderData {
    enum Enum
    {
      Custom,
      Coord2,
      Coord3,
      Coord4,
      TexCoord1,
      TexCoord2,
      TexCoord3,
      Normal,
      Diffuse,
      Specular,
      SpecularExp,
      JointIndex,
      JointWeight,
      Tangent,
      Bitangent
    };}
  
  class Shader : public Resource
  {
    CLASS( Shader, Resource,
      d3c5406f,0544,429e,bb11e4d70e95e11e );
    
  private:

    friend class Material;

    struct VertexAttrib
    {
      CharString name;
      DataUnit unit;
      Int32 ID;

      VertexAttrib () {};
      VertexAttrib (const DataUnit &u, const CharString &n )
        { unit = u; name = n; }
    };

    struct Uniform
    {
      ShaderType::Enum loc;
      CharString name;
      DataUnit unit;
      Uint32 count;
      Int32 ID;
      
      Uniform () {};
      Uniform (ShaderType::Enum l, const DataUnit &u, const CharString &n, Uint32 c=1 )
        { loc = l; unit = u; name = n; count = c; }
    };

    struct Socket
    {
      ShaderData::Enum data;
      DataSource::Enum source;
      DataUnit unit;
      CharString name;
      int index;
      bool builtInAccess[2];

      Socket() : index( -1 ) {}

      Socket( ShaderData::Enum d, int i=-1 ) {
        source = DataSource::Attribute;
        data = d;
        index = i; 
        resolveKnownData();
      }
      Socket( const DataUnit &u, const CharString &n, int i=-1 ) {
        source = DataSource::Attribute;
        data = ShaderData::Custom;
        unit = u;
        name = n;
        index = i;
        resolveKnownData();
      }
      bool operator== (const Socket &s)
      {
        //Data type must match
        if (data != s.data)
          return false;
        //If not custom, match by index
        if (data != ShaderData::Custom)
          return (index == s.index);
        //Custom have to be matched by all properties
        return (name == s.name && unit == s.unit && index == s.index);
      }
      void resolveKnownData();
      CharString getInitString();
    };

    struct Node
    {
      ShaderType::Enum location;
      ArrayList< Socket > inSocks;
      ArrayList< Socket > outSocks;
      CharString code;
    };

    Node *newShaderNode;
    LinkedList< Node* > vertShaderNodes;
    LinkedList< Node* > fragShaderNodes;
    ArrayList<Uniform> uniforms;
    ArrayList<VertexAttrib> attribs;

    void findNodesForSockets (LinkedList<Socket> *requiredSocks,
                              LinkedList<Node*> *availableNodes,
                              LinkedList<Socket> *satisfiedSocks,
                              LinkedList<Node*> *usedNodes);

    CharString outputShader (LinkedList<Socket> *varying,
                             LinkedList<Socket> *init,
                             LinkedList<Socket> *done,
                             LinkedList<Node*> *nodes,
                             const CharString &code,
                             ShaderType::Enum target);
    
    ArrayList< GLShader* > vertShaders;
    ArrayList< GLShader* > fragShaders;
    GLProgram *program;
    void attachShaders ();
    void detachShaders ();
    void deleteShaders ();
    void deleteProgram ();
    
  public:
    
    Shader ();
    virtual ~Shader ();

    //Use this before any method of shader compilation
    Int32 registerVertexAttrib (const DataUnit &unit,
                                const CharString &name);

    Int32 registerUniform (ShaderType::Enum location,
                           const DataUnit &unit,
                           const CharString &name,
                           Uint32 count=1);

    //Used for automatic shader compositing
    void composeNodeNew (ShaderType::Enum location);
    void composeNodeCode (const CharString &code);
    void composeNodeEnd ();

    void composeNodeSocket (SocketFlow::Enum flow,
                            ShaderData::Enum data,
                            int index=-1);

    void composeNodeSocket (SocketFlow::Enum flow,
                            const DataUnit &unit,
                            const CharString &name,
                            int index=-1 );

    bool compose (RenderTarget::Enum target);

    //Used for linking of multiple vertex/fragment sources
    bool compile (ShaderType::Enum target,
                  const CharString &source);
    bool link();

    //Used for single vertex/fragment source case
    bool fromString (const CharString &strVertex,
                     const CharString &strFragment);

    bool fromFile (const String &fileVertex,
                   const String &fileFragment);

    UintSize getNumVertexShaders ();
    UintSize getNumFragmentShaders ();
    const GLShader* getVertex (UintSize index);
    const GLShader* getFragment (UintSize index);
    const GLProgram* getProgram ();

    UintSize getVertexAttribCount ();
    Int32 getVertexAttribID (UintSize index);
    Int32 getVertexAttribID (const CharString &name);
    Int32 getUniformID (UintSize index);
    Int32 getUniformID (const CharString &name);
    
    UintSize getUniformCount ();
    Uniform& getUniform (UintSize index);
    void use ();
  };
}

#pragma warning(pop)
#endif //__GE_SHADER_H
