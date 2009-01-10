#define GE_API_EXPORT
#include "geEngine.h"
#include "geGLHeaders.h"

namespace GE
{
  Light::Light()
  {
    setIsRenderable( false );
  }
  
  RenderRole::Enum Light::getRenderRole()
  {
    return RenderRole::Light;
  }

  void Light::setDirection (const Vector3 &dir)
  {
  }

  void Light::setPosition (const Vector3 &pos)
  {
  }
  
  DirLight::DirLight (const Vector3 &dir)
  {
    setDirection( dir );
  }

  void DirLight::setDirection (const Vector3 &dir) {
    direction = dir;
  };
  
  PointLight::PointLight (const Vector3 &pos) {
    setPosition( pos );
  }

  void PointLight::setPosition (const Vector3 &pos) {
    position = pos;
  };
  
  SpotLight::SpotLight (const Vector3 &pos, const Vector3 &dir, float angle)
  {
    setPosition( pos );
    setDirection( dir );
    setAngle( angle );
  }

  void SpotLight::setPosition (const Vector3 &pos) {
    position = pos;
  }

  void SpotLight::setDirection (const Vector3 &dir) {
    direction = dir;
  }

  void SpotLight::setAngle (float degAngle) {
    angle = degAngle; 
  }
  
  void Light::enable (int index)
  {
    Float L = 0.9f;
    GLfloat ambient[4] = {0.2f,0.2f,0.2f, 1.0f};
    GLfloat diffuse[4] = {L, L, L, 1.0f};
    
    glEnable( GL_LIGHT0 + index );
    glLightfv( GL_LIGHT0 + index, GL_AMBIENT, ambient );
    glLightfv( GL_LIGHT0 + index, GL_DIFFUSE, diffuse );
  }

  void DirLight::enable (int index)
  {
    Light::enable( index );

    Vector4 glpos = direction.xyz( 0.0f );
    glEnable(  GL_LIGHT0 + index );
    glLightfv( GL_LIGHT0 + index, GL_POSITION, (GLfloat*) &glpos );
    glLighti(  GL_LIGHT0 + index, GL_SPOT_CUTOFF, 180 );
  }

  void PointLight::enable (int index)
  {
    Light::enable( index );

    Vector4 glpos = position.xyz( 1.0f );
    glEnable(  GL_LIGHT0 + index );
    glLightfv( GL_LIGHT0 + index, GL_POSITION, (GLfloat*) &glpos );
    glLighti(  GL_LIGHT0 + index, GL_SPOT_CUTOFF, 180 );
  }

  void HeadLight::enable (int index)
  {
    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    Light::enable( index );
    
    Vector4 glpos = position.xyz( 1.0f );
    glEnable(  GL_LIGHT0 + index );
    glLightfv( GL_LIGHT0 + index, GL_POSITION, (GLfloat*) &glpos );
    glLighti(  GL_LIGHT0 + index, GL_SPOT_CUTOFF, 180 );

    glPopMatrix();
  }
  
  void SpotLight::enable (int index)
  {
    Light::enable( index );

    Vector4 glpos = position.xyz( 1.0f );
    Float glangle = Util::Clamp( angle, 0.0f , 90.0f );
    
    glEnable(  GL_LIGHT0 + index );
    glLightfv( GL_LIGHT0 + index, GL_POSITION, (GLfloat*) &glpos );
    glLightf(  GL_LIGHT0 + index, GL_SPOT_CUTOFF, glangle );
  }

}//namespace GE
