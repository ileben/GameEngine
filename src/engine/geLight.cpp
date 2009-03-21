#include "geLight.h"
#include "geGLHeaders.h"

namespace GE
{
  DEFINE_CLASS( Light );
  DEFINE_CLASS( DirLight );
  DEFINE_CLASS( PointLight );
  DEFINE_CLASS( SpotLight );
  DEFINE_CLASS( HeadLight );

  Light::Light()
  {
    shadowsOn = false;
    diffuseColor.set( .9f, .9f, .9f );
    shadowColor.set( .2f, .2f, .2f );
    setIsRenderable( false );
    volumeDLinit = false;
    attenuationEnd = 1000.0f;
  }

  void Light::renderVolume()
  {
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

  void Light::setShadowColor (const Vector3 &c) {
    shadowColor = c;
  }

  const Vector3& Light::getDiffuseColor () {
    return diffuseColor;
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

  Matrix4x4 Light::getProjection (Float nearClip, Float farClip)
  {
    Matrix4x4 m;
    m.setIdentity();
    return m;
  }

  void Light::setAttenuationEnd (Float end)
  {
    attenuationEnd = end;
    updateVolume();
  }

  Float Light::getAttenuationEnd ()
  {
    return attenuationEnd;
  }
  
  DirLight::DirLight (const Vector3 &dir)
  {
    setDirection( dir );
  }
  
  PointLight::PointLight (const Vector3 &pos)
  {
    setPosition( pos );
  }
  
  SpotLight::SpotLight (const Vector3 &pos, const Vector3 &dir,
                        Float outerAngle, Float innerAngle)
  {
    setPosition( pos );
    setDirection( dir );
    setAngle( outerAngle, innerAngle );
    updateVolume();
  }
  
  void SpotLight::setAngle (Float outer, Float inner)
  {
    angleOuter = outer;
    angleInner = inner;
    updateVolume();
  }
  
  void Light::enable (int index)
  {
    GLfloat ambient[4] = {0.2f,0.2f,0.2f, 1.0f};
    GLfloat diffuse[4] = {diffuseColor.x, diffuseColor.y, diffuseColor.z, 1.0f};
    
    glEnable( GL_LIGHT0 + index );
    glLightfv( GL_LIGHT0 + index, GL_AMBIENT, ambient );
    glLightfv( GL_LIGHT0 + index, GL_DIFFUSE, diffuse );
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

  void PointLight::enable (int index)
  {
    Light::enable( index );
    
    //The position will be transformed by the modelview matrix
    //W coordinate of 1 says its a point light
    Vector4 glpos( 0, 0, 0, 1 );
    
    glLightfv( GL_LIGHT0 + index, GL_POSITION, (GLfloat*) &glpos );
    glLighti(  GL_LIGHT0 + index, GL_SPOT_CUTOFF, 180 );
  }
  
  void SpotLight::enable (int index)
  {
    Light::enable( index );

    //The positoin & direction will be transformed by the modelview matrix
    Vector4 glpos( 0, 0, 0, 1 );
    Vector4 gldir( 0, 0, 1, 0 );

    //Angle must be clamped to GL accepted values
    Float glangleOuter = Util::Clamp( angleOuter * 0.5f, 0.0f , 90.0f );
    Float glangleInner = (angleInner < 0.0f) ? angleOuter : Util::Min( angleInner * 0.5f, angleOuter );
    
    glLightfv( GL_LIGHT0 + index, GL_POSITION, (GLfloat*) &glpos );
    glLightfv( GL_LIGHT0 + index, GL_SPOT_DIRECTION, (GLfloat*) &gldir );
    glLightf(  GL_LIGHT0 + index, GL_SPOT_CUTOFF, glangleOuter );
    glLightf(  GL_LIGHT0 + index, GL_SPOT_EXPONENT, COS( Util::DegToRad( glangleInner )) );
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

  bool SpotLight::isPointInVolume (const Vector3 &p, Float threshold)
  {
    Matrix4x4 worldToLight = getMatrix().affineInverse();
    Vector3 lightP = worldToLight * p;

    //Check whether point close enough
    if (lightP.z > attenuationEnd)
      return false;

    //Check whether point inside the cone
    Float volumeR = lightP.z * TAN( Util::DegToRad( angleOuter * 0.5f ));
    Float pointR = lightP.xy().norm();
    if (pointR > volumeR + threshold)
      return false;

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
    Float r = attenuationEnd * TAN( Util::DegToRad( angleOuter * 0.5f ));
    
    glBegin( GL_TRIANGLE_FAN );
    glVertex3f( 0, 0, 0 );
    
    for (s=0, a=0.0f; s <= numSegments; ++s, a += step)
      glVertex3f( COS(a) * r, SIN(a) * r, attenuationEnd );
    
    glEnd();
    
    glBegin( GL_TRIANGLE_FAN );
    glVertex3f( 0, 0, attenuationEnd );
    
    for (s=0, a=0.0f; s <= numSegments; ++s, a += step)
      glVertex3f( - COS(a) * r, SIN(a) * r, attenuationEnd );
    
    glEnd();

    glEndList();
  }

}//namespace GE
