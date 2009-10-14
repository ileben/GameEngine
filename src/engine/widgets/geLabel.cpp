#include "geLabel.h"
#include "engine/geGLHeaders.h"

namespace GE
{
  DEFINE_CLASS (Label);
  
  void Label::setText (const String &newText) {
    text = newText;
    ByteString cstr = newText;
    setSize( (Float) glutStrokeLength( GLUT_STROKE_ROMAN, cstr.buffer() ), 20.0f );
    //setSize( (Float) glutBitmapLength( GLUT_BITMAP_TIMES_ROMAN_24 , cstr.buffer() ), 20.0f );
  }
  
  void Label::setColor (const Vector3 &c) {
    color = c;
  }
  
  void Label::draw ()
  {
    float k = 0.15f;
    float x = getLeft() + 0.5f;
    float y = getTop() + 20.0f - 0.5f;
    
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
        //glRasterPos2f(x,y);
        //glTranslatef(0,0, 0);
        continue; }
      glutStrokeCharacter(GLUT_STROKE_ROMAN, text[i]);
      //glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24 , text[i]);
    }
    
    glPopMatrix();
    glEnable(GL_MULTISAMPLE);
  }
  
}//namespace GE
