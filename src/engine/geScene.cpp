#include "geScene.h"
#include "geLight.h"

namespace GE
{
  DEFINE_SERIAL_CLASS (Scene3D, ClassID( 0x847aa932u, 0xf887, 0x475b, 0xa94d9a9b017e1f7aull ));

  Scene3D::Scene3D()
  {
    cam = NULL;
  }

  Scene3D::Scene3D (SM *sm)
    : Scene (sm), animations (sm)
  {
    cam = NULL;
  }

  Scene3D::~Scene3D ()
  {
    for (UintSize a=0; a<animations.size(); ++a)
      delete animations[ a ];
  }
  
  void Scene3D::updateChanges()
  {
    bool changed = hasChanged();
    Scene::updateChanges();

    if (!changed) return;
    if (getRoot() == NULL) return;

    //Init stack based on last size
    ArrayList< TravNode > stack( traversal.size() );

    //Clear old data
    lights.clear();
    traversal.clear();
    
    //Walk the scene tree and store traversal order
    stack.pushBack( TravNode( (Actor3D*) getRoot(), TravEvent::Begin ));
    while (!stack.empty())
    {
      //Take last node off the stack
      TravNode node = stack.last();
      Actor3D *a = node.actor;
      stack.popBack();
      
      //Store data
      traversal.pushBack( node );
      if (node.event == TravEvent::End )
        continue;
      
      //Store lights
      Light *l = SafeCast( Light, a );
      if (l != NULL)
        if (ClassOf( l ) != Class( PointLight ))
        lights.pushBack( l );
      
      //Put children actors onto the stack
      stack.pushBack( TravNode( a, TravEvent::End ));
      for (int c=(int)a->getChildren().size()-1; c >= 0; --c)
      {
        Actor3D *child = (Actor3D*) a->getChildren().at(c);
        stack.pushBack( TravNode( child, TravEvent::Begin ));
      }
    }
  }

  void Scene3D::setAmbientColor (const Vector3 &color) {
    ambientColor = color;
  }

  const Vector3& Scene3D::getAmbientColor () {
    return ambientColor;
  }
}
