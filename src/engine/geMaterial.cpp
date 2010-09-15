#include "geKernel.h"
#include "geMaterial.h"
#include "geTexture.h"
#include "geShaders.h"
#include "geShader.h"
#include "geGLHeaders.h"

namespace GE
{
  DEFINE_SERIAL_CLASS( Material,         ClassID( 0xc3598aadu, 0x4bca, 0x455d, 0x99c9d3f6b15e4948ull ));
  DEFINE_SERIAL_CLASS( StandardMaterial, ClassID( 0x619518d9u, 0x5540, 0x4e9e, 0x85dedc0cbb70b480ull ));
  DEFINE_SERIAL_CLASS( MultiMaterial,    ClassID( 0x53fe780du, 0x23ea, 0x4fdb, 0x844d901a4ac5be39ull ));
  DEFINE_SERIAL_CLASS( DiffuseTexMat,    ClassID( 0x2cbb66fdu, 0x4ce1, 0x4e01, 0x8b6765c546c5bbccull ));
  DEFINE_SERIAL_CLASS( NormalTexMat,     ClassID( 0xdc2c8562u, 0xfe36, 0x4903, 0x9c494f8976323411ull ));
  
  /*
  ============================================
  
  Material base
  
  ============================================*/
  
  void Material::BeginDefault()
  {
    //Use fixed functionality
    GLProgram::UseFixed ();
    
    //Default is opaque-white
    glColor3f (1, 1, 1);
    glDisable (GL_BLEND);
    glEnable (GL_LIGHTING);
    glEnable (GL_DEPTH_TEST);
    glEnable (GL_COLOR_MATERIAL);

    //Make sure texture unit 0 is active
    glActiveTexture (GL_TEXTURE0);
  }
  
  void Material::EndDefault ()
  {
  }
  
  
  /*
  ============================================
  
  Standard material (fixed functionality)
  
  ============================================*/

  StandardMaterial::StandardMaterial()
  {
    ambientColor.set (0, 0, 0);
    diffuseColor.set (1, 1, 1);
    specularColor.set (1, 1, 1);
    specularity = 0.0f;
    glossiness = 0.5f;
    opacity = 1.0f;
    luminosity = 0.0f;
    lighting = true;
    culling = true;
    cell = false;
    wire = false;

    gotUniforms = false;
  }

  StandardMaterial::StandardMaterial (SM *sm)
  {
    wire = false;
    gotUniforms = false;
  }

  void StandardMaterial::setDiffuseColor(const Vector3 &color) {
    diffuseColor = color;
  }

  const Vector3& StandardMaterial::getDiffuseColor() {
    return diffuseColor;
  }

  void StandardMaterial::setAmbientColor(const Vector3 &color) {
    ambientColor = color;
  }

  const Vector3& StandardMaterial::getAmbientColor() {
    return ambientColor;
  }

  void StandardMaterial::setSpecularColor(const Vector3 &color) {
    specularColor = color;
  }

  const Vector3& StandardMaterial::getSpecularColor() {
    return specularColor;
  }

  void StandardMaterial::setOpacity(Float opacity) {
    this->opacity = opacity;
  }

  Float StandardMaterial::getOpacity() {
    return opacity;
  }
  
  void StandardMaterial::setSpecularity(Float spec) {
    specularity = spec;
  }

  Float StandardMaterial::getSpecularity() {
    return specularity;
  }

  void StandardMaterial::setGlossiness(Float gloss) {
    glossiness = gloss;
  }

  Float StandardMaterial::getGlossiness() {
    return glossiness;
  }

  void StandardMaterial::setLuminosity (Float l) {
    luminosity = l;
  }

  Float StandardMaterial::getLuminosity () {
    return luminosity;
  }

  void StandardMaterial::setUseLighting(bool enable) {
    lighting = enable;
  }

  bool StandardMaterial::getUseLighting() {
    return lighting;
  }

  void StandardMaterial::setCullBack(bool enable) {
    culling = enable;
  }

  bool StandardMaterial::getCullBack() {
    return culling;
  }

  void StandardMaterial::setCellShaded (bool enable) {
    cell = enable;
  }

  bool StandardMaterial::getCellShaded () {
    return cell;
  }

  void StandardMaterial::setWireframe (bool enable) {
    wire = enable;
  }

  bool StandardMaterial::getWireframe () {
    return wire;
  }

  void StandardMaterial::composeShader( Shader *shader )
  {
    shader->registerUniform( ShaderType::Fragment, DataUnit::Float, "uLuminosity" );
    shader->registerUniform( ShaderType::Fragment, DataUnit::Float, "uSpecularity" );
    shader->registerUniform( ShaderType::Fragment, DataUnit::Float, "uCellShading" );
  }
  
  void StandardMaterial::begin ()
  {
    Material::begin ();

    Shader *shader = Kernel::GetInstance()->getRenderer()->getCurrentShader();
    if (!gotUniforms)
    {
      uLuminosity = shader->getUniformID( "uLuminosity" );
      uSpecularity = shader->getUniformID( "uSpecularity" );
      uCellShading = shader->getUniformID( "uCellShading" );
      gotUniforms = true;
    }

    //We will set material color manually (not via glColor)
    glDisable (GL_COLOR_MATERIAL);

    //Luminosity
    glUniform1f( uLuminosity, luminosity );
    
    //Ambient color
    Vector4 ambient = ambientColor.xyz (1.0f);
    glMaterialfv( GL_FRONT_AND_BACK, GL_AMBIENT, (Float*) &ambient );
    
    //Diffuse color
    Vector4 diffuse = diffuseColor.xyz (opacity);
    glMaterialfv( GL_FRONT_AND_BACK, GL_DIFFUSE, (Float*) &diffuse );

    //We want specularity to be aplied to texture colors too
    GLint param = GL_SEPARATE_SPECULAR_COLOR;
    glLightModeliv( GL_LIGHT_MODEL_COLOR_CONTROL, &param );     

    //Specularity
    glUniform1f( uSpecularity, specularity );
      
    //Specular color
    Vector4 spec = (specularColor).xyz(1);
    glMaterialfv( GL_FRONT_AND_BACK, GL_SPECULAR, (Float*) &spec );
    
    //GL uses an integer value for maximum glossiness
    int shininess = (int)(glossiness * _GL_MAX_SHININESS);
    glMateriali( GL_FRONT_AND_BACK, GL_SHININESS, shininess );
    
    /*
    //Blending
    bool blend = false;
    
    if (opacity < 1.0f)
      blend = true;
    
    if (blend) {
      glEnable (GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }else glDisable (GL_BLEND);
    */

    /*
    //Lighting
    if (lighting)
      glEnable (GL_LIGHTING);
    else
      glDisable (GL_LIGHTING);
    */

    //Back-face culling
    if(culling) {
      glEnable (GL_CULL_FACE);
    }else glDisable (GL_CULL_FACE);

    //Wirefrace
    if (wire)
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    
    //Normalize all normals so
    //we can freely scale actors
    glEnable( GL_NORMALIZE );

    //Lighting model
    glUniform1f( uCellShading, cell ? 1.0f : 0.0f );
  }

  void StandardMaterial::end()
  {
    if (wire)
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
  }
  
  /*
  ============================================
  
  Uses texture for diffuse color
  
  ============================================*/

  DiffuseTexMat::DiffuseTexMat ()
  {
    texDiffuse = NULL;
    gotUniforms = false;
  }

  DiffuseTexMat::DiffuseTexMat (SM *sm)
    : StandardMaterial (sm), texDiffuse (sm)
  {
    gotUniforms = false;
  }

  void DiffuseTexMat::setDiffuseTexture (Texture *tex) {
    texDiffuse = tex;
  }

  void DiffuseTexMat::setDiffuseTexture (const CharString &name) {
    texDiffuse = name;
  }

  Texture* DiffuseTexMat::getDiffuseTexture () {
    return texDiffuse;
  }

  void DiffuseTexMat::composeShader (Shader *shader)
  {
    StandardMaterial::composeShader( shader );

    shader->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "diffSampler" );

    shader->composeNodeNew( ShaderType::Fragment );
    shader->composeNodeSocket( SocketFlow::In, ShaderData::TexCoord2 );
    shader->composeNodeSocket( SocketFlow::Out, ShaderData::Diffuse );
    shader->composeNodeCode( "outDiffuse = texture2D( diffSampler, inTexCoord2 );\n" );
    shader->composeNodeEnd();
  }

  void DiffuseTexMat::begin()
  {
    StandardMaterial::begin();

    Shader *shader = Kernel::GetInstance()->getRenderer()->getCurrentShader();
    if (!gotUniforms)
    {
      uDiffSampler = shader->getUniformID( "diffSampler" );
      gotUniforms = true;
    }

    glUniform1i( uDiffSampler, 0 );
    if (texDiffuse != NULL)
    {
      glActiveTexture( GL_TEXTURE0 );
      glBindTexture( GL_TEXTURE_2D, texDiffuse->getHandle() );
      glEnable( GL_TEXTURE_2D );
    }
  }

  void DiffuseTexMat::end()
  {
    StandardMaterial::end();

    if (texDiffuse != NULL)
    {
      glActiveTexture( GL_TEXTURE0 );
      glDisable( GL_TEXTURE_2D );
    }
  }

  /*
  ============================================
  
  Uses texture for normal mapping
  
  ============================================*/

  NormalTexMat::NormalTexMat()
  {
    texNormal = NULL;
    gotUniforms = false;
  }

  NormalTexMat::NormalTexMat (SM *sm)
    : DiffuseTexMat(sm), texNormal (sm)
  {
    gotUniforms = false;
  }

  void NormalTexMat::setNormalTexture (Texture *tex) {
    texNormal = tex;
  }

  void NormalTexMat::setNormalTexture (const CharString &name) {
    texNormal = name;
  }

  Texture* NormalTexMat::getNormalTexture () {
    return texNormal;
  }

  void NormalTexMat::composeShader (Shader *shader)
  {
    //StandardMaterial::composeShader( shader );
    DiffuseTexMat::composeShader( shader );

    shader->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "normSampler" );

    shader->composeNodeNew( ShaderType::Vertex );
    shader->composeNodeSocket( SocketFlow::In, ShaderData::Tangent );
    shader->composeNodeSocket( SocketFlow::Out, ShaderData::Tangent );
    shader->composeNodeCode( "outTangent = normalize( gl_NormalMatrix * inTangent );\n" );
    shader->composeNodeEnd();

    shader->composeNodeNew( ShaderType::Vertex );
    shader->composeNodeSocket( SocketFlow::In, ShaderData::Bitangent );
    shader->composeNodeSocket( SocketFlow::Out, ShaderData::Bitangent );
    shader->composeNodeCode( "outBitangent = normalize( gl_NormalMatrix * inBitangent );\n" );
    shader->composeNodeEnd();

    shader->composeNodeNew( ShaderType::Fragment );
    shader->composeNodeSocket( SocketFlow::In, ShaderData::Tangent );
    shader->composeNodeSocket( SocketFlow::In, ShaderData::Bitangent );
    shader->composeNodeSocket( SocketFlow::In, ShaderData::Normal );
    shader->composeNodeSocket( SocketFlow::In, ShaderData::TexCoord2 );
    shader->composeNodeSocket( SocketFlow::Out, ShaderData::Normal );
    shader->composeNodeCode(
      "mat3 normMatrix = mat3( inTangent, inBitangent, inNormal);\n"
      "vec3 normTexel = texture2D( normSampler, inTexCoord2 ).xyz;\n"
      "vec3 localNormal = ((normTexel * 2.0) - vec3(1.0,1.0,1.0));\n"
      //"localNormal.x = -localNormal.x;\n"
      //"localNormal.y = -localNormal.y;\n"
      "vec3 worldNormal = normMatrix * localNormal;\n"
      //"vec3 worldNormal = ((normTexel * 2.0) - vec3(1.0,1.0,1.0));\n"
      //"outDiffuse = vec4( (worldNormal* 0.5) + vec3(0.5,0.5,0.5), 1.0 );\n"
      //"outDiffuse = vec4( worldNormal, 1.0 );\n"
      "outNormal = worldNormal;\n"
      );
    shader->composeNodeEnd();
  }

  void NormalTexMat::begin()
  {
    //StandardMaterial::begin();
    DiffuseTexMat::begin();

    Shader *shader = Kernel::GetInstance()->getRenderer()->getCurrentShader();
    if (!gotUniforms)
    {
      uNormSampler = shader->getUniformID( "normSampler" );
      gotUniforms = true;
    }
    
    glUniform1i( uNormSampler, 1 );
    if (texNormal != NULL)
    {
      glActiveTexture( GL_TEXTURE1 );
      glBindTexture( GL_TEXTURE_2D, texNormal->getHandle() );
      glEnable( GL_TEXTURE_2D );
    }
  }

  void NormalTexMat::end()
  {
    //StandardMaterial::end();
    DiffuseTexMat::end();

    if (texNormal != NULL)
    {
      glActiveTexture( GL_TEXTURE1 );
      glDisable( GL_TEXTURE_2D );
    }
  }
  
  
  /*
  ============================================
  
  MultiMaterial
  
  ============================================*/
  
  MultiMaterial::MultiMaterial ()
  {
    selectedID = 0;
  }

  MultiMaterial::MultiMaterial (SM *sm)
  {
    selectedID = 0;
  }

  /*
  -----------------------------------------------
  Extends or shrinks sub-material array
  -----------------------------------------------*/
  
  void MultiMaterial::setNumSubMaterials( UintSize n )
  {
    if( n < 0 || n > GE_MAX_MATERIAL_ID )
      return;
    
    if( n > subMaterials.size() ){
      for( UintSize i=subMaterials.size(); i<n; ++i )
        subMaterials.pushBack( NULL );

    }else if( n < subMaterials.size() ){
      for( UintSize i=subMaterials.size(); i>n; --i )
        subMaterials.last() = NULL;
        subMaterials.popBack();
    }
  }

  UintSize MultiMaterial::getNumSubMaterials()
  {
    return subMaterials.size();
  }
  
  /*
  --------------------------------------------------
  Sets or replaces exiting sub-material reference
  --------------------------------------------------*/
  
  void MultiMaterial::setSubMaterial( MaterialID id, Material *m )
  {
    if (id >= subMaterials.size())
      return;
    
    subMaterials[ id ] = m;
  }
  
  Material* MultiMaterial::getSubMaterial( MaterialID id )
  {
    if (id >= subMaterials.size())
      return NULL;
    
    return subMaterials[ id ];
  }
  
  /*
  ----------------------------------------------
  Selects a sub-material for rendering
  ----------------------------------------------*/
  
  bool MultiMaterial::selectionValid ()
  {
    if( selectedID < 0 || selectedID >= subMaterials.size () )
      return false;
    
    if( subMaterials[ selectedID ] == NULL )
      return false;
    
    return true;
  }
  
  bool MultiMaterial::selectSubMaterial( MaterialID id )
  {
    selectedID = id;
    return selectionValid();
  }
  
  /*
  ---------------------------------------------------
  Sets-up the OpenGL state for the selected material
  ---------------------------------------------------*/
  
  void MultiMaterial::begin()
  {
    if( selectionValid() )
      subMaterials[ selectedID ]->begin();
    else
      Material::BeginDefault();
  }
  
  void MultiMaterial::end ()
  {
    if( selectionValid() )
      subMaterials[ selectedID ]->end();
    else
      Material::EndDefault();
  }

  
}/* namespace GE */
