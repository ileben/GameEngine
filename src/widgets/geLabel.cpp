#define GE_API_EXPORT
#include "../geEngine.h"
#include "../geGLHeaders.h"
using namespace OCC;

namespace GE
{
  DEFINE_CLASS (Label);
  
  void Label::setText (const String &newText) {
    text = newText;
  }
  
  void Label::setColor (const Vector3 &c) {
    color = c;
  }
  
  void Label::draw ()
  {
    float k = 0.15f;
    float x = location.x + 0.5f;
    float y = location.y - 0.5f;
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_LINE_SMOOTH);
    glDisable(GL_MULTISAMPLE);
    
    glColor3fv((GLfloat*)&color);
    
    //Glut assumes bottom-left origin
    //so we mirror it vertically
    glPushMatrix();
    glLoadIdentity();
    glTranslatef(x,y,0);
    glScalef(k,-k,k);
    glLineWidth(1.0f);
    
    for (int i=0; i<text.length(); ++i) {
      if (text[i] == '\n') {
        y += 22;
        glLoadIdentity();
        glTranslatef(x, y, 0);
        glScalef(k,-k,k);
        continue; }
      glutStrokeCharacter(GLUT_STROKE_ROMAN, text[i]);
    }
    
    glPopMatrix();
    glEnable(GL_MULTISAMPLE);
  }
  
}//namespace GE
