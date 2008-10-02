#ifndef __GESERIALIZE_H
#define __GESERIALIZE_H

namespace GE
{

  class GE_API_ENTRY SerializeManager
  {
  private:

    struct ResPtrInfo
    {
      ClassPtr    cls;        //class of the resource
      void       *ptr;        //pointer to the resource
      UintP       count;      //number of resources in the array
      UintP       offset;     //offset to the serialized resource
      bool        isptrptr;   //if true its a pointer to an array of pointers
      bool        detached;   //if true the pointer is not to be adjusted (e.g. root)
      UintP       ptroffset;  //offset to the serialized pointer to this resource
    };
    
    struct DynPtrInfo
    {
      void  *ptr;        //pointer to data on the heap
      UintP  size;       //the size of the memory to be copied
      UintP  ptroffset;  //offset to the serialized pointer to this data
    };

    struct ClsHeader
    {
      ClassID id;
      UintP  count;
      UintP  offset;
    };
    
    struct PtrHeader
    {
      UintP offset;
    };
    
    Uint8 *data;
    UintP  offset;
    ResPtrInfo *current;
    OCC::LinkedList <ResPtrInfo> resQueue;
    OCC::LinkedList <DynPtrInfo> dynQueue;
    OCC::ArrayList <ClsHeader> clsList;
    OCC::ArrayList <PtrHeader> ptrList;
    bool simulate;
    
    void copy (void *ptr, UintP size);
    void adjust (UintP ptrOffset);
    void run (ClassPtr rootCls, void *rootPtr);

  public:
    
    void serialize (ClassPtr cls, void *root, void **outData, UintP *outSize);
    void resourcePtr (ClassPtr cls, void **pptr, UintP count);
    void resourcePtrPtr (ClassPtr cls, void ***pptr, UintP count);
    void dynamicPtr (void **pptr, UintP size);
    
    template <class TR> void serialize (TR *root, void **outData, UintP *outSize)
      { serialize (Class(TR), root, outData, outSize); }
    
    template <class TR> void resourcePtr (TR **pptr, UintP count=1)
      { resourcePtr (Class(TR), (void**)pptr, count); }
    
    template <class TR> void resourcePtrPtr (TR ***pptr, UintP count=1)
      { resourcePtrPtr (Class(TR), (void***)pptr, count); }
    
    template <class TD> void dynamicPtr (TD **pptr, UintP size)
      { dynamicPtr ((void**)pptr, size); }
    
    void* deserialize (void *data);
  };

  typedef SerializeManager SM;

}//namespace GE
#endif//__GESERIALIZE_H
