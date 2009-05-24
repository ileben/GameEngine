#include "engine/geController.h"
#include "engine/geCamera.h"
#include "engine/geKernel.h"
#include "engine/geGLHeaders.h"

namespace GE
{
  Controller::Controller ()
  {
    cam = NULL;
  }

  void Controller::attachCamera (Camera3D *c) {
    cam = c;
  }

  Camera3D* Controller::getAttachedCamera () {
    return cam;
  }

  FpsController::FpsController()
  {
    moveSpeed = 15.0f;
    moveDir = 0;
    strafeSpeed = 15.0f;
    strafeDir = 0;
    climbSpeed = 15.0f;
    climbDir = 0;
  }

  void FpsController::setMoveSpeed (Float speed)
  {
    moveSpeed = strafeSpeed = climbSpeed = speed;
  }

  Float FpsController::getMoveSpeed ()
  {
    return moveSpeed;
  }

  void FpsController::keyDown (unsigned char key)
  {
    switch (key)
    {
    case 'e':
      moveDir = 1;
      break;
    case 'd':
      moveDir = -1;
      break;
    case 'f':
      strafeDir = 1;
      break;
    case 's':
      strafeDir = -1;
      break;
    case ' ':
      climbDir = 1;
      break;
    }
  }

  void FpsController::keyUp (unsigned char key)
  {
    switch (key)
    {
    case 'e':
      if (moveDir == 1) moveDir = 0;
      break;
    case 'd':
      if (moveDir == -1) moveDir = 0;
      break;
    case 'f':
      if (strafeDir == 1) strafeDir = 0;
      break;
    case 's':
      if (strafeDir == -1) strafeDir = 0;
      break;
    case ' ':
      if (climbDir == 1) climbDir = 0;
      break;
    }
  }

  void FpsController::mouseClick (int button, int state, int x, int y)
  {
    if (state != GLUT_DOWN)
    {
      mouseDown = false;
      return;
    }

    lastMouse.set( (Float)x, (Float)y );
    mouseDown = true;
  }

  void FpsController::mouseMove (int x, int y)
  {
    if (!mouseDown) return;

    Vector2 diff = Vector2( (Float)x,(Float)y ) - lastMouse;
    lastMouse.set( (Float)x, (Float)y );
    
    Float angleH = diff.x * (2*PI) / 400;
    Float angleV = diff.y * (2*PI) / 400;

    cam->setCenter( cam->getEye() );
    cam->orbitH( angleH, true );
    cam->orbitV( angleV, true );
  }

  void FpsController::tick()
  {
    Float interval = Kernel::GetInstance()->getInterval();

    if (moveDir != 0) {
      Vector3 look = cam->getLook();
      cam->translate( look * moveSpeed * (Float)moveDir * interval );
    }

    if (strafeDir != 0) {
      Vector3 side = cam->getSide();
      cam->translate( side * strafeSpeed * (Float)strafeDir * interval );
    }

    if (climbSpeed != 0) {
      Vector3 up = Vector3(0,1,0);
      cam->translate( up * climbSpeed * (Float)climbDir * interval );
    }
  }

}//namespace GE
