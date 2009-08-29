#include "util/geUtil.h"
#include "math/geMath.h"

namespace GE
{
namespace UI
{

  /*
  ----------------------------------------------
  Forward declarations
  ----------------------------------------------*/
  class Widget;
  class Window;
  class Stage;


  /*
  ----------------------------------------------
  Pin
  ----------------------------------------------*/

  class Pin
  {
    friend class Stage;

  private:
    Uint id;
    ArrayList <Widget*> targets;

    virtual void pin (Widget *w) {};
    virtual void unpin (Widget *w) {};
    virtual void repin (Widget *w) {};

  public:
    virtual void drop (Widget *w);
    virtual void lift ();
  };

  struct Pinhole
  {
    bool oldHole;
    bool newHole;

    Pinhole() : oldHole (false), newHole (false) {}
  };

  /*
  ----------------------------------------------
  Event
  ----------------------------------------------*/

  class Event
  {
    DECLARE_CLASS( Event );
    DECLARE_END;

  private:
    Widget *target;

  public:
    void trigger (Widget *w, bool bubbleUp=false);
  };

  /*
  ----------------------------------------------
  Stage
  ----------------------------------------------*/

  class Stage
  {
  private:
    static Stage *instance;

  public:
    static Stage* GetInstance() { return instance; }

  private:
    ArrayList <Pin*> pins;
    ArrayList <Widget*> invalidWidgets;
  
  public:
    Stage();
    void registerPin (Pin *p);
    ArrayList <Pin*> & getPins () { return pins; }
    void clearInvalid ();
  };

  /*
  ----------------------------------------------
  Widget
  ----------------------------------------------*/

  class Widget
  {
    DECLARE_CLASS( Widget );
    DECLARE_END;
    friend class Pin;

  protected:
    Vector2 loc;
    Vector2 box;
    Matrix4x4 mat;

    Widget *parent;
    ArrayList<Widget*> children;
    ArrayList<Pinhole> pinholes;
    bool valid;

  public:
    Widget ();
    virtual ~Widget() {}

    void destroy ();
    bool isValid () { return valid; }

    //Location
    Matrix4x4& getMatrix() { return mat; }
    Matrix4x4 getGlobalMatrix();

    void setLoc (float x, float y);
    void setLoc (const Vector2 &loc);
    Vector2& getLoc () { return loc; }
    
    virtual void setSize (float w, float h);
    virtual void setSize (const Vector2 &s);
    virtual Vector2 getSize () { return box; }

    inline Float getLeft ();
    inline Float getRight ();
    inline Float getTop ();
    inline Float getBottom ();
    inline Vector2 getTopLeft ();
    inline Vector2 getTopRight ();
    inline Vector2 getBottomLeft ();
    inline Vector2 getBottomRight ();

    virtual bool hitTest (float x, float y);

    //Structure
    void addChild (Widget* c);
    void removeChild (Widget* c);
    void setParent (Widget* c);
    void getAncestors (ArrayList<Widget*> &list);
    const ArrayList<Widget*> & getChildren () { return children; }

    Widget* getParent () { return parent; }
    Window* getWindow();

    //Events
    virtual void onEvent (Event *e) {}
  };

  /*
  ----------------------------------------------
  Window
  ----------------------------------------------*/

  class Window : public Widget
  {
    DECLARE_SUBCLASS( Window, Widget );
    DECLARE_END;

  private:
    bool changed;
    ArrayList <Widget*> traversal;

  public:
    Window();

    void updateChanges ();
    void markChanged ();
    bool hasChanged () { return changed; }

    Widget* findTopWidgetAt (float x, float y);
    const ArrayList<Widget*> & getTraversal () { return traversal; }
  };

  /*
  Inlines
  -----------------------------------------------*/

  Float Widget::getLeft ()
    { return loc.x; }
  
  Float Widget::getRight ()
    { return loc.x + box.x; }
  
  Float Widget::getTop ()
    { return loc.y; }
  
  Float Widget::getBottom ()
    { return loc.y + box.y; }

  Vector2 Widget::getTopLeft ()
    { return Vector2( getLeft(), getTop() ); }
  
  Vector2 Widget::getTopRight ()
    { return Vector2( getRight(), getTop() ); }
  
  Vector2 Widget::getBottomLeft ()
    { return Vector2( getLeft(), getBottom() ); }

  Vector2 Widget::getBottomRight ()
    { return Vector2( getRight(), getBottom() ); }

}//namespace UI
}//namespace GE
