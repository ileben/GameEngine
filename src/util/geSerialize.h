#ifndef __GESERIALIZE_H
#define __GESERIALIZE_H

namespace GE
{

  class GE_API_ENTRY SerializeManager
  {
  private:

    struct ResPtrInfo
    {
      ClassPtr   cls;        //class of the resource
      void      *ptr;        //pointer to the resource
      UintSize   count;      //number of variables in array
      UintSize   offset;     //offset to the serialized resource
      bool       isptr;      //if true its a pointer else its a direct member
      bool       isarray;    //if true its an array else its a single variable
      bool       detached;   //if true the pointer is not to be adjusted (e.g. root)
      UintSize   ptroffset;  //offset to the serialized pointer to this resource
    };
    
    struct DynPtrInfo
    {
      void     *ptr;        //pointer to data on the heap
      UintSize  size;       //the size of the memory to be copied
      UintSize  ptroffset;  //offset to the serialized pointer to this data
    };

    struct ClsHeader
    {
      ClassID   id;
      UintSize  count;
      UintSize  offset;
    };
    
    struct PtrHeader
    {
      UintSize offset;
    };

    class State
    {
    public:
      Uint8 *data;
      UintSize offset;
      ResPtrInfo *current;
      std::deque <ResPtrInfo> resQueue;
      std::deque <DynPtrInfo> dynQueue;
      std::vector <ClsHeader> clsList;
      std::vector <PtrHeader> ptrList;
      SerializeManager *sm;
      bool simulate;
      
      virtual void dataVar (void *ptr, UintSize size) = 0;
      virtual void objectVar (ClassPtr cls, void *ptr) = 0;
      virtual void objectPtr (ClassPtr cls, void **pptr) = 0;
      virtual void objectArray (ClassPtr cls, void ***pptr, UintSize count) = 0;
      virtual void dataPtr (void **pptr, UintSize size) = 0;
      virtual void run (ClassPtr rootCls, void *rootPtr) = 0;
      
      void rootPtr (ClassPtr cls, void *ptr);
      void arrayPtr (ClassPtr cls, void *ptr, UintSize ptroffset);
      void store (void *ptr, UintSize size);
      void load (void *ptr, UintSize size);
      void reset (UintSize startOffset, bool realRun);
    };

    class StateSerial : public State
    { public:
      virtual void dataVar (void *ptr, UintSize size);
      virtual void objectVar (ClassPtr cls, void *ptr);
      virtual void objectPtr (ClassPtr cls, void **pptr);
      virtual void objectArray (ClassPtr cls, void ***pptr, UintSize count);
      virtual void dataPtr (void **pptr, UintSize size);
      virtual void run (ClassPtr rootCls, void *rootPtr);
      void adjust (UintSize ptrOffset);
    };

    class StateSave : public StateSerial
    { public:
      virtual void dataVar (void *ptr, UintSize size);
      virtual void objectVar (ClassPtr cls, void *ptr);
      virtual void objectPtr (ClassPtr cls, void **pptr);
      virtual void objectArray (ClassPtr cls, void ***pptr, UintSize count);
      virtual void dataPtr (void **pptr, UintSize size);
      virtual void run (ClassPtr rootCls, void *rootPtr);
    };

    class StateLoad : public StateSerial
    { public:
      virtual void dataVar (void *ptr, UintSize size);
      virtual void objectVar (ClassPtr cls, void *ptr);
      virtual void objectPtr (ClassPtr cls, void **pptr);
      virtual void objectArray (ClassPtr cls, void ***pptr, UintSize count);
      virtual void dataPtr (void **pptr, UintSize size);
      virtual void run (ClassPtr rootCls, void *rootPtr);
    };
    
    StateSerial stateSerial;
    StateSave stateSave;
    StateLoad stateLoad;
    State *state;
    
    void copy (void *ptr, UintSize size);
    void adjust (UintSize ptrOffset);
    void run (ClassPtr rootCls, void *rootPtr);

  public:

    bool isSerializing ();
    bool isDeserializing ();
    bool isSaving ();
    bool isLoading ();
    
    void dataVar (void *ptr, UintSize size);
    void objectVar (ClassPtr cls, void *ptr);
    void objectPtr (ClassPtr cls, void **pptr);
    void objectArray (ClassPtr cls, void ***pptr, UintSize count);
    void dataPtr (void **pptr, UintSize size);
    
    void serialize (ClassPtr cls, void *root, void **outData, UintSize *outSize);
    void save (ClassPtr cls, void *root, void **outData, UintSize *outSize);

    template <class TV> void dataVar (TV *ptr)
      { dataVar ((void*)ptr, sizeof(TV)); }

    template <class TV> void objectVar (TV *ptr)
      { objectVar (Class(TV), (void*)ptr); }
    
    template <class TR> void objectPtr (TR **pptr)
      { objectPtr (Class(TR), (void**)pptr); }
    
    template <class TR> void objectArray (TR ***pptr, UintSize count=1)
      { objectArray (Class(TR), (void***)pptr, count); }
    
    template <class TD> void dataPtr (TD **pptr, UintSize size)
      { dataPtr ((void**)pptr, size); }

    template <class TR> void serialize (TR *root, void **outData, UintSize *outSize)
      { serialize (Class(TR), root, outData, outSize); }

    template <class TR> void save (TR *root, void **outData, UintSize *outSize)
      { save (Class(TR), root, outData, outSize); }
    
    void* deserialize (void *data);
    void* load (void *data);
  };
  
  typedef SerializeManager SM;

}//namespace GE
#endif//__GESERIALIZE_H
