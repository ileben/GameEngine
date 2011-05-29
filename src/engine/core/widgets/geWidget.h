#ifndef __WIDGET_H
#define __WIDGET_H

#include "util/geUtil.h"
#include "math/geVectors.h"
#include "ui/uiUI.h"

namespace GE
{

  /*
  -------------------------------------------
  UI Events
  -------------------------------------------*/

  class MouseEvent : public Event
  {
    CLASS( MouseEvent, Event,
      c0327bc1,d74a,4369,ac4f56419ca834c3 );

  public:
    int x;
    int y;
  };

  class MouseClickEvent : public MouseEvent
  {
    CLASS( MouseClickEvent, MouseEvent,
      d082540c,2ad0,4757,85f311a1e70a00cd );

  public:
    int button;
    int state;
  };

  class MouseLeaveEvent : public MouseEvent
  {
    CLASS( MouseLeaveEvent, MouseEvent,
      24d401ef,58e6,410d,bf98462ff7fc0672 );
  };

  class MouseEnterEvent : public MouseEvent
  {
    CLASS( MouseEnterEvent, MouseEvent,
      c9b294cf,6881,432a,86bce8b2820a17f1 );
  };

  /*
  -----------------------------------------------
  Pins that trigger UI events on sets of widgets
  -----------------------------------------------*/

  class MouseClickPin : public Pin
  {
  public:
    MouseClickEvent eClick;

    void bothPins (Actor *w) {
      eClick.trigger( w );
    };
  };

  class MouseFocusPin : public Pin
  {
  public:
    MouseEnterEvent eEnter;
    MouseLeaveEvent eLeave;

    void thisPin (Actor *w)  {
      eEnter.trigger( w );
    }

    void otherPin (Actor *w) {
      eLeave.trigger( w );
    }
  };

  /*
  -------------------------------------------------------
  UI controller handles raw mouse / keyboard event input
  -------------------------------------------------------*/

  class UICtrl
  {
    MouseClickPin pMouseClick[2];
    bool mouseDown;

    MouseFocusPin pMouseFocus[2];
    Int iMouseFocus;

    Scene *scene;

  public:

    UICtrl();
    void bindScene (Scene *scene);
    void mouseClick (int button, int state, int x, int y);
    void mouseMove (int x, int y);
  };


  /*
  -------------------------------------------------------
  Widget is a UI-specific actor
  -------------------------------------------------------*/

  class Widget : public Actor
  {
    ABSTRACT( Widget, Actor,
      84d2920d,755a,4e22,89e785cf99967d93 );

    friend class Renderer;
    friend class UICtrl;
    
  protected:

    virtual void onEvent (Event *e);
    virtual void onMouseClick (MouseClickEvent *e) {}
    virtual void onMouseEnter (MouseEnterEvent *e) {}
    virtual void onMouseLeave (MouseLeaveEvent *e) {}
    virtual void draw() = 0;
  };
  
}//namespace GE
#endif//__WIDGET_H
