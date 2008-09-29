#ifndef __GESERIALIZE_H
#define __GESERIALIZE_H

namespace GE
{

  class SerializeManager;
  
  class GE_API_ENTRY IResource { public:
    virtual void getPointers (SerializeManager *sm) = 0;
  };
  
  class GE_API_ENTRY SerializeManager
  {
  private:

    struct ResPtrInfo
    {
      ClassPtr    cls;        //class of the resource
      void       *ptr;        //pointer to the resource
      UintP       count;      //number of resources in the array
      UintP       offset;     //offset to the serialized resource
      bool        detached;   //if true there is no pointer to this resource
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
    void dynamicPtr (void **pptr, UintP size);
    
    template <class R> void serialize (R *root, void **outData, UintP *outSize)
      { serialize (ClassOf(root), root, outData, outSize); }
    
    template <class R> void resourcePtr (R **pptr, UintP count=1)
      { resourcePtr (ClassOf(*pptr), (void**)pptr, count); }
    
    template <class D> void dynamicPtr (D **pptr, UintP size)
      { dynamicPtr ((void**)pptr, size); }
    
    IResource* deserialize (void *data);
  };

}//namespace GE
#endif//__GESERIALIZE_H
