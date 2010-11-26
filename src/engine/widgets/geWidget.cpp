#include "engine/widgets/geWidget.h"
#include "engine/geGLHeaders.h"

namespace GE
{

  UICtrl::UICtrl()
  {
    scene = NULL;
    iMouseFocus = 0;
    mouseDown = false;
  }

  void UICtrl::bindScene (Scene *s)
  {
    scene = s;
  }
  
  void UICtrl::mouseClick (int button, int state, int x, int y)
  {
    if (scene == NULL) return;
    if (button != GLUT_LEFT_BUTTON) return;

    Actor *top = scene->findTopActorAt( (float) x, (float) y );

    if (state == GLUT_DOWN)
    {
      pMouseClick[0].drop( top );
      mouseDown = true;
    }

    if (state == GLUT_UP)
    {
      pMouseClick[1].eClick.button = button;
      pMouseClick[1].eClick.state = state;
      pMouseClick[1].eClick.x = x;
      pMouseClick[1].eClick.y = y;

      pMouseClick[1].drop( top );
      pMouseClick[1].intersect( pMouseClick[0] );

      mouseDown = false;
      mouseMove( x, y );
    }
  }

  void UICtrl::mouseMove (int x, int y)
  {
    if (scene == NULL) return;

    if (!mouseDown)
    {
      Actor *top = scene->findTopActorAt( (float) x, (float) y );
      
      int iNew = (iMouseFocus+1) % 2;
      pMouseFocus[ iNew ].drop( top );
      pMouseFocus[ iNew ].intersect( pMouseFocus[ iMouseFocus ] );
      iMouseFocus = iNew;
    }
  }

  void Widget::onEvent (Event *e)
  {
    MouseClickEvent *eClick = Class::SafeCast< MouseClickEvent >( e );
    if (eClick != NULL) onMouseClick( eClick );

    MouseEnterEvent *eEnter = Class::SafeCast< MouseEnterEvent >( e );
    if (eEnter != NULL) onMouseEnter( eEnter );

    MouseLeaveEvent *eLeave = Class::SafeCast< MouseLeaveEvent >( e );
    if (eLeave != NULL) onMouseLeave( eLeave );
  }

}//namespace GE
