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


#endif//__APP_H
