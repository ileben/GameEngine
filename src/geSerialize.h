#ifndef __GESERIALIZE_H
#define __GESERIALIZE_H

namespace GE
{

  class SerializeManager;
  
  class GE_API_ENTRY IResource
  {
  public:
    virtual Uint32 getID () = 0;
    virtual UintP getSize () = 0;
    virtual void getPointers (SerializeManager *sm) = 0;
  };

  class GE_API_ENTRY SerializeManager
  {
  private:

    struct ResPtrInfo
    {
      IResource  *ptr;        //pointer to the resource
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
      Uint32 id;
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
    void run (IResource *root);

  public:
    
    void resourcePtr (void *pptr);
    void arrayPtr (void *pptr, UintP count);
    void dynamicPtr (void *pptr, UintP size);
    void serialize (IResource *root, void **outData, UintP *outSize);
    IResource* deserialize (void *data);
  };

}//namespace GE
#endif//__GESERIALIZE_H
