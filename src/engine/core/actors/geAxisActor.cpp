#include "geAxisActor.h"
#include "core/geGLHeaders.h"

namespace GE
{
  void AxisActor::render (MaterialID materialID)
  {
    glBegin( GL_LINES );
    glColor3f ( 1, 0, 0 );
    glVertex3f( 0, 0, 0 );
    glVertex3f( 1, 0, 0 );

    glColor3f ( 0, 1, 0 );
    glVertex3f( 0, 0, 0 );
    glVertex3f( 0, 1, 0 );

    glColor3f ( 0, 0, 1 );
    glVertex3f( 0, 0, 0 );
    glVertex3f( 0, 0, 1 );
    glEnd();
  }

}//namespace GE
