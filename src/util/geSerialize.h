#ifndef __GESERIALIZE_H
#define __GESERIALIZE_H

namespace GE
{

  class GE_API_ENTRY SerializeManager
  {
  private:

    struct ObjectNode
    {
      void *var;             //pointer to variable
      ClassPtr cls;          //class of object
      UintSize offset;       //offset to serialized object
      UintSize ptroffset;    //offset to serialized pointer to object
      bool pointedTo;        //if true, a serialized pointer to object exists
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
      std::deque <ObjectNode> objQueue;
      std::vector <ClsHeader> clsList;
      std::vector <PtrHeader> ptrList;
      SerializeManager *sm;
      bool simulate;
      
      void enqueueObjVar (ClassPtr cls, void *var, UintSize offset=0);
      void enqueueObjPtr (ClassPtr cls, void *var, UintSize ptroffset=0);

      virtual void run (ClassPtr rootCls, void **rootPtr) = 0;
      
      void rootPtr (ClassPtr cls, void **pptr);
      void arrayMemberPtr (ClassPtr cls, void **pptr, UintSize ptroffset);

      void store (void *ptr, UintSize size);
      void load (void *ptr, UintSize size);

      void reset (UintSize startOffset, bool realRun);
    };

    class StateSerial : public State
    { public:
      virtual void dataVar (void *ptr, UintSize size);
      virtual void dataPtr (void **pptr, UintSize size);
      virtual void objectVar (ClassPtr cls, void *ptr);
      virtual void objectArray (ClassPtr cls, void **pptr, UintSize count);
      virtual void objectPtr (ClassPtr cls, void **pptr);
      virtual void objectPtrArray (ClassPtr cls, void ***pptr, UintSize count);
      virtual void run (ClassPtr rootCls, void **rootPtr);
      void adjust (UintSize ptrOffset);
    };

    class StateSave : public StateSerial
    { public:
      virtual void dataVar (void *ptr, UintSize size);
      virtual void run (ClassPtr rootCls, void **rootPtr);
    };

    class StateLoad : public StateSerial
    { public:
      virtual void dataVar (void *ptr, UintSize size);
      virtual void run (ClassPtr rootCls, void **rootPtr);
    };
    
    StateSerial stateSerial;
    StateSave stateSave;
    StateLoad stateLoad;
    State *state;
    
    void copy (void *ptr, UintSize size);
    void adjust (UintSize ptrOffset);
    void run (ClassPtr rootCls, void **rootPtr);

  public:

    bool isSerializing ();
    bool isDeserializing ();
    bool isSaving ();
    bool isLoading ();
    
    void dataVar (void *ptr, UintSize size);
    void dataPtr (void **pptr, UintSize size);
    void objectVar (ClassPtr cls, void *ptr);
    void objectArray (ClassPtr cls, void **pptr, UintSize count);
    void objectPtr (ClassPtr cls, void **pptr);
    void objectPtrArray (ClassPtr cls, void ***pptr, UintSize count);
    
    void serialize (ClassPtr cls, void *root, void **outData, UintSize *outSize);
    void save (ClassPtr cls, void *root, void **outData, UintSize *outSize);

    template <class TV> void dataVar (TV *ptr)
      { dataVar ((void*)ptr, sizeof(TV)); }

    template <class TD> void dataPtr (TD **pptr, UintSize size)
      { dataPtr ((void**)pptr, size); }

    template <class TV> void objectVar (TV *ptr)
      { objectVar (Class(TV), (void*)ptr); }

    template <class TR> void objectArray (TR **pptr, UintSize count)
      { objectArray (Class(TR), (void**)pptr, count); }
    
    template <class TR> void objectPtr (TR **pptr)
      { objectPtr (Class(TR), (void**)pptr); }

    template <class TR> void objectPtrArray (TR ***pptr, UintSize count)
      { objectPtrArray (Class(TR), (void***)pptr, count); }

    template <class TR> void serialize (TR *root, void **outData, UintSize *outSize)
      { serialize (Class(TR), root, outData, outSize); }

    template <class TR> void save (TR *root, void **outData, UintSize *outSize)
      { save (Class(TR), root, outData, outSize); }
    
    void* deserialize (const void *data, ClassPtr *outCls=NULL);
    void* load (const void *data, ClassPtr *outCls=NULL);

    const void* getSignature ();
    UintSize getSignatureSize ();
    bool checkSignature (const void *data);
  };
  
  typedef SerializeManager SM;

}//namespace GE
#endif//__GESERIALIZE_H
