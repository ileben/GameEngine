#include "ui/uiActor.h"

namespace GE
{
namespace UI
{
  DEFINE_CLASS( Event );
  DEFINE_CLASS( Widget );
  DEFINE_CLASS( Window );

  Stage* Stage::instance = NULL;

  Stage::Stage()
  {
    assert( Stage::instance == NULL );
    Stage::instance = this;
  }

  void Stage::registerPin (Pin *p)
  {
    p->id = (Uint) pins.size();
    pins.pushBack( p );
  }

  Widget::Widget()
  {
    valid = true;
    parent = NULL;

    //Get stage instance
    Stage *stage = Stage::GetInstance();
    assert( stage != NULL );
    
    //Init pinholes for every registered pin
    ArrayList <Pin*> &pins = stage->getPins();
    for (UintSize p=0; p<pins.size(); ++p)
      pinholes.pushBack( Pinhole() );
  }

  void Widget::destroy()
  {
    for (UintSize c=0; c<children.size(); ++c)
      children[c]->destroy();

    setParent( NULL );
    valid = false;
  }

  Window* Widget::getWindow()
  {
    Widget *parent = this;
    while (parent != NULL)
    {
      Window *w = SafeCast( Window, parent );
      if (w != NULL) return w;
      parent = parent->getParent();
    }

    return NULL;
  }

  void Widget::addChild (Widget* c)
  {
    if (c->parent != NULL) {
      c->parent->children.remove( this );
      c->parent = NULL;
    }
    
    children.pushBack( c );
    c->parent = this;

    Window *w = getWindow();
    if (w != NULL) w->markChanged();
  }
  
  void Widget::removeChild (Widget* c)
  {
    children.remove( c );
    c->parent = NULL;

    Window *w = getWindow();
    if (w != NULL) w->markChanged();
  }

  void Widget::setParent (Widget* p)
  {
    if (parent != NULL) {
      parent->children.remove( this );
      parent = NULL;
    }

    if (p != NULL) {
      p->children.pushBack( this );
      parent = p;
    }

    Window *w = getWindow();
    if (w != NULL) w->markChanged();
  }

  void Widget::getAncestors (ArrayList<Widget*> &list)
  {
    Widget *parent = getParent();
    while (parent != NULL)
    {
      list.pushBack( parent );
      parent = parent->getParent();
    }
  }

  Window::Window()
  {
    changed = false;
  }

  void Window::markChanged()
  {
    changed = true;
  }

  void Window::updateChanges()
  {
    ArrayList <Widget*> stack;
    stack.pushBack( this );
    traversal.clear();

    while (!stack.empty())
    {
      //Pop top
      Widget* parent = stack.last();
      stack.popBack();

      //Add to traversal
      traversal.pushBack( parent );

      //Push children to stack
      const ArrayList<Widget*> & children = parent->getChildren();
      for (UintSize c=0; c<children.size(); ++c)
        stack.pushBack( children[c] );
    }
  }

  void Pin::drop (Widget *w)
  {
    //Find new pin targets
    ArrayList<Widget*> newTargets;
    if (w != NULL)
    {
      newTargets.pushBack( w );
      w->getAncestors( newTargets );
    }

    //Mark new holes
    for (UintSize t=0; t<newTargets.size(); ++t)
      newTargets[t]->pinholes[ id ].newHole = true;

    //Unpin old targets
    for (UintSize t=0; t<targets.size(); ++t)
    {
      //Make sure its still valid
      Widget *target = targets[t];
      if (! target->isValid() ) continue;

      //Unpin if not a new target
      if (! target->pinholes[ id ].newHole) {
        target->pinholes[ id ].oldHole = false;
        unpin( target);
      }
    }

    //Clear old targets
    targets.clear();

    //Pin/Re-pin new targets
    for (UintSize t=0; t<newTargets.size(); ++t)
    {
      //Make sure its still valid
      Widget *newTarget = newTargets[t];
      if (! newTarget->isValid() ) continue;

      //Pin if not an old target, re-pin otherwise
      if (! newTarget->pinholes[ id ].oldHole) {
        newTarget->pinholes[ id ].oldHole = true;
        pin( newTarget );
      } else repin( newTarget);

      //Mark as old target
      newTarget->pinholes[ id ].newHole = false;
      targets.pushBack( newTargets[t] );
    }
  }

  void Pin::lift ()
  {
    //Unpin old targets
    for (UintSize t=0; t<targets.size(); ++t)
    {
      Widget *target = targets[t];
      if (! target->isValid() ) continue;
      target->pinholes[ id ].oldHole = false;
      unpin( target );
    }

    //Clear old targets
    targets.clear();
  }

  void Event::trigger (Widget *w, bool bubbleUp)
  {
    //Trigget event on top widget
    this->target = w;
    w->onEvent( this );

    if (bubbleUp)
    {
      //Get all parents
      ArrayList<Widget*> ancestors;
      w->getAncestors( ancestors );

      //Triget event on ancestors
      for (UintSize a=0; a<ancestors.size(); ++a)
        ancestors[a]->onEvent( this );
    }
  }


  //--------------------------------------------------------------

  void Widget::setLoc (float x, float y) {
    loc.set( x, y );
  }

  void Widget::setLoc (const Vector2 &l) {
    setLoc( l.x, l.y );
  }
  
  void Widget::setSize (float w, float h) {
    box.set( w, h );
  }

  void Widget::setSize (const Vector2 &s) {
    setSize( s.x, s.y );
  }

  Matrix4x4 Widget::getGlobalMatrix()
  {
    Matrix4x4 g = mat;
    
    Widget *p = getParent();
    while (p != NULL)
    {
      g = p->getMatrix() * g;
      p = p->getParent();
    }

    return g;
  }

  bool Widget::hitTest (float x, float y)
  {
    Matrix4x4 invg = getGlobalMatrix().affineInverse();
    Vector3 hit = invg * Vector3( x,y,0 );
    if (hit.x < loc.x) return false;
    if (hit.y < loc.y) return false;
    if (hit.x > loc.x + box.x) return false;
    if (hit.y > loc.y + box.y) return false;
    return true;
  }

  Widget* Window::findTopWidgetAt (float x, float y)
  {
    //Traverse backwards
    for (int t=(int)traversal.size()-1; t>=0; --t) {
      Widget* w = traversal[t];

      //Check for hit
      if (w->hitTest(x, y))
        return w;
    }

    return NULL;
  }
}
}
