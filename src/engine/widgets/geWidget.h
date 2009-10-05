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
    DECLARE_SUBCLASS( MouseEvent, Event );
    DECLARE_END;

  public:
    int x;
    int y;
  };

  class MouseClickEvent : public MouseEvent
  {
    DECLARE_SUBCLASS( MouseClickEvent, MouseEvent );
    DECLARE_END;

  public:
    int button;
    int state;
  };

  class MouseLeaveEvent : public MouseEvent
  {
    DECLARE_SUBCLASS( MouseLeaveEvent, MouseEvent );
    DECLARE_END;
  };

  class MouseEnterEvent : public MouseEvent
  {
    DECLARE_SUBCLASS( MouseEnterEvent, MouseEvent );
    DECLARE_END;
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
    DECLARE_SUBABSTRACT( Widget, Actor );
    DECLARE_END;

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
