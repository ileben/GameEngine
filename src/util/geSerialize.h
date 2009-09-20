#ifndef __GESERIALIZE_H
#define __GESERIALIZE_H

namespace GE
{

  class GE_API_ENTRY SerializeManager
  {
  private:

    struct ObjectInfo
    {
      Uint id;               //unique ID
      Object *ptr;           //pointer to object
      //ClassPtr cls;          //class of object
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

      ObjectInfo obj;
      MemberInfo mbr;

      std::deque <ObjectInfo> objQueue;
      std::vector <ClsHeader> clsList;
      std::vector <PtrHeader> ptrList;
      std::vector <Object**> refList;
      SerializeManager *sm;
      bool simulate;

      void reset (UintSize startOffset, bool realRun);
      void store (void *from, UintSize to, UintSize size);
      void store (void *from, UintSize size);
      void load (void *to, UintSize size);

      void enqueueObjVar (Object *ptr);
      void enqueueObjPtr (Object *ptr);

      void run (Object **rootPtr);
      virtual bool processObject () { return true; }
      virtual void processDataVar (void *pmbr) {}
      virtual void processObjVar (Object *pmbr, ObjectInfo &newObj) {}
      virtual void processObjPtr (Object **pmbr, ObjectInfo &newObj) {}
      virtual void processObjRef (Object **pmbr) {}
      virtual void processObjArray (Object **pmbr) {}
      virtual void processObjArrayItem (Object *pmbr, ObjectInfo &newObj) {}
      virtual void processObjPtrArray (Object ***pmbr) {}
      virtual void processObjPtrArrayItem (Object **pmbr, ObjectInfo &newObj) {}
      virtual void processDataPtr (void **pmbr) {}
    };

    class StateSave : public State
    { public:
      virtual bool processObject ();
      virtual void processDataVar (void *pmbr);
      virtual void processDataPtr (void **pmbr);
      virtual void processObjRef (Object **pmbr);
    };

    class StateLoad : public State
    { public:
      virtual bool processObject ();
      virtual void processDataVar (void *pmbr);
      virtual void processDataPtr (void **pmbr);
      virtual void processObjRef (Object **pmbr);
      virtual void processObjArray (Object **pmbr);
      virtual void processObjPtrArray (Object ***pmbr);
    };

    class StateSerial : public State
    {
    private:
      void adjust (UintSize ptrOffset);
      
    public:
      virtual bool processObject ();
      virtual void processObjVar (Object *pmbr, ObjectInfo &newObj);
      virtual void processObjPtr (Object **pmbr, ObjectInfo &newObj);
      virtual void processObjArrayItem (Object *pmbr, ObjectInfo &newObj);
      virtual void processObjPtrArrayItem (Object **pmbr, ObjectInfo &newObj);
      virtual void processDataPtr (void **pmbr);
    };
    
    //States
    StateSerial stateSerial;
    StateSave stateSave;
    StateLoad stateLoad;
    State *state;

    //Statistics
    std::vector <Object*> allObjects;

  public:

    bool isSerializing ();
    bool isDeserializing ();
    bool isSaving ();
    bool isLoading ();
    
    void serialize (Object *root, void **outData, UintSize *outSize);
    void save (Object *root, void **outData, UintSize *outSize);

    Object* deserialize (const void *data, ClassPtr *outCls=NULL);
    Object* load (const void *data, ClassPtr *outCls=NULL);

    const void* getSignature ();
    UintSize getSignatureSize ();
    bool checkSignature (const void *data);

    typedef std::vector <Object*> ObjectList;
    const ObjectList & getObjects () { return allObjects; }
  };
  
  typedef SerializeManager SM;

}//namespace GE
#endif//__GESERIALIZE_H
