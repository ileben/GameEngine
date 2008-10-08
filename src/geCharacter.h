#ifndef __GECHARACTER_H
#define __GECHARACTER_H

namespace GE
{
  
  class GE_API_ENTRY MaxCharacter
  {
    DECLARE_SERIAL_CLASS (MaxCharacter)
    DECLARE_CALLBACK (CLSEVT_SERIALIZE, serialize);
    DECLARE_END;

  public:
    
    SkinMesh *mesh;
    SkinPose *pose;
    ClassArrayList <SkinAnim> *anims;
    
    void serialize (void *sm)
    {
      if (mesh != NULL)
        ((SM*)sm)->resourcePtr (&mesh);
      if (pose != NULL)
        ((SM*)sm)->resourcePtr (&pose);
      
      ((SM*)sm)->resourcePtr (&anims);
    }
    
    MaxCharacter (SM *sm) {}
    
    MaxCharacter ()
    {
      mesh = NULL;
      pose = NULL;
      anims = new ClassArrayList <SkinAnim>;
    }
    
    ~MaxCharacter ()
    {
      if (mesh != NULL)
        delete mesh;

      if (pose != NULL)
        delete pose;
      
      for (int a=0; a<anims->size(); ++a)
        delete anims->at (a);
      
      delete anims;
    }
  };


}//namespace GE
#endif//__GECHARACTER_H
