#include "geLight.h"
#include "geGLHeaders.h"
#include "engine/geShader.h"
#include "engine/embedit/Light.embedded"

namespace GE
{
  DEFINE_SERIAL_CLASS( Light,        ClassID( 0x89f9d379u, 0x0ed5, 0x4294, 0xa9c06ea3a38efcdbull ));
  DEFINE_SERIAL_CLASS( SpotLight,    ClassID( 0xcce8f0fbu, 0xbac0, 0x4596, 0xa2db479d83cdb273ull ));
  DEFINE_SERIAL_CLASS( PyramidLight, ClassID( 0xbf58eaa8u, 0x3ef1, 0x4885, 0x89ffaad24b3df257ull ));
  DEFINE_SERIAL_CLASS( PointLight,   ClassID( 0xad22d8b9u, 0xac26, 0x4433, 0x836176166387df78ull ));
  DEFINE_CLASS( DirLight );
  DEFINE_CLASS( HeadLight );

  Light::Light()
  {
    shadowsOn = false;
    diffuseColor.set( .9f, .9f, .9f );
    shadowColor.set( .2f, .2f, .2f );
    specularColor.set( 1, 1, 1);
    attEnd = 1000.0f;
    attStart = -1.0f;

    uniformsInit = false;
    volumeDLinit = false;
    volumeChanged = true;
    setIsRenderable( false );
  }

  Light::Light(SM *sm)
  {
    uniformsInit = false;
    volumeDLinit = false;
    volumeChanged = true;
    setIsRenderable( false );
  }

  void Light::renderVolume()
  {
    if (volumeChanged)
    {
      updateVolume();
      volumeChanged = false;
    }

    glCallList( volumeDL );
  }

  void Light::setCastShadows (bool cast) {
    shadowsOn = cast;
  }

  bool Light::getCastShadows () {
    return shadowsOn;
  }

  void Light::setDiffuseColor (const Vector3 &c) {
    diffuseColor = c;
  }

  const Vector3& Light::getDiffuseColor () {
    return diffuseColor;
  }

  void Light::setSpecularColor (const Vector3 &c) {
    specularColor = c;
  }

  const Vector3& Light::getSpecularColor () {
    return specularColor;
  }

  void Light::setShadowColor (const Vector3 &c) {
    shadowColor = c;
  }

  const Vector3& Light::getShadowColor () {
    return shadowColor;
  }

  void Light::setDirection (const Vector3 &dir)
  {
    lookAt( dir, Vector3( 0,1,0 ));
  }

  void Light::setPosition (const Vector3 &pos)
  {
    Matrix4x4 m = getMatrix();
    m.setColumn( 3, pos.xyz( 1.0f));
    setMatrix( m );
  }

  Vector3 Light::getDirection ()
  {
    return getMatrix().getColumn(2).xyz();
  }

  Vector3 Light::getPosition ()
  {
    return getMatrix().getColumn(3).xyz();
  }

  Matrix4x4 Light::getProjection (Float nearClip, Float farClip)
  {
    Matrix4x4 m;
    m.setIdentity();
    return m;
  }

  void Light::setAttenuation (Float end, Float start)
  {
    attEnd = end;
    attStart = start;
    volumeChanged = true;
  }

  Float Light::getAttenuationEnd () {
    return attEnd;
  }
  
  Float Light::getAttenuationStart () {
    return attStart;
  }

  PointLight::PointLight()
  {
    Vector3 subDirs[6] =
    {
      Vector3(  1,  0,  0 ),
      Vector3( -1,  0,  0 ),
      Vector3(  0,  1,  0 ),
      Vector3(  0, -1,  0 ),
      Vector3(  0,  0,  1 ),
      Vector3(  0,  0, -1 )
    };
    
    for (UintSize s=0; s<6; ++s)
    {
      PyramidLight *subLight = new PyramidLight;
      subLight->setDirection( subDirs[ s ] );
      subLight->setAngle( 90.0f );
      subLight->setParent( this );
      subLights.pushBack( subLight );
    }
  }

  PointLight::PointLight (const Vector3 &pos)
  {
    PointLight();
    setPosition( pos );
  }

  void PointLight::setCastShadows (bool cast)
  {
    Light::setCastShadows( cast );
    for (UintSize s=0; s<6; ++s)
      subLights[ s ]->setCastShadows( cast );
  }

  void PointLight::setDiffuseColor (const Vector3 &color)
  {
    Light::setDiffuseColor( color );
    for (UintSize s=0; s<6; ++s)
      subLights[ s ]->setDiffuseColor( color );
  }

  void PointLight::setSpecularColor (const Vector3 &color)
  {
    Light::setSpecularColor( color );
    for (UintSize s=0; s<6; ++s)
      subLights[ s ]->setSpecularColor( color );
  }

  void PointLight::setShadowColor (const Vector3 &color)
  {
    Light::setShadowColor( color );
    for (UintSize s=0; s<6; ++s)
      subLights[ s ]->setShadowColor( color );
  }

  void PointLight::setAttenuation (Float end, Float start)
  {
    Light::setAttenuation( end, start );
    for (UintSize s=0; s<6; ++s)
      subLights[ s ]->setAttenuation( end, start );
  }

  DirLight::DirLight (const Vector3 &dir)
  {
    setDirection( dir );
  }
  
  SpotLight::SpotLight (const Vector3 &pos, const Vector3 &dir,
                        Float outerAngle, Float innerAngle)
  {
    setPosition( pos );
    setDirection( dir );
    setAngle( outerAngle, innerAngle );
    volumeChanged = true;
  }
  
  void SpotLight::setAngle (Float outer, Float inner)
  {
    angleOuter = outer;
    angleInner = inner;
    volumeChanged = true;
  }

  PyramidLight::PyramidLight (const Vector3 &pos, const Vector3 &dir,
                              Float angle)
  {
    setPosition( pos );
    setDirection( dir );
    setAngle( angle );
    volumeChanged = true;
  }

  void PyramidLight::setAngle (Float a)
  {
    angle = a;
    volumeChanged = true;
  }

  void Light::composeShader (Shader *shader)
  {
    shader->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerNormal" );
    shader->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerColor" );
    shader->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerSpec" );
    shader->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerParams" );
    shader->registerUniform( ShaderType::Fragment, DataUnit::Sampler2D, "samplerShadow" );
    shader->registerUniform( ShaderType::Fragment, DataUnit::Int, "castShadow" );
    shader->registerUniform( ShaderType::Fragment, DataUnit::Vec2, "winSize" );
    shader->registerUniform( ShaderType::Fragment, DataUnit::Float, "attStart" );
    shader->registerUniform( ShaderType::Fragment, DataUnit::Float, "attEnd" );

    shader->compile( ShaderType::Vertex, Light_VS );
    shader->compile( ShaderType::Fragment, Light_FS );
  }

  void SpotLight::composeShader (Shader *shader)
  {
    shader->compile( ShaderType::Fragment, SpotLight_Func );
    Light::composeShader( shader );
  }

  void PyramidLight::composeShader (Shader *shader)
  {
    shader->compile( ShaderType::Fragment, PyramidLight_Func );
    Light::composeShader( shader );
  }

  void Light::begin (Shader *shader, Vector2 winSize, Uint32 *deferredMaps, Uint32 shadowMap)
  {
    if (!uniformsInit)
    {
      uSampler[ Deferred::Normal ] = shader->getUniformID( "samplerNormal" );
      uSampler[ Deferred::Color ] = shader->getUniformID( "samplerColor" );
      uSampler[ Deferred::Specular ] = shader->getUniformID( "samplerSpec" );
      uSampler[ Deferred::Params ] = shader->getUniformID( "samplerParams" );
      uSampler[ Deferred::Shadow ] = shader->getUniformID( "samplerShadow" );
      uCastShadow = shader->getUniformID( "castShadow" );
      uWinSize = shader->getUniformID( "winSize" );
      uAttStart = shader->getUniformID( "attStart" );
      uAttEnd = shader->getUniformID( "attEnd" );
      uniformsInit = true;
    }

    //Pass in light parameters
    glUniform1f( uAttEnd, (GLfloat) attEnd );
    glUniform1f( uAttStart, (GLfloat) (attStart < 0.0f ? attEnd : Util::Min( attStart, attEnd )));
    glUniform2f( uWinSize, (GLfloat) winSize.x, (GLfloat) winSize.y );

    if (shadowsOn)
    {
      //Bind shadow texture
      glUniform1i( uCastShadow, 1);
      glUniform1i( uSampler[Deferred::Shadow], Deferred::Shadow );
      glActiveTexture( GL_TEXTURE0 + Deferred::Shadow );
      glBindTexture( GL_TEXTURE_2D, shadowMap );
      glEnable( GL_TEXTURE_2D );
    }
    else
    {
      //Disable shadows
      glUniform1i( uCastShadow, 0);
    }

    //Bind geometry textures
    for (int d=0; d<GE_NUM_GBUFFERS; ++d)
    {
      glUniform1i( uSampler[d], d );
      glActiveTexture( GL_TEXTURE0 + d );
      glBindTexture( GL_TEXTURE_2D, deferredMaps[d] );
      glEnable( GL_TEXTURE_2D );
    }
  }

  void Light::end()
  {
    //Disable geometry textures
    for (int d=GE_NUM_GBUFFERS-1; d>=0; --d)
    {
      glActiveTexture( GL_TEXTURE0 + d );
      glDisable( GL_TEXTURE_2D );
    }

    //Disable shadow texture
    glActiveTexture( GL_TEXTURE0 + Deferred::Shadow );
    glDisable( GL_TEXTURE_2D );
  }
  
  void Light::enable (int index)
  {
    GLfloat ambient[4] = {0.2f,0.2f,0.2f, 1.0f};
    GLfloat diffuse[4] = {diffuseColor.x, diffuseColor.y, diffuseColor.z, 1.0f};
    GLfloat specular[4] = {specularColor.x, specularColor.y, specularColor.z, 1.0f};
    
    glEnable( GL_LIGHT0 + index );
    glLightfv( GL_LIGHT0 + index, GL_AMBIENT, ambient );
    glLightfv( GL_LIGHT0 + index, GL_DIFFUSE, diffuse );
    glLightfv( GL_LIGHT0 + index, GL_SPECULAR, specular );
  }

  void DirLight::enable (int index)
  {
    Light::enable( index );
    
    //The direction will be transformed by the modelview matrix
    //W coordinate of 0 says its a directional light
    //(In GL directional lights take position from vertex
    //towards the light source, hence -1!)
    Vector4 gldir( 0, 0, -1, 0 );

    glLightfv( GL_LIGHT0 + index, GL_POSITION, (GLfloat*) &gldir );
    glLighti(  GL_LIGHT0 + index, GL_SPOT_CUTOFF, 180 );
  }
  
  void SpotLight::enable (int index)
  {
    Light::enable( index );

    //The position & direction will be transformed by the modelview matrix
    Vector4 glpos( 0, 0, 0, 1 );
    Vector4 gldir( 0, 0, 1, 0 );

    //Angle must be clamped to GL accepted values
    Float glangleOuter = Util::Clamp( angleOuter * 0.5f, 0.0f , 90.0f );
    Float glangleInner = (angleInner < 0.0f) ? glangleOuter
      : Util::Min( angleInner * 0.5f, glangleOuter );
    
    glLightfv( GL_LIGHT0 + index, GL_POSITION, (GLfloat*) &glpos );
    glLightfv( GL_LIGHT0 + index, GL_SPOT_DIRECTION, (GLfloat*) &gldir );
    glLightf(  GL_LIGHT0 + index, GL_SPOT_CUTOFF, glangleOuter );
    glLightf(  GL_LIGHT0 + index, GL_SPOT_EXPONENT, COS( Util::DegToRad( glangleInner )) );
  }

  void PyramidLight::enable (int index)
  {
    Light::enable( index );

    //The position will be transformed by the modelview matrix
    Vector4 glpos( 0, 0, 0, 1 );
    glLightfv( GL_LIGHT0 + index, GL_POSITION, (GLfloat*) &glpos );

    //Don't need direction since the light volume will define affected pixels
  }

  void HeadLight::enable (int index)
  {
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    PointLight::enable( index );
    
    glPopMatrix();
  }

  Matrix4x4 SpotLight::getProjection (Float nearClip, Float farClip)
  {
    Matrix4x4 m;
    m.setPerspectiveFovLH( Util::DegToRad( angleOuter ), 1.0f, nearClip, farClip );
    return m;
  }

  Matrix4x4 PyramidLight::getProjection (Float nearClip, Float farClip)
  {
    Matrix4x4 m;
    m.setPerspectiveFovLH( Util::DegToRad( angle ), 1.0f, nearClip, farClip );
    return m;
  }

  bool SpotLight::isPointInVolume (const Vector3 &p, Float threshold)
  {
    //Transform point to light space
    Matrix4x4 worldToLight = getGlobalMatrix().affineNormalize().affineInverse();
    Vector3 lightP = worldToLight * p;

    //Check whether point in range
    if (lightP.z < 0.0 || lightP.z > attEnd)
      return false;

    //Check whether point inside the cone
    Float volumeR = lightP.z * TAN( Util::DegToRad( angleOuter * 0.5f ));
    Float pointR = lightP.xy().norm();
    if (pointR > volumeR + threshold)
      return false;

    return true;
  }

  bool PyramidLight::isPointInVolume (const Vector3 &p, Float threshold)
  {
    //Transform point to light space
    Matrix4x4 worldToLight = getGlobalMatrix().affineNormalize().affineInverse();
    Vector3 lightP = worldToLight * p;

    //Check whether point in range
    if (lightP.z < 0.0 || lightP.z > attEnd)
      return false;

    //Check whether point inside the pyramid
    Float volumeR = lightP.z * TAN( Util::DegToRad( angle * 0.5f ));
    if (fabsf( lightP.x ) > volumeR + threshold) return false;
    if (fabsf( lightP.y ) > volumeR + threshold) return false;

    return true;
  }

  void SpotLight::updateVolume ()
  {
    if (!volumeDLinit)
    {
      volumeDL = glGenLists( 1 );
      volumeDLinit = true;
    }

    glNewList( volumeDL, GL_COMPILE );

    int s; Float a;
    int numSegments = 50;
    Float step = 2 * PI / numSegments;
    Float r = attEnd * TAN( Util::DegToRad( angleOuter * 0.5f ));
    
    glBegin( GL_TRIANGLE_FAN );
    glVertex3f( 0, 0, 0 );
    
    for (s=0, a=0.0f; s <= numSegments; ++s, a += step)
      glVertex3f( COS(a) * r, SIN(a) * r, attEnd );
    
    glEnd();
    
    glBegin( GL_TRIANGLE_FAN );
    glVertex3f( 0, 0, attEnd );
    
    for (s=0, a=0.0f; s <= numSegments; ++s, a += step)
      glVertex3f( - COS(a) * r, SIN(a) * r, attEnd );
    
    glEnd();

    glEndList();
  }

  void PyramidLight::updateVolume()
  {
    if (!volumeDLinit)
    {
      volumeDL = glGenLists( 1 );
      volumeDLinit = true;
    }

    glNewList( volumeDL, GL_COMPILE );

    Float r = attEnd * TAN( Util::DegToRad( angle * 0.5f ));

    glBegin( GL_TRIANGLE_FAN );
    glVertex3f(  0, 0, 0 );
    glVertex3f( +r,+r, attEnd );
    glVertex3f( -r,+r, attEnd );
    glVertex3f( -r,-r, attEnd );
    glVertex3f( +r,-r, attEnd );
    glVertex3f( +r,+r, attEnd );
    glEnd();

    glBegin( GL_QUADS );
    glVertex3f( +r,+r, attEnd );
    glVertex3f( +r,-r, attEnd );
    glVertex3f( -r,-r, attEnd );
    glVertex3f( -r,+r, attEnd );
    glEnd();

    glEndList();
  }

}//namespace GE
