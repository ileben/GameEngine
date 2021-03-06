#ifndef __APP_H
#define __APP_H

#include <cstdlib>
#include <cstdio>
#include <iostream>

#include <engine/geEngine.h>
#include <app/appConvo.h>
using namespace GE;

typedef void (*AnimateFunc) ();
typedef void (*KeyFunc) (unsigned char key);

void appAnimateFunc (AnimateFunc func);
void appKeyDownFunc (KeyFunc func);
void appInit (int resX, int resY);

Scene* appScene2D ();
Scene3D* appScene3D (const CharString &filename);
FpsController* appCamCtrl (Camera3D *cam);
void appRun ();

void appSwitchCamera (Camera3D *cam);
void appSwitchScene (Scene3D *scene);
AnimController* appPlayAnim (const CharString &name);
AnimController* appGetAnim (const CharString &name);
void appFinishAnim (const CharString &name);
void appStopAnim (const CharString &name);
BoundingBox appProjectActor (Actor3D *actor, Camera *camera,
                             Float viewX, Float viewY, Float viewW, Float viewH);

namespace ActorMouseEvent {
  enum Enum {
    Enter,
    Leave,
    Click
  };}

typedef void (*ActorFunc) (ActorMouseEvent::Enum evt, Actor3D *actor);
void appActorMouseFunc (ActorMouseEvent::Enum evt, Actor3D *actor, ActorFunc func);
void removeActorMouseFunc (ActorMouseEvent::Enum evt, Actor3D *actor);

#endif//__APP_H
