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

  void SerializeManager::State::enqueueObjVar (Object *ptr)
  {
    ObjectInfo o;
    o.ptr = ptr;
    o.offset = 0;
    o.ptroffset = 0;
    o.pointedTo = false;
    objQueue.push_back( o );
  }

  void SerializeManager::State::enqueueObjPtr (Object *ptr)
  {
    ObjectInfo o;
    o.ptr = ptr;
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
    refList.clear ();
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
    //Check for NULL pointer
    if (obj.ptr == NULL)
    {
      //Store invalid class ID
      ClassID clsid(0,0,0,0);
      store( &clsid, sizeof(ClassID) );
      return false;
    }
    else
    {
      //Store class ID
      ClassPtr cls = obj.ptr->GetInstanceClassPtr();
      ClassID clsid = cls->getID();
      store( &clsid, sizeof(ClassID) );
      return true;
    }
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

  void SerializeManager::StateSave::processObjRef (Object **pmbr)
  {
    //Check for NULL pointer
    if (*pmbr == NULL)
    {
      //Store invalid object ID
      UintSize id = 0;
      store( &id, sizeof(UintSize) );
    }
    else
    {
      //Store object ID
      UintSize id = (*pmbr)->serialID;
      store( &id, sizeof(UintSize) );
    }
  }

  /*
  --------------------------------------------------
  Loading state
  --------------------------------------------------*/

  bool SerializeManager::StateLoad::processObject ()
  {
    //Load class ID
    ClassID clsid;
    load( &clsid, sizeof(ClassID) );
    ClassPtr cls = IClass::FromID( clsid );
    if (cls == NULL) return false;
    
    //Construct new object
    if (obj.pointedTo)
      obj.ptr = (Object*) cls->newSerialInstance( sm );
    else cls->newSerialInPlace( obj.ptr, sm );
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
    (Object **pmbr)
  {
    //Allocate an array of objects
    *pmbr = (Object*) std::malloc( mbr.size * mbr.cls->getSize() );
  }

  void SerializeManager::StateLoad::processObjPtrArray
    (Object ***pmbr)
  {
    //Allocate an array of pointers to objects
    *pmbr = (Object**) std::malloc( mbr.size * sizeof(Object*) );
  }

  void SerializeManager::StateLoad::processDataPtr
    (void **pmbr)
  {
    //Allocate a buffer for data
    *pmbr = std::malloc( mbr.size );
    
    //Load data into buffer
    load( *pmbr, mbr.size );
  }

  void SerializeManager::StateLoad::processObjRef (Object **pmbr)
  {
    //Load object ID
    UintSize id = 0;
    load( &id, sizeof(UintSize) );

    //Check for NULL pointer
    if (id == 0) *pmbr = NULL;
    else if (id-1 >= sm->allObjects.size()) *pmbr = NULL;
    else *pmbr = sm->allObjects[ id-1 ];
  }

  /*
  --------------------------------------------------
  Serialization state
  --------------------------------------------------*/

  bool SerializeManager::StateSerial::processObject ()
  {
    //Check for NULL pointer
    if (obj.ptr == NULL)
      return false;

    //Store class header
    ClassPtr cls = obj.ptr->GetInstanceClassPtr();
    ClsHeader clsHeader;
    clsHeader.id = cls->getID();
    clsHeader.offset = obj.offset;
    clsHeader.count = 1;

    //Is there a pointer to it?
    if (obj.pointedTo)
    {
      //Set object offset and adjust pointer to it
      obj.offset = offset;
      adjust( obj.ptroffset );

      //Store object data
      store( obj.ptr, cls->getSize() );
    }
    else
    {
      //Store object data at its offset
      store( obj.ptr, obj.offset, cls->getSize() );
    }

    return true;
  }

  void SerializeManager::StateSerial::processObjVar
    (Object *pmbr, ObjectInfo &newObj)
  {
    //Set offset to the new object
    newObj.offset = obj.offset + Util::PtrDist( obj.ptr, pmbr );
  }

  void SerializeManager::StateSerial::processObjPtr
    (Object **pmbr, ObjectInfo &newObj)
  {
    //Set offset to pointer to the new object
    newObj.ptroffset = obj.offset + Util::PtrDist( obj.ptr, pmbr );
  }

  void SerializeManager::StateSerial::processObjArrayItem
    (Object *pmbr, ObjectInfo &newObj)
  {
    //Set offset to the new object
    newObj.offset = offset;

    //Reserve space for serialized object
    offset += mbr.cls->getSize();
  }

  void SerializeManager::StateSerial::processObjPtrArrayItem
    (Object **pmbr, ObjectInfo &newObj)
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
    adjust( obj.offset + Util::PtrDist( obj.ptr, pmbr ) );
    
    //Store data from the buffer
    store( *pmbr, mbr.size );
  }

  /*
  ----------------------------------------------------
  Generic serialization routine
  ----------------------------------------------------*/

  void SerializeManager::State::run (Object **root)
  {
    std::deque<MemberPtr> members;

    //Push root object on queue
    enqueueObjPtr( *root );
    refList.push_back( root );

    //Process objects in queue
    while (!objQueue.empty())
    {
      //Pop first object info off the queue
      obj = objQueue.front();
      objQueue.pop_front();

      //Process object
      if (!processObject())
        continue;

      //Assign unique ID to object
      ClassPtr objcls = obj.ptr->GetInstanceClassPtr();
      obj.ptr->serialID = sm->allObjects.size() + 1;
      sm->allObjects.push_back( obj.ptr );

      //Get all the object members
      members.clear();
      objcls->getAllMembers( members );

      //Walk object members
      while (!members.empty())
      {
        //Pop first member off the queue
        MemberPtr member = members.front();
        members.pop_front();

        //Get member info
        mbr = member->getInfo( obj.ptr );
        switch (mbr.type)
        {
        case MemberType::DataVar:
        {
          //Process data variable
          void *pmbr = member->getFrom( obj.ptr );
          processDataVar( pmbr );
          break;
        }
        case MemberType::ObjVar:
        {
          //Enqueue and process object
          Object *pmbr = (Object*) member->getFrom( obj.ptr );
          enqueueObjVar( pmbr );
          processObjVar( pmbr, objQueue.back() );
          break;
        }
        case MemberType::ObjPtr:
        {
          //Store the reference
          Object **pmbr = (Object**) member->getFrom( obj.ptr );
          refList.push_back( pmbr );

          //Enqueue and process pointer to object
          enqueueObjPtr( *pmbr );
          processObjPtr( pmbr, objQueue.back() );
          break;
        }
        case MemberType::ObjRef:
        {
          //Store the reference
          Object **pmbr = (Object**) member->getFrom( obj.ptr );
          refList.push_back( pmbr );
          break;
        }
        case MemberType::ObjArray:
        {
          if (mbr.size == 0) break;

          //Process array of objects
          Object **pmbr = (Object**) member->getFrom( obj.ptr );
          processObjArray( pmbr );

          //Walk the array of objects
          Object *p = *pmbr;
          for (UintSize o=0; o<mbr.size; ++o)
          {
            //Enqueue and process each object
            enqueueObjVar( p );
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
          Object ***pmbr = (Object***) member->getFrom( obj.ptr );
          processObjPtrArray( pmbr );

          //Walk the array of pointers to objects
          Object **p = *pmbr;
          for (UintSize o=0; o<mbr.size; ++o)
          {
            //Store the reference
            refList.push_back( p );

            //Enqueue and processs each pointer to object
            enqueueObjPtr( *p );
            processObjPtr( p, objQueue.back() );

            //Next pointer in the array
            Util::PtrAdd( &p, sizeof(Object*) );
          }
          break;
        }
        case MemberType::DataPtr:
        {
          if (mbr.size == 0) break;

          //Process data buffer
          void **pmbr = (void**) member->getFrom( obj.ptr );
          processDataPtr( pmbr );

          break;
        }}
      }

      //Notify class
      if (sm->isLoading())
        objcls->invokeCallback( ClassEvent::Loaded, obj.ptr, sm );
    }

    //Process references
    for (UintSize r=0; r<refList.size(); ++r)
      processObjRef( refList[r] );
  }

  /*
  --------------------------------------------------
  Saving start
  --------------------------------------------------*/

  void SerializeManager::save (Object *root, void **outData, UintSize *outSize)
  {
    //Enter saving state
    state = &stateSave;
    state->sm = this;
    
    //Simulation run
    state->reset( 0, false );
    state->run( &root );
    
    //Allocate data
    UintSize dataSize = sizeof (ClassID) + state->offset;
    state->data = (Uint8*) std::malloc( dataSize );
    *outData = state->data;
    *outSize = dataSize;
    
    //Real run
    ClassID rootID = root->GetInstanceClassPtr()->getID();
    state->reset( 0, true );
    state->store( &rootID, sizeof(ClassID) );
    state->run( &root );
  }

  /*
  --------------------------------------------------
  Loading start
  --------------------------------------------------*/
  
  Object* SerializeManager::load (const void *data, ClassPtr *outClass)
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
    Object *rootPtr = NULL;
    state->run( &rootPtr );

    //Output
    if (outClass != NULL)
      *outClass = rootCls;

    return rootPtr;
  }

  /*
  --------------------------------------------------
  Serialization start
  --------------------------------------------------*/
  
  void SerializeManager::serialize( Object *root, void **outData, UintSize *outSize )
  {
    //Enter serialization state
    state = &stateSerial;
    state->sm = this;
    
    //Simulation run
    state->reset( 0, false );
    state->run( &root );
    
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
    state->run( &root );
    
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
  
  Object* SerializeManager::deserialize( const void *data, ClassPtr *outClass )
  {
    state = NULL;
    void *root = NULL;
    ClassPtr rootCls = NULL;
    
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

    return (Object*) root;
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
