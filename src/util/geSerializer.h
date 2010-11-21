#ifndef __GESERIALIZER_H
#define __GESERIALIZER_H 1

namespace GE
{

  class Serializer
  {
    void serialize (Object *root, void **outData, UintSize *outSize);
    Object* deserialize (const void *data);
  };

}//namespace GE
#endif//__GESERIALIZER_H
