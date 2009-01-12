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
  }
  
  void SpotLight::setAngle (Float outer, Float inner)
  {
    angleOuter = outer;
    angleInner = inner;
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
    Float glangleOuter = Util::Clamp( angleOuter, 0.0f , 90.0f );
    Float glangleInner = (angleInner < 0.0f) ? angleOuter : Util::Min( angleInner, angleOuter );

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
    m.setPerspectiveFovLH( angleOuter, 1.0f, nearClip, farClip );
    return m;
  }

}//namespace GE
