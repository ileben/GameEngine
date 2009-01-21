#include "geScene.h"
#include "geLight.h"

namespace GE
{
  DEFINE_CLASS (Scene);

  Scene::Scene()
  {
    changed = false;
    cam = NULL;
  }

  bool Scene::hasChanged()
  {
    return changed;
  }

  void Scene::markChanged()
  {
    changed = true;
  }

  void Scene::updateChanges()
  {
    //Init stack based on last size
    ArrayList< TravNode > stack( traversal.size() );

    //Clear old data
    lights.clear();
    traversal.clear();
    
    //Walk the scene tree and store traversal order
    stack.pushBack( TravNode( this, TravEvent::Begin ));
    while (!stack.empty())
    {
      //Take last node off the stack
      TravNode node = stack.last();
      Actor *a = node.actor;
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
      for (int c=(int)a->getChildren()->size()-1; c >= 0; --c)
        stack.pushBack( TravNode( a->getChildren()->at(c), TravEvent::Begin ));
    }
  }
}
