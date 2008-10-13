#ifndef __GECHARACTER_H
#define __GECHARACTER_H

namespace GE
{
  
  class GE_API_ENTRY MaxCharacter
  {
    DECLARE_SERIAL_CLASS (MaxCharacter)
    DECLARE_CALLBACK (ClassEvent::Serialize, serialize);
    DECLARE_END;

  public:
    
    SkinMesh *mesh;
    SkinPose *pose;
    ClassArrayList <SkinAnim> anims;
    
    void serialize (void *sm)
    {
      if (mesh != NULL)
        ((SM*)sm)->objectPtr( &mesh );
      if (pose != NULL)
        ((SM*)sm)->objectPtr( &pose );
      
      ((SM*)sm)->objectVar( &anims );
    }
    
    MaxCharacter (SM *sm) : anims (sm)
    {}
    
    MaxCharacter ()
    {
      mesh = NULL;
      pose = NULL;
    }
    
    ~MaxCharacter ()
    {
      if (mesh != NULL)
        delete mesh;

      if (pose != NULL)
        delete pose;
      
      for (UintSize a=0; a<anims.size(); ++a)
        delete anims[ a ];
    }
  };


}//namespace GE
#endif//__GECHARACTER_H
