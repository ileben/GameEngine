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
      { type=DataType::Int; count=1; }
    DataUnit (DataType::Enum t, int c)
      { type=t; count=c; }
    bool operator== (const DataUnit &u)
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
      Coord,
      TexCoord,
      Normal,
      Diffuse,
      Specular,
      SpecularExp,
      Tangent,
      Bitangent,
      Attribute
    };}
  
  class GE_API_ENTRY Shader : public Resource
  {
    DECLARE_SUBCLASS (Shader, Resource); DECLARE_END;
    friend class Material;
    
  private:

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

      Socket()
        : index( -1 ) {}
      Socket( ShaderData::Enum d, DataSource::Enum s = DataSource::BuiltIn, int i=-1 )
        { data = d; source = s; index = i; resolveKnownData(); }
      Socket( ShaderData::Enum d, DataSource::Enum s, const DataUnit &u, const CharString &n, int i=-1 )
        { data = d; source = s; unit = u; name = n; index = i; resolveKnownData(); }
      bool operator== (const Socket &s)
      {
        if (data != s.data)
          return false;
        if (source == DataSource::BuiltIn && s.source == DataSource::BuiltIn)
          return (index == s.index);
        else
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
    
    GLShader  *vertex;
    GLShader  *fragment;
    GLProgram *program;
    ArrayList<Uniform> uniforms;
    ArrayList<VertexAttrib> attribs;
    
    void freeProgram ();
    
  public:
    
    Shader ();
    virtual ~Shader ();

    Int32 registerVertexAttrib (const DataUnit &unit,
                                const CharString &name);

    Int32 registerUniform (ShaderType::Enum location,
                           const DataUnit &unit,
                           const CharString &name,
                           Uint32 count=1);

    void composeNodeNew (ShaderType::Enum location);
    void composeNodeCode (const CharString &code);
    void composeNodeEnd ();

    void composeNodeSocket (SocketFlow::Enum flow,
                            ShaderData::Enum data,
                            DataSource::Enum source = DataSource::BuiltIn,
                            int index=-1);

    void composeNodeSocket (SocketFlow::Enum flow,
                            ShaderData::Enum data,
                            DataSource::Enum source,
                            const DataUnit &unit,
                            const CharString &name,
                            int index=-1 );

    bool compose (RenderTarget::Enum target);


    Int32 getVertexAttribID (UintSize index);
    Int32 getVertexAttribID (const CharString &name);
    Int32 getUniformID (UintSize index);
    Int32 getUniformID (const CharString &name);

    bool fromString (const CharString &strVertex,
                     const CharString &strFragment);

    bool fromFile (const String &fileVertex,
                   const String &fileFragment);
    
    UintSize getUniformCount ();
    Uniform& getUniform (UintSize index);
    const GLProgram* getGLProgram ();
    void use ();
  };
}

#pragma warning(pop)
#endif //__GE_SHADER_H
