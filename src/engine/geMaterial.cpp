#include "geKernel.h"
#include "geMaterial.h"
#include "geTexture.h"
#include "geShaders.h"
#include "geShader.h"
#include "geGLHeaders.h"

namespace GE
{
  DEFINE_CLASS (Material);
  DEFINE_CLASS (StandardMaterial);
  DEFINE_CLASS (MultiMaterial);
  DEFINE_CLASS (PhongMaterial);
  DEFINE_CLASS (DiffuseTexMat);
  
  /*
  ============================================
  
  Property - material property interface
  
  ============================================*/
  
  const CharString& Material::UniformProperty::getName()
  {
    return name;
  }
  
  Material::UniformProperty::~UniformProperty ()
  {
  }
  
  /*
  =================================================
  
  UniformVecProperty - material property that
  sets up vector uniform variables of the shader
  
  =================================================*/
  
  template <class T>
  Material::UniformVecProperty<T>::UniformVecProperty (GLProgram *program,
                                                       const String &name,
                                                       int count)
  {
    this->program = program;
    this->name = name;
    this->count = count;
    
    if (this->count <= 0)
      this->count = 1;
    
    val = NULL;
  }
  
  template <class T>
  Material::UniformVecProperty<T>::~UniformVecProperty ()
  {
    freeval ();
  }
  
  template <class T>
  void Material::UniformVecProperty<T>::freeval ()
  {
    if (val != NULL) {
      delete[] val;
      val = NULL;
    }
  }
  
  template <class T>  
  void Material::UniformVecProperty<T>::set (void *value)
  {
    freeval ();
    val = new T [count];
    memcpy (val, value, count * sizeof (T));
  }
  
  template <class T>
  void Material::UniformVecProperty<T>::begin ()
  {
  }
  
  template <class T>
  void Material::UniformVecProperty<T>::end ()
  {
  }
  
  /*
  ===============================================
  
  UniformVecProperty - integer specializations
  
  ===============================================*/
  
  template <>
  void Material::UniformVecProperty<Int32>::begin ()
  {
    //Obtain address of the variable in the shader program
    GLint propaddr = program->getUniform (getName().toCSTR().buffer());
    
    //Don't pass if not set
    if (val == NULL)
      return;
    
    //Pass integers into vector
    switch (count) {
    case 1: glUniform1i (propaddr, val[0]); break;
    case 2: glUniform2i (propaddr, val[0], val[1]); break;
    case 3: glUniform3i (propaddr, val[0], val[1], val[2]); break;
    case 4: glUniform4i (propaddr, val[0], val[1], val[2], val[3]); break;
    }
  }
  
  /*
  ===============================================
  
  UniformVecProperty - float specializations
  
  ===============================================*/
  
  template <>
  void Material::UniformVecProperty<Float32>::begin ()
  {
    //Obtain address of the variable in the shader program
    GLint propaddr = program->getUniform (name.toCSTR().buffer());
    
    //Don't pass if not set
    if (val == NULL)
      return;
    
    //Pass integers into vector
    switch (count) {
    case 1: glUniform1f (propaddr, val[0]); break;
    case 2: glUniform2f (propaddr, val[0], val[1]); break;
    case 3: glUniform3f (propaddr, val[0], val[1], val[2]); break;
    case 4: glUniform4f (propaddr, val[0], val[1], val[2], val[3]); break;
    }
  }
  
  /*
  ============================================================
  
  UniformTexProperty - material property that passes current
  texture unit to a shader's sampler uniform variable and
  sets up GL texturing state.
  
  ============================================================*/
  
  Material::UniformTexProperty::UniformTexProperty (GLProgram *program,
                                                    const String &name,
                                                    int textureUnit)
  {
    this->program = program;
    this->name = name;
    this->texunit = textureUnit;
    val = NULL;
  }
  
  void Material::UniformTexProperty::set (void *value)
  {
    val = (Texture*)value;
  }
  
  void Material::UniformTexProperty::begin ()
  {
    //Obtain address of the variable in the shader program
    GLint propaddr = program->getUniform (name.toCSTR().buffer());
    
    //Don't pass if not set
    if (val == NULL)
      return;
    
    //Tell sampler which texture unit to use
    glUniform1f (propaddr, texunit);
    
    //Bind texture to given texture unit and enable
    glActiveTexture (GL_TEXTURE0 + texunit);
    glBindTexture (GL_TEXTURE_2D, val->getHandle());
    glEnable (GL_TEXTURE_2D);
  }
  
  void Material::UniformTexProperty::end ()
  {
    //Disable texture for our texture unit
    glActiveTexture (GL_TEXTURE0 + texunit);
    glDisable (GL_TEXTURE_2D);
  }
  
  /*
  ============================================
  
  Material base
  
  ============================================*/
  
  Material::Material ()
  {
  }
  
  Material::~Material ()
  {
    freeUniformProps ();
  }
  
  /*
  Removes and frees uniform props */
  
  void Material::freeUniformProps ()
  {
    for( UintSize p=0; p<uniformProps.size(); ++p )
      delete uniformProps[ p ];
    
    uniformProps.clear ();
  }
  
  void Material::setShader (Shader *shader)
  {
    //Free shader-related props
    freeUniformProps ();
    
    //Shader must exist and be loaded
    if (shader == NULL)
      return;
    
    if (shader->program == NULL)
      return;
    
    //Add uniform properties to material for each uniform found in shader
    int texUnit = 0;
    
    for (UintSize u=0; u < shader->getUniformCount(); ++u)
    {
      Shader::Uniform &uni = shader->getUniform (u);
      switch (uni.unit.type)
      {
      case DataType::Int:
        uniformProps.pushBack (new UniformVecProperty<Int32>
                               (shader->program,
                                uni.name, uni.unit.count));
        break;
        
      case DataType::Float:
        uniformProps.pushBack (new UniformVecProperty<Float>
                               (shader->program,
                                uni.name, uni.unit.count));
        break;
        
      case DataType::Matrix:
        //TODO
        break;
        
      case DataType::Sampler2D:
        uniformProps.pushBack (new UniformTexProperty
                               (shader->program,
                                uni.name, texUnit++));
        break;
      }
    }
  }
  
  void Material::setProperty( const String &name, void *value )
  {
    //Find property with given name
    for( UintSize p=0; p<uniformProps.size(); ++p )
    {
      //Set its value
      if( uniformProps[p]->getName() == name ){
        uniformProps[p]->set( value );
        return;
      }
    }
  }
  
  void Material::begin()
  {    
    //Begin properties
    for( UintSize p=0; p<uniformProps.size(); ++p )
      uniformProps[ p ]->begin();
  }
  
  void Material::end()
  {
    //Unwind properties
    for( UintSize p=0; p<uniformProps.size(); ++p )
      uniformProps[ p ]->end();
  }
  
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
    ambientColor.set (0.2f, 0.2f, 0.2f);
    diffuseColor.set (1, 1, 1);
    specularColor.set (1, 1, 1);
    specularity = 0.0f;
    glossiness = 0.5f;
    opacity = 1.0f;
    lighting = true;
    culling = true;
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
  
  void StandardMaterial::begin ()
  {
    //Named Properties to shader
    Material::begin ();
    
    //Lighting
    if (lighting)
    {
      glEnable (GL_LIGHTING);
      
      //We will set material color manually (not via glColor)
      glDisable (GL_COLOR_MATERIAL);
      
      //Ambient color
      Vector4 ambient = ambientColor.xyz (1.0f);
      glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT, (Float*) &ambient);
      
      //Diffuse color
      Vector4 diffuse = diffuseColor.xyz (opacity);
      glMaterialfv (GL_FRONT_AND_BACK, GL_DIFFUSE, (Float*) &diffuse);
      
      //Specularity
      if (specularity > 0.0f) {
        
        //We want specularity to be aplied to texture colors too
        GLint param = GL_SEPARATE_SPECULAR_COLOR;
        glLightModeliv (GL_LIGHT_MODEL_COLOR_CONTROL, &param);
        
        //Specular color
        Vector4 spec = (specularColor * specularity).xyz(1);
        glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, (Float*) &spec);
        
        //GL uses an integer value for maximum glossiness
        int shininess = (int)(glossiness * _GL_MAX_SHININESS);
        glMateriali (GL_FRONT_AND_BACK, GL_SHININESS, shininess);
        
      }else{
        
        //Just make specular color invisible
        Vector4 specular (0,0,0,0);
        glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, (Float*)&specular);
      }
      
    }else{
      
      glDisable (GL_LIGHTING);
      glDisable (GL_COLOR_MATERIAL);
      
      //Diffuse color
      Vector4 diffuse = diffuseColor.xyz (opacity);
      glColor4fv ((Float*) &diffuse);
    }
    
    //Blending
    bool blend = false;
    
    if (opacity < 1.0f)
      blend = true;
    
    if (blend) {
      glEnable (GL_BLEND);
      glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }else glDisable (GL_BLEND);
    
    //Back-face culling
    if(culling) {
      glEnable (GL_CULL_FACE);
    }else glDisable (GL_CULL_FACE);
    
    //Buffers & tests
    glEnable( GL_DEPTH_TEST );
    
    //Normalize all normals so
    //we can freely scale actors
    glEnable( GL_NORMALIZE );
  }
  
  
  
  /*
  ============================================
  
  MultiMaterial
  
  ============================================*/
  
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
  
  /*
  ============================================
  
  Per-pixel phong-shaded material
  
  ============================================*/
  
  PhongMaterial::PhongMaterial() : StandardMaterial()
  {
    //Setup the shader object
    Shader *specShader = new Shader;
    specShader->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "textures[0]" );
    specShader->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "textures[1]" );
    specShader->registerUniform( ShaderType::Fragment, DataUnit::Int, "useTextures[0]" );
    specShader->registerUniform( ShaderType::Fragment, DataUnit::Int, "useTextures[1]" );
    specShader->fromFile( "specularity.vertex.c", "specularity.fragment.c" );
    
    //Assign shader
    setShader( specShader );
    
    //Init other properties
    texDiffuse = NULL;
    texSpecularity = NULL;
  }
  
  void PhongMaterial::setDiffuseTexture (Texture *tex)
  {
    if (texDiffuse != NULL)
      texDiffuse->dereference();
    
    texDiffuse = tex;
    texDiffuse->reference();
    
    int use = (tex == NULL) ? 0 : 1;
    setProperty ("useTextures[0]", &use);
    setProperty ("textures[0]", tex);
  }
  
  Texture* PhongMaterial::getDiffuseTexture ()
  {
    return texDiffuse;
  }
  
  void PhongMaterial::setSpecularityTexture (Texture *tex)
  {
    if (texSpecularity != NULL)
      texSpecularity->dereference();

    texSpecularity = tex;
    texSpecularity->reference();
    
    int use = (tex = NULL) ? 0 : 1;
    setProperty ("useTextures[1]", &use);
    setProperty ("textures[1]", tex);
  }

  Texture* PhongMaterial::getSpecularityTexture()
  {
    return texSpecularity;
  }
  
  void PhongMaterial::begin()
  {
    StandardMaterial::begin();
    
    if (texDiffuse != NULL) {
      if (texDiffuse->getFormat() == COLOR_FORMAT_GRAY_ALPHA ||
          texDiffuse->getFormat() == COLOR_FORMAT_RGB_ALPHA) {
        
        glEnable (GL_BLEND);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      }}
  }

  /*
  ============================================
  
  Uses texture for diffuse color
  
  ============================================*/


  DiffuseTexMat::DiffuseTexMat()
  {
    texDiffuse = NULL;
  }

  void DiffuseTexMat::setDiffuseTexture (Texture *tex) {
    texDiffuse = tex;
  }

  Texture* DiffuseTexMat::getDiffuseTexture () {
    return texDiffuse;
  }

  void DiffuseTexMat::composeShader (Shader *shader)
  {
    shader->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "diffSampler" );

    shader->composeNodeNew( ShaderType::Fragment );
    shader->composeNodeSocket( SocketFlow::In, ShaderData::TexCoord, 0 );
    shader->composeNodeSocket( SocketFlow::Out, ShaderData::Diffuse );
    shader->composeNodeCode( "outDiffuse = texture2D( diffSampler, inTexCoord0.xy );\n" );
    shader->composeNodeEnd();
  }

  void DiffuseTexMat::begin()
  {
    StandardMaterial::begin();

    Shader *shader = Kernel::GetInstance()->getRenderer()->getCurrentShader();
    Int32 samplerID = shader->getUniformID( "diffSampler" );
    glUniform1i( samplerID, 0 );

    glActiveTexture( GL_TEXTURE0 );
    glBindTexture( GL_TEXTURE_2D, texDiffuse->getHandle() );
    glEnable( GL_TEXTURE_2D );
  }

  void DiffuseTexMat::end()
  {
    StandardMaterial::end();

    glActiveTexture( GL_TEXTURE0 );
    glDisable( GL_TEXTURE_2D );
  }
  
}/* namespace GE */
