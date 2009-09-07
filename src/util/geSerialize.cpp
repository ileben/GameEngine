#include "util/geUtil.h"

namespace GE
{
  /*
  ----------------------------------------------------------
  These functions are to be called from within the
  CLSEVT_SERIALIZE callback function of a resource.
  They mark pointers to other resources within the current
  one to recurse into, or a dynamically allocated block of
  data to be copied in.
  
  The byte offset to the serialized conterpart of the
  pointers are calculated from the serialized offset of
  the currently processed resource and the offset of the
  pointer member variable within its structure.

  The serialized version of the pointer is adjusted to
  point to the serialized version of the object within the
  serialized data. At deserialization the pointer is again
  adjusted to point within the loaded memory block.

  As opposed to serialize/deserialize, the save/load
  process never copies entire structures or any pointers
  into the serialized data block. Instead the object's
  serialization callback is called both at save and load
  time to define the classes to be constructed and data
  to be copied.

  One exception which cannot be directly serialized is
  a pointer to an array of resources. This is a problem
  only when save-ing / load-ing, because the array
  version of the new operator always invokes the default
  constructor so it's not possible to call new[] with
  a SerializeManager* argument on every member of the array.
  
  A workaround is to use multiple in-place constructions,
  that's why the objectArray() serialize instruction is
  only valid on arrays where members are to be constructed
  in-place. ObjArrayList class is an example of such usage.
  ---------------------------------------------------------*/
  
  bool SerializeManager::isSerializing () {
    return state == &stateSerial;
  }

  bool SerializeManager::isDeserializing () {
    return state == NULL;
  }

  bool SerializeManager::isSaving () {
    return state == &stateSave;
  }
  
  bool SerializeManager::isLoading () {
    return state == &stateLoad;
  }

  /*
  ---------------------------------------------------------
  State utilities
  ---------------------------------------------------------*/

  void SerializeManager::State::enqueueObjVar (ClassPtr cls, void *var)
  {
    ObjectInfo o;
    o.cls = cls;
    o.var = var;
    o.offset = 0;
    o.ptroffset = 0;
    o.pointedTo = false;
    objQueue.push_back( o );
  }

  void SerializeManager::State::enqueueObjPtr (ClassPtr cls, void *var)
  {
    ObjectInfo o;
    o.cls = cls;
    o.var = var;
    o.offset = 0;
    o.ptroffset = 0;
    o.pointedTo = true;
    objQueue.push_back( o );
  }

  void SerializeManager::State::reset (UintSize startOffset, bool realRun)
  {
    sm->allObjects.clear();
    objQueue.clear ();
    ptrList.clear ();
    clsList.clear ();
    offset = startOffset;
    simulate = !realRun;
  }

  void SerializeManager::State::store (void *from, UintSize to, UintSize size)
  {
    if (!simulate)
    {
      //Copy [size] bytes of data to buffer at given offset
      std::memcpy( data + to, from, size );
    }
  }

  void SerializeManager::State::store (void *from, UintSize size)
  {
    if (!simulate)
    {
      //Copy [size] bytes of data to buffer at copy offset
      std::memcpy( data + offset, from, size );
    }
    
    //Advance copy offset
    offset += size;
  }

  void SerializeManager::State::load (void *to, UintSize size)
  {
    if (!simulate)
    {
      //Copy [size] bytes of data from buffer at copy offset
      std::memcpy( to, data + offset, size );
    }

    //Advance copy offset
    offset += size;
  }

  void SerializeManager::StateSerial::adjust (UintSize ptrOffset)
  {
    if (!simulate)
    {
      //Make serialized pointer at given offset
      //point to current store offset
      void *pptr = data + ptrOffset;
      Util::PtrSet (pptr, offset);
    }
    
    //Add to the list of adjusted pointers
    PtrHeader ph;
    ph.offset = ptrOffset;
    ptrList.push_back (ph);
  }

  /*
  --------------------------------------------------
  Saving state
  --------------------------------------------------*/

  bool SerializeManager::StateSave::processObject ()
  {
    if (obj.pointedTo)
    {
      //Check for NULL pointer
      void **pmbr = (void**)obj.var;
      if (*pmbr == NULL)
      {
        //Store 0 class ID
        ClassID clsid( 0,0,0,0 );
        store( &clsid, sizeof(ClassID) );
        return false;
      }

      //Find actual class ID
      obj.cls = obj.cls->getFinalClass( *pmbr );

      //Store class ID
      ClassID clsid = obj.cls->getID();
      store( &clsid, sizeof(ClassID) );
    }

    return true;
  }

  void SerializeManager::StateSave::processDataVar
    (void *pmbr)
  {
    //Store data from the variable
    store( pmbr, mbr.size );
  }

  void SerializeManager::StateSave::processDataPtr
    (void **pmbr)
  {
    //Store data from the buffer
    store( *pmbr, mbr.size );
  }

  /*
  --------------------------------------------------
  Loading state
  --------------------------------------------------*/

  bool SerializeManager::StateLoad::processObject ()
  {
    //Is there a pointer to it?
    if (obj.pointedTo)
    {
      //Load actual class ID
      ClassID clsid;
      load( &clsid, sizeof(ClassID) );
      obj.cls = IClass::FromID( clsid );

      //Check for NULL pointer
      void **pmbr = (void**)obj.var;
      if (obj.cls == NULL)
      {
        //Set pointer to NULL
        *pmbr = NULL;
        return false;
      }

      //Construct new object instance and adjust pointer to it
      *pmbr = obj.cls->newSerialInstance( sm );
    }
    else
    {
      //Construct new object in-place
      obj.cls->newSerialInPlace( obj.var, sm );
    }

    return true;
  }

  void SerializeManager::StateLoad::processDataVar
    (void *pmbr)
  {
    //Load data into variable
    load( pmbr, mbr.size );
    int *a = (int*)pmbr;
  }

  void SerializeManager::StateLoad::processObjArray
    (void **pmbr)
  {
    //Allocate an array of objects
    *pmbr = std::malloc( mbr.size * mbr.cls->getSize() );
  }

  void SerializeManager::StateLoad::processObjPtrArray
    (void ***pmbr)
  {
    //Allocate an array of pointers to objects
    *pmbr = (void**) std::malloc( mbr.size * sizeof(void*) );
  }

  void SerializeManager::StateLoad::processDataPtr
    (void **pmbr)
  {
    //Allocate a buffer for data
    *pmbr = std::malloc( mbr.size );
    
    //Load data into buffer
    load( *pmbr, mbr.size );
  }

  /*
  --------------------------------------------------
  Serialization state
  --------------------------------------------------*/

  bool SerializeManager::StateSerial::processObject ()
  {
    //Is there a pointer to it?
    if (obj.pointedTo)
    {
      //Check for NULL pointer
      void **pmbr = (void**)obj.var;
      if (*pmbr == NULL)
      {
        //Set serialized pointer to NULL
        void *pnull = NULL;
        store( &pnull, obj.ptroffset, sizeof(void*) );
        return false;
      }

      //Set object offset and adjust pointer to it
      obj.offset = offset;
      adjust( obj.ptroffset );

      //Find actual class ID
      obj.cls = obj.cls->getFinalClass( *pmbr );

      //Store object data
      store( *pmbr, obj.cls->getSize() );
    }
    else
    {
      //Store object data at its offset
      store( obj.var, obj.offset, obj.cls->getSize() );
    }

    //Store class header for deserialization
    ClsHeader clsHeader;
    clsHeader.id = obj.cls->getID();
    clsHeader.offset = obj.offset;
    clsHeader.count = 1;
    return true;
  }

  void SerializeManager::StateSerial::processObjVar
    (void *pmbr, ObjectInfo &newObj)
  {
    //Set offset to the new object
    newObj.offset = obj.offset + Util::PtrDist( pobj, pmbr );
  }

  void SerializeManager::StateSerial::processObjPtr
    (void **pmbr, ObjectInfo &newObj)
  {
    //Set offset to pointer to the new object
    newObj.ptroffset = obj.offset + Util::PtrDist( pobj, pmbr );
  }

  void SerializeManager::StateSerial::processObjArrayItem
    (void **pmbr, ObjectInfo &newObj)
  {
    //Set offset to the new object
    newObj.offset = offset;

    //Reserve space for serialized object
    offset += mbr.cls->getSize();
  }

  void SerializeManager::StateSerial::processObjPtrArrayItem
    (void **pmbr, ObjectInfo &newObj)
  {
    //Set offset to pointer to the new object
    newObj.ptroffset = offset;

    //Reserve space for serialized pointer
    offset += sizeof(void*);
  }

  void SerializeManager::StateSerial::processDataPtr
    (void **pmbr)
  {
    //Adjust pointer to the buffer
    adjust( obj.offset + Util::PtrDist( pobj, pmbr ) );
    
    //Store data from the buffer
    store( *pmbr, mbr.size );
  }

  /*
  ----------------------------------------------------
  Generic serialization routine
  ----------------------------------------------------*/

  void SerializeManager::State::run (ClassPtr rcls, void **rptr)
  {
    std::deque<MemberPtr> members;

    //Push root object on queue
    enqueueObjPtr( rcls, rptr );

    //Process objects in queue
    while (!objQueue.empty())
    {
      //Pop first object info off the queue
      obj = objQueue.front();
      objQueue.pop_front();

      //Process object
      if (!processObject()) continue;
      pobj = obj.pointedTo ? *((void**)obj.var) : obj.var;

      //Get all the object members
      members.clear();
      obj.cls->getAllMembers( members );

      //Walk object members
      while (!members.empty())
      {
        //Pop first member off the queue
        MemberPtr member = members.front();
        members.pop_front();

        //Get member info
        mbr = member->getInfo( pobj );
        switch (mbr.type)
        {
        case MemberType::DataVar:
        {
          //Process data variable
          void *pmbr = member->getFrom( pobj );
          processDataVar( pmbr );
          break;
        }
        case MemberType::ObjVar:
        {
          //Enqueue and process object
          void *pmbr = member->getFrom( pobj );
          enqueueObjVar( mbr.cls, pmbr );
          processObjVar( pmbr, objQueue.back() );
          break;
        }
        case MemberType::ObjPtr:
        {
          //Enqueue and process pointer to object
          void **pmbr = (void**) member->getFrom( pobj );
          enqueueObjPtr( mbr.cls, pmbr );
          processObjPtr( pmbr, objQueue.back() );
          break;
        }
        case MemberType::ObjArray:
        {
          if (mbr.size == 0) break;

          //Process array of objects
          void **pmbr = (void**) member->getFrom( pobj );
          processObjArray( pmbr );

          //Walk the array of objects
          void *p = *pmbr;
          for (UintSize o=0; o<mbr.size; ++o)
          {
            //Enqueue and process each object
            enqueueObjVar( mbr.cls, p );
            processObjVar( p, objQueue.back() );

            //Next object in the array
            Util::PtrAdd( &p, mbr.cls->getSize() );
          }
          break;
        }
        case MemberType::ObjPtrArray:
        {
          if (mbr.size == 0) break;

          //Process array of pointers to objects
          void ***pmbr = (void***) member->getFrom( pobj );
          processObjPtrArray( pmbr );

          //Walk the array of pointers to objects
          void **p = *pmbr;
          for (UintSize o=0; o<mbr.size; ++o)
          {
            //Enqueue and processs each pointer to object
            enqueueObjPtr( mbr.cls, p );
            processObjPtr( p, objQueue.back() );

            //Next pointer in the array
            Util::PtrAdd( &p, sizeof(void*) );
          }
          break;
        }
        case MemberType::DataPtr:
        {
          if (mbr.size == 0) break;

          //Process data buffer
          void **pmbr = (void**) member->getFrom( pobj );
          processDataPtr( pmbr );

          break;
        }}

      }//Process members

      //Notify class
      if (sm->isLoading())
        obj.cls->invokeCallback( ClassEvent::Loaded, pobj, sm );

      //Store for statistics
      sm->allObjects.push_back( ObjectPtr( obj.cls, pobj ) );

    }//Process objects
  }

  /*
  --------------------------------------------------
  Serialization start
  --------------------------------------------------*/
  
  void SerializeManager::serialize( ClassPtr cls, void *root,
                                    void **outData, UintSize *outSize )
  {
    //Enter serialization state
    state = &stateSerial;
    state->sm = this;
    
    //Simulation run
    state->reset( 0, false );
    state->run( cls, &root );
    
    //Calculate the size of pointer and class table
    UintSize introSize =
      (sizeof(UintSize)  +
       sizeof(PtrHeader) * state->ptrList.size() +
       sizeof(UintSize)  +
       sizeof(ClsHeader) * state->clsList.size());
    
    //Allocate data
    UintSize dataSize = introSize + state->offset;
    state->data = (Uint8*) std::malloc( dataSize );
    *outData = state->data;
    *outSize = dataSize;
    
    //Real run
    state->reset( introSize, true );
    state->run( cls, &root );
    
    //Back to start of data
    state->offset = 0;
    
    //Store pointer list
    UintSize numPointers = state->ptrList.size();
    state->store( &numPointers, sizeof(UintSize) );
    state->store( &state->ptrList[0],
                  state->ptrList.size() * sizeof(PtrHeader) );
    
    //Store class list
    UintSize numClasses = state->clsList.size();
    state->store( &numClasses, sizeof(UintSize) );
    state->store( &state->clsList[0],
                  state->clsList.size() * sizeof(ClsHeader) );
  }
  
  void* SerializeManager::deserialize( const void *data, ClassPtr *outClass )
  {
    state = NULL;
    void *root = NULL;
    ClassPtr rootCls;
    
    //Get the number of pointers
    UintSize *numPointers = (UintSize*) (data);
    PtrHeader *ptrList = (PtrHeader*) (numPointers + 1);
    
    for (UintSize p=0; p<*numPointers; ++p)
    {
      //Get the pointer to the pointer
      void *pptr = Util::PtrOff (data, ptrList[p].offset);
      
      //Add the base data address to it
      Util::PtrAdd (pptr, (UintSize)data);
    }
    
    //Get the number of resources
    UintSize *numClasses = (UintSize*) (ptrList + *numPointers);
    ClsHeader *clsList = (ClsHeader*) (numClasses + 1);
    
    //Get the list of class headers
    for (UintSize c=0; c<*numClasses; ++c)
    {
      //Get the pointer to the object and its class
      void *pres = Util::PtrOff (data, clsList[c].offset);
      ClassPtr cls = ClassFromID (clsList[c].id);

      //Store root
      if (c == 0) {
        root = pres;
        rootCls = cls;
      }

      //Construct object (array) in-place
      for (UintSize i=0; i<clsList[c].count; ++i)
      {
        cls->newSerialInPlace (pres, this);
        Util::PtrAdd (pres, cls->getSize());
      }
    }
    
    //Output
    if (outClass != NULL)
      *outClass = rootCls;

    return root;
  }

  /*
  --------------------------------------------------
  Saving start
  --------------------------------------------------*/

  void SerializeManager::save (ClassPtr cls, void *root, void **outData, UintSize *outSize)
  {
    //Enter saving state
    state = &stateSave;
    state->sm = this;
    
    //Simulation run
    state->reset( 0, false );
    state->run( cls, &root );
    
    //Allocate data
    UintSize dataSize = sizeof (ClassID) + state->offset;
    state->data = (Uint8*) std::malloc( dataSize );
    *outData = state->data;
    *outSize = dataSize;
    
    //Real run
    ClassID rootID = cls->getID();
    state->reset( 0, true );
    state->store( &rootID, sizeof(ClassID) );
    state->run( cls, &root );
  }

  /*
  --------------------------------------------------
  Loading start
  --------------------------------------------------*/
  
  void* SerializeManager::load (const void *data, ClassPtr *outClass)
  {
    //Enter loading state
    state = &stateLoad;
    state->sm = this;
    state->data = (Uint8*) data;

    //Load root class ID
    ClassID rootID;
    state->reset( 0, true );
    state->load( &rootID, sizeof(ClassID) );
    
    //Get root class and check if valid
    ClassPtr rootCls;
    rootCls = IClass::FromID( rootID );
    if (rootCls == NULL) return NULL;
    
    //Run
    void *rootPtr = NULL;
    state->run( rootCls, &rootPtr );

    //Output
    if (outClass != NULL)
      *outClass = rootCls;

    return rootPtr;
  }

  const void* SerializeManager::getSignature ()
  {
    static const ClassID sigID = CLSID_SIGNATURE;
    return &sigID;
  }

  UintSize SerializeManager::getSignatureSize ()
  {
    //Signature is a ClassID
    return sizeof( ClassID );
  }

  bool SerializeManager::checkSignature (const void *data)
  {
    //Compare data to signature
    ClassID *sigID = (ClassID*) data;
    return (*sigID == CLSID_SIGNATURE);
  }

}//namespace GE
