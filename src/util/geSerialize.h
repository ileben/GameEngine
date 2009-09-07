#ifndef __GESERIALIZE_H
#define __GESERIALIZE_H

namespace GE
{

  class GE_API_ENTRY SerializeManager
  {
  private:

    struct ObjectInfo
    {
      void *var;             //pointer to variable
      ClassPtr cls;          //class of object
      UintSize offset;       //offset to serialized object
      UintSize ptroffset;    //offset to serialized pointer to object
      bool pointedTo;        //if true, a pointer to object exists
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

      void *pobj;
      ObjectInfo obj;
      MemberInfo mbr;

      std::deque <ObjectInfo> objQueue;
      std::vector <ClsHeader> clsList;
      std::vector <PtrHeader> ptrList;
      SerializeManager *sm;
      bool simulate;

      void reset (UintSize startOffset, bool realRun);
      void store (void *from, UintSize to, UintSize size);
      void store (void *from, UintSize size);
      void load (void *to, UintSize size);

      void enqueueObjVar (ClassPtr cls, void *var);
      void enqueueObjPtr (ClassPtr cls, void *var);

      void run (ClassPtr rootCls, void **rootPtr);
      virtual bool processObject () { return true; }
      virtual void processDataVar (void *pmbr) {}
      virtual void processObjVar (void *pmbr, ObjectInfo &newObj) {}
      virtual void processObjPtr (void **pmbr, ObjectInfo &newObj) {}
      virtual void processObjArray (void **pmbr) {}
      virtual void processObjArrayItem (void **pmbr, ObjectInfo &newObj) {}
      virtual void processObjPtrArray (void ***pmbr) {}
      virtual void processObjPtrArrayItem (void **pmbr, ObjectInfo &newObj) {}
      virtual void processDataPtr (void **pmbr) {}
    };

    class StateSave : public State
    { public:
      //virtual void run (ClassPtr rootCls, void **rootPtr);
      virtual bool processObject ();
      virtual void processDataVar (void *pmbr);
      virtual void processDataPtr (void **pmbr);
    };

    class StateLoad : public State
    { public:
      //virtual void run (ClassPtr rootCls, void **rootPtr);
      virtual bool processObject ();
      virtual void processDataVar (void *pmbr);
      virtual void processObjArray (void **pmbr);
      virtual void processObjPtrArray (void ***pmbr);
      virtual void processDataPtr (void **pmbr);
    };

    class StateSerial : public State
    {
    private:
      void adjust (UintSize ptrOffset);
      
    public:
      //virtual void run (ClassPtr rootCls, void **rootPtr);
      virtual bool processObject ();
      virtual void processObjVar (void *pmbr, ObjectInfo &newObj);
      virtual void processObjPtr (void **pmbr, ObjectInfo &newObj);
      virtual void processObjArrayItem (void **pmbr, ObjectInfo &newObj);
      virtual void processObjPtrArrayItem (void **pmbr, ObjectInfo &newObj);
      virtual void processDataPtr (void **pmbr);
    };
    
    //States
    StateSerial stateSerial;
    StateSave stateSave;
    StateLoad stateLoad;
    State *state;

    //Statistics
    std::vector <ObjectPtr> allObjects;

  public:

    bool isSerializing ();
    bool isDeserializing ();
    bool isSaving ();
    bool isLoading ();
    
    void serialize (ClassPtr cls, void *root, void **outData, UintSize *outSize);
    void save (ClassPtr cls, void *root, void **outData, UintSize *outSize);

    template <class TR> void serialize (TR *root, void **outData, UintSize *outSize)
      { serialize (Class(TR), root, outData, outSize); }

    template <class TR> void save (TR *root, void **outData, UintSize *outSize)
      { save (Class(TR), root, outData, outSize); }
    
    void* deserialize (const void *data, ClassPtr *outCls=NULL);
    void* load (const void *data, ClassPtr *outCls=NULL);

    const void* getSignature ();
    UintSize getSignatureSize ();
    bool checkSignature (const void *data);

    typedef std::vector <ObjectPtr> ObjectList;
    const ObjectList & getObjects () { return allObjects; }
  };
  
  typedef SerializeManager SM;

}//namespace GE
#endif//__GESERIALIZE_H
