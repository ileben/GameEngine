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

    class State
    {
    public:
      Uint8 *data;
      UintP offset;
      ResPtrInfo *current;
      OCC::LinkedList <ResPtrInfo> resQueue;
      OCC::LinkedList <DynPtrInfo> dynQueue;
      OCC::ArrayList <ClsHeader> clsList;
      OCC::ArrayList <PtrHeader> ptrList;
      SerializeManager *sm;
      bool simulate;
      
      virtual void memberVar (void *ptr, UintP size) = 0;
      virtual void resourcePtr (ClassPtr cls, void **pptr) = 0;
      virtual void resourcePtrPtr (ClassPtr cls, void ***pptr, UintP count) = 0;
      virtual void dynamicPtr (void **pptr, UintP size) = 0;
      virtual void run (ClassPtr rootCls, void *rootPtr) = 0;
      
      void rootPtr (ClassPtr cls, void *ptr);
      void arrayPtr (ClassPtr cls, void *ptr, UintP ptroffset);
      void store (void *ptr, UintP size);
      void load (void *ptr, UintP size);
      void reset (UintP startOffset, bool realRun);
    };

    class StateSerial : public State
    { public:
      virtual void memberVar (void *ptr, UintP size);
      virtual void resourcePtr (ClassPtr cls, void **pptr);
      virtual void resourcePtrPtr (ClassPtr cls, void ***pptr, UintP count);
      virtual void dynamicPtr (void **pptr, UintP size);
      virtual void run (ClassPtr rootCls, void *rootPtr);
      void adjust (UintP ptrOffset);
    };

    class StateSave : public StateSerial
    { public:
      virtual void memberVar (void *ptr, UintP size);
      virtual void resourcePtr (ClassPtr cls, void **pptr);
      virtual void resourcePtrPtr (ClassPtr cls, void ***pptr, UintP count);
      virtual void dynamicPtr (void **pptr, UintP size);
      virtual void run (ClassPtr rootCls, void *rootPtr);
    };

    class StateLoad : public StateSerial
    { public:
      virtual void memberVar (void *ptr, UintP size);
      virtual void resourcePtr (ClassPtr cls, void **pptr);
      virtual void resourcePtrPtr (ClassPtr cls, void ***pptr, UintP count);
      virtual void dynamicPtr (void **pptr, UintP size);
      virtual void run (ClassPtr rootCls, void *rootPtr);
    };
    
    StateSerial stateSerial;
    StateSave stateSave;
    StateLoad stateLoad;
    State *state;
    
    void copy (void *ptr, UintP size);
    void adjust (UintP ptrOffset);
    void run (ClassPtr rootCls, void *rootPtr);

  public:

    bool isSerializing ();
    bool isDeserializing ();
    bool isSaving ();
    bool isLoading ();
    
    void memberVar (void *ptr, UintP size);
    void resourcePtr (ClassPtr cls, void **pptr);
    void resourcePtrPtr (ClassPtr cls, void ***pptr, UintP count);
    void dynamicPtr (void **pptr, UintP size);
    
    void serialize (ClassPtr cls, void *root, void **outData, UintP *outSize);
    void save (ClassPtr cls, void *root, void **outData, UintP *outSize);

    template <class TM> void memberVar (TM *ptr)
      { memberVar ((void*)ptr, sizeof(TM)); }
    
    template <class TR> void resourcePtr (TR **pptr)
      { resourcePtr (Class(TR), (void**)pptr); }
    
    template <class TR> void resourcePtrPtr (TR ***pptr, UintP count=1)
      { resourcePtrPtr (Class(TR), (void***)pptr, count); }
    
    template <class TD> void dynamicPtr (TD **pptr, UintP size)
      { dynamicPtr ((void**)pptr, size); }

    template <class TR> void serialize (TR *root, void **outData, UintP *outSize)
      { serialize (Class(TR), root, outData, outSize); }

    template <class TR> void save (TR *root, void **outData, UintP *outSize)
      { save (Class(TR), root, outData, outSize); }
    
    void* deserialize (void *data);
    void* load (void *data);
  };
  
  typedef SerializeManager SM;

}//namespace GE
#endif//__GESERIALIZE_H
