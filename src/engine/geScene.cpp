#include "geScene.h"
#include "geLight.h"

namespace GE
{
  DEFINE_CLASS (Scene3D);

  Scene3D::Scene3D()
  {
    cam = NULL;
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
      if (l != NULL) lights.pushBack( l );
      
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
