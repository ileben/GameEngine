#ifndef __GESCENE_H
#define __GESCENE_H

namespace GE
{ 
  /*--------------------------------------------
   * Forward declarations
   *--------------------------------------------*/

  class Camera;

  /*----------------------------------------------
   * A renderable scene - root for object tree
   *----------------------------------------------*/

  class GE_API_ENTRY Scene : public Group
  {
    DECLARE_SUBCLASS (Scene, Group); DECLARE_END;

  private:
    Camera *cam;

  public:

    void bindCamera(Camera *cam);
  };
}

#endif /* __GESCENE_H */