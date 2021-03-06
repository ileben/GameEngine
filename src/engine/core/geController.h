#ifndef __GECONTROLLER_H
#define __GECONTROLLER_H

#include "util/geUtil.h"
#include "math/geVectors.h"

namespace GE
{
  /*
  ===========================
  Forward declarations
  ===========================*/

  class Camera3D;

  /*
  ===========================
  Controllers
  ===========================*/

  class Controller
  {
  protected:

    Camera3D *cam;

  public:

    Controller();
    virtual ~Controller() {};

    void attachCamera (Camera3D *cam);
    Camera3D *getAttachedCamera ();

    virtual void keyDown (unsigned char key) {}
    virtual void keyUp (unsigned char key) {}
    virtual void mouseClick (int button, int state, int x, int y) {}
    virtual void mouseMove (int x, int y) {};
    virtual void resetState () {}
    virtual void tick () {};
  };

  class FpsController : public Controller
  {
  private:

    bool lookEnabled;
    bool moveEnabled;
    bool clickToLook;

    bool mouseDown;
    Vector2 lastMouse;

    float moveSpeed;
    int moveDir;
    float strafeSpeed;
    int strafeDir;
    float climbSpeed;
    int climbDir;

  public:
    FpsController();
    
    void setMoveSpeed (Float speed);
    Float getMoveSpeed ();

    void setClickToLook (bool enabled);
    bool getClickToLook ();

    void enableMove (bool enabled);
    void enableLook (bool enabled);

    virtual void keyDown (unsigned char key);
    virtual void keyUp (unsigned char key);
    virtual void mouseClick (int button, int state, int x, int y);
    virtual void mouseMove (int x, int y);
    virtual void resetState ();
    virtual void tick ();
  };

  class EditorController : public Controller
  {
  public:
    virtual void keyDown (unsigned char key) {}
    virtual void keyUp (unsigned char key) {}
    virtual void mouseClick (int button, int state, int x, int y) {}
    virtual void mouseMove (int x, int y) {};
  };

}//namespace GE
#endif//__GECONTROLLER_H
