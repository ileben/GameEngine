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

  void SerializeManager::State::reset (UintSize startOffset, bool realRun)
  {
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

  void SerializeManager::State::enqueueObjVar
    (ClassPtr cls, void *var, UintSize offset)
  {
    ObjectNode o;
    o.cls = cls;
    o.var = var;
    o.offset = offset;
    o.ptroffset = 0;
    o.pointedTo = false;
    objQueue.push_back( o );
  }

  void SerializeManager::State::enqueueObjPtr
    (ClassPtr cls, void *var, UintSize ptroffset)
  {
    ObjectNode o;
    o.cls = cls;
    o.var = var;
    o.offset = 0;
    o.ptroffset = ptroffset;
    o.pointedTo = true;
    objQueue.push_back( o );
  }

  /*
  ---------------------------------------------------------
  State-specific serialization routine
  ---------------------------------------------------------*/

  void SerializeManager::StateSave::run (ClassPtr rcls, void **rptr)
  {
    //Push root info to queue
    ObjectNode robj;
    robj.cls = rcls;
    robj.var = rptr;
    robj.offset = 0;
    robj.ptroffset = 0;
    robj.pointedTo = true;
    objQueue.push_back( robj );

    //Process objects in queue
    while (!objQueue.empty())
    {
      //Pop first object info off the stack
      ObjectNode obj = objQueue.front();
      objQueue.pop_front();

      //Get pointer to object
      void *pobj = obj.pointedTo ? *((void**)obj.var) : obj.var;

      //Walk object members
      const MTable *members = obj.cls->getMembers();
      for (UintSize m=0; m<members->size(); ++m)
      {
        //Get member info
        MemberInfo mbr = members->at(m)->getInfo( pobj );
        switch (mbr.type)
        {
        case MemberType::DataVar:
        {
          //Store data from the variable
          void *pmbr = members->at(m)->getFrom( pobj );
          store( pmbr, mbr.size );
          break;

        }
        case MemberType::ObjVar:
        {
          //Enqueue object
          void *pmbr = members->at(m)->getFrom( pobj );
          enqueueObjVar( mbr.cls, pmbr );
          break;
        }
        case MemberType::ObjPtr:
        {
          //Enqueue pointer to object
          void **pmbr = (void**) members->at(m)->getFrom( pobj );
          enqueueObjPtr( mbr.cls, pmbr );
          break;
        }
        case MemberType::ObjArray:
        {
          if (mbr.size == 0) break;

          //Walk the array of objects
          void *p = *((void**) members->at(m)->getFrom( pobj ));
          for (UintSize o=0; o<mbr.size; ++o)
          {
            //Enqueue object
            enqueueObjVar( mbr.cls, p );

            //Next object in the array
            Util::PtrAdd( &p, mbr.cls->getSize() );
          }
          break;
        }
        case MemberType::ObjPtrArray:
        {
          if (mbr.size == 0) break;

          //Walk the array of pointers to objects
          void **p = *((void***) members->at(m)->getFrom( pobj ));
          for (UintSize o=0; o<mbr.size; ++o)
          {
            //Enqueue pointer to object
            enqueueObjPtr( mbr.cls, p );

            //Next pointer in the array
            Util::PtrAdd( &p, sizeof(void*) );
          }
          break;
        }
        case MemberType::DataPtr:
        {
          if (mbr.size == 0) break;

          //Store data from the buffer
          void **pmbr = (void**) members->at(m)->getFrom( pobj );
          store( *pmbr, mbr.size );
          break;
        }}
      }
    }
  }

  void SerializeManager::StateLoad::run (ClassPtr rcls, void **rptr)
  {
    //Push root info to queue
    ObjectNode robj;
    robj.cls = rcls;
    robj.var = rptr;
    robj.offset = 0;
    robj.ptroffset = 0;
    robj.pointedTo = true;
    objQueue.push_back( robj );

    //Process objects in queue
    while (!objQueue.empty())
    {
      //Pop first object info off the stack
      ObjectNode obj = objQueue.front();
      objQueue.pop_front();
      void *pobj;

      //Is there a pointer to it?
      if (obj.pointedTo)
      {
        //Construct new object instance and adjust pointer to it
        void **p = (void**)obj.var;
        *p = obj.cls->newInstance();
        pobj = *p;
      }
      else
      {
        //Construct new object in-place
        obj.cls->newInPlace( obj.var );
        pobj = obj.var;
      }

      //Walk object members
      const MTable *members = obj.cls->getMembers();
      for (UintSize m=0; m<members->size(); ++m)
      {
        //Get member info
        MemberInfo mbr = members->at(m)->getInfo( pobj );
        switch (mbr.type)
        {
        case MemberType::DataVar:
        {
          //Load data into variable
          void *pmbr = members->at(m)->getFrom( pobj );
          load( pmbr, mbr.size );
          break;
        }
        case MemberType::ObjVar:
        {
          //Enqueue object
          void *pmbr = members->at(m)->getFrom( pobj );
          enqueueObjVar( mbr.cls, pmbr );
          break;
        }
        case MemberType::ObjPtr:
        {
          //Enqueue pointer to object
          void **pmbr = (void**) members->at(m)->getFrom( pobj );
          enqueueObjPtr( mbr.cls, pmbr );
          break;
        }
        case MemberType::ObjArray:
        {
          if (mbr.size == 0) break;

          //Allocate an array of objects
          void **pmbr = (void**) members->at(m)->getFrom( pobj );
          *pmbr = std::malloc( mbr.size * mbr.cls->getSize() );

          //Walk the array of objects
          void *p = *pmbr;
          for (UintSize o=0; o<mbr.size; ++o)
          {
            //Enqueue each object
            enqueueObjVar( mbr.cls, p );

            //Next object in the array
            Util::PtrAdd( &p, mbr.cls->getSize() );
          }
          break;
        }
        case MemberType::ObjPtrArray:
        {
          if (mbr.size == 0) break;

          //Allocate an array of pointers to objects
          void ***pmbr = (void***) members->at(m)->getFrom( pobj );
          *pmbr = (void**) std::malloc( mbr.size * sizeof(void*) );

          //Walk the array of pointers to objects
          void **p = *pmbr;
          for (UintSize o=0; o<mbr.size; ++o)
          {
            //Enqueue each pointer to object
            enqueueObjPtr( mbr.cls, p );

            //Next pointer in the array
            Util::PtrAdd( &p, sizeof(void*) );
          }
          break;
        }
        case MemberType::DataPtr:
        {
          if (mbr.size == 0) break;

          //Allocate a buffer for data
          void **pmbr = (void**) members->at(m)->getFrom( pobj );
          *pmbr = std::malloc( mbr.size );

          //Load data into buffer
          load( *pmbr, mbr.size );
          break;
        }}
      }
    }
  }

  void SerializeManager::StateSerial::run (ClassPtr rcls, void **rptr)
  {
    //Push root info to queue
    ObjectNode robj;
    robj.cls = rcls;
    robj.var = rptr;
    robj.offset = 0;
    robj.ptroffset = 0;
    robj.pointedTo = true;
    objQueue.push_back( robj );

    //Process objects in queue
    while (!objQueue.empty())
    {
      //Pop first object info off the stack
      ObjectNode obj = objQueue.front();
      objQueue.pop_front();
      
      //Get pointer to object
      void *pobj = obj.pointedTo ? *((void**)obj.var) : obj.var;

      //Is there a pointer to it?
      if (obj.pointedTo)
      {
        //Set object offset and adjust pointer to it
        obj.offset = offset;
        adjust( obj.ptroffset );

        //Store object data
        store( pobj, obj.cls->getSize() );
      }
      else
      {
        //Store object data at its offset
        store( pobj, obj.offset, obj.cls->getSize() );
      }

      //TODO: store ClassHeader


      //Walk object members
      const MTable *members = obj.cls->getMembers();
      for (UintSize m=0; m<members->size(); ++m)
      {
        //Get member info
        MemberInfo mbr = members->at(m)->getInfo( pobj );
        switch (mbr.type)
        {
        case MemberType::DataVar:
        {
          //Data stored along with the object
          break;
        }
        case MemberType::ObjVar:
        {
          //Enqueue object
          void *pmbr = members->at(m)->getFrom( pobj );
          UintSize offset = obj.offset + Util::PtrDist( pobj, pmbr );
          enqueueObjVar( mbr.cls, pmbr, offset );
          break;
        }
        case MemberType::ObjPtr:
        {
          //Enqueue pointer to object
          void **pmbr = (void**) members->at(m)->getFrom( pobj );
          UintSize ptroffset = obj.offset + Util::PtrDist( pobj, pmbr );
          enqueueObjPtr( mbr.cls, pmbr, ptroffset );
          break;
        }
        case MemberType::ObjArray:
        {
          if (mbr.size == 0) break;

          //Walk the array of objects
          void *p = *((void**) members->at(m)->getFrom( pobj ));
          for (UintSize o=0; o<mbr.size; ++o)
          {
            //Reserve space for serialized object
            offset += mbr.cls->getSize();

            //Enqueue object with offset
            enqueueObjVar( mbr.cls, p, offset  );

            //Next object in the array
            Util::PtrAdd( &p, mbr.cls->getSize() );
          }
          break;
        }
        case MemberType::ObjPtrArray:
        {
          if (mbr.size == 0) break;

          //Walk the array of pointers to objects
          void **p = *((void***) members->at(m)->getFrom( pobj ));
          for (UintSize o=0; o<mbr.size; ++o)
          {
            //Reserve space for serialized pointer
            offset += sizeof(void*);

            //Enqueue pointer to object with offset
            enqueueObjPtr( mbr.cls, p, offset );

            //Next pointer in the array
            Util::PtrAdd( &p, sizeof(void*) );
          }
          break;
        }
        case MemberType::DataPtr:
        {
          if (mbr.size == 0) break;

          //Adjust pointer to the buffer
          void **pmbr = (void**) members->at(m)->getFrom( pobj );
          adjust( obj.offset + Util::PtrDist( pobj, pmbr ) );

          //Store data from the buffer
          store( *pmbr, mbr.size );
          break;
        }}
      }
    }
  }
  
  /*
  --------------------------------------------------
  Serialization state
  --------------------------------------------------*/
 
  /*
  void SerializeManager::StateSerial::run
    (ClassPtr rcls, void **rptr)
  {
    //Push root info to queue
    rootPtr( rcls, rptr );
    while (!resQueue.empty())
    {
      //Pop first resource info off the stack
      ResPtrInfo ri = resQueue.front();
      resQueue.pop_front();
      current = &ri;

      if (ri.isptrarray)
      {
        //An array of pointers to objects
        void **pptr = *((void***)ri.ptr);

        //Adjust the serialized pointer to the array
        if (!ri.isroot) adjust( ri.ptroffset );
        
        //Walk the array of pointers to objects
        for (UintSize c=0; c<ri.count; ++c)
        {
          //Enqueue each pointer to object
          arrayMemberPtr( ri.cls, pptr, offset );
          
          //Leave space for serialized pointer to object
          offset += sizeof( void* );

          //Next pointer in the array
          Util::PtrAdd( &pptr, sizeof(void*) );
        }
      }
      else if (ri.isptr)
      {
        //A pointer to an object (array)
        void *ptr = *((void**)ri.ptr);

        //Adjust the serialized pointer to object (array)
        if (!ri.isroot) adjust( ri.ptroffset );

        //Add to list of classes to deserialize
        ClsHeader ch;
        ch.id = ri.cls->getID();
        ch.count = ri.count;
        ch.offset = offset;
        clsList.push_back (ch);

        //Walk the array of objects
        for (UintSize c=0; c<ri.count; ++c)
        {
          //Update serialized offset
          ri.offset = offset;
          
          //Copy object data (this will increase current offset)
          store( ptr, ri.cls->getSize() );
          
          //Get new members to serialize from the object
          ri.cls->invokeCallback( ClassEvent::Serialize, ptr, sm );

          //Next object in the array
          Util::PtrAdd( &ptr, ri.cls->getSize() );
        }
      }
      else
      {
        //An automatic object variable
        void *ptr = ri.ptr;

        //Get new members to serialize from the object
        ri.cls->invokeCallback( ClassEvent::Serialize, ptr, sm );
      }
      
      while (!dynQueue.empty())
      {
        //Pop first dynamic info off the stack
        DynPtrInfo di = dynQueue.front();
        dynQueue.pop_front();

        //A pointer to dynamically allocated memory
        void *ptr = *((void**)di.ptr);

        //Adjust the serialized pointer to dynamic data
        adjust( di.ptroffset );
        
        //Copy dynamic data
        store( ptr, di.size );
      }
    }
  }
*/
  /*
  --------------------------------------------------
  Saving state
  --------------------------------------------------*/
/*
  void SerializeManager::StateSave::run( ClassPtr rcls, void **rptr )
  {
    //Push root info to queue
    rootPtr( rcls, rptr );
    while (!resQueue.empty())
    {
      //Pop first resource info off the stack
      ResPtrInfo ri = resQueue.front();
      resQueue.pop_front();
      current = &ri;
      
      if (ri.isptrarray)
      {
        //An array of pointers to objects
        void **pptr = *((void***)ri.ptr);

        //Walk the array of pointers to resources
        for (UintSize c=0; c<ri.count; ++c)
        {
          //Enqueue each pointer to object
          arrayMemberPtr( ri.cls, pptr, 0 );

          //Next pointer in the array
          Util::PtrAdd( &pptr, sizeof(void*) );
        }
      }
      else if (ri.isptr)
      {
        //A pointer to an object (array)
        void *ptr = *((void**)ri.ptr);

        //Walk the array of objects
        for (UintSize c=0; c<ri.count; ++c)
        {
          //Get new members to serialize from the object
          ri.cls->invokeCallback( ClassEvent::Serialize, ptr, sm );

          //Next object in the array
          Util::PtrAdd( &ptr, ri.cls->getSize() );
        }
      }
      else
      {
        //An automatic object variable
        void *ptr = ri.ptr;

        //Get new members to serialize from the object
        ri.cls->invokeCallback( ClassEvent::Serialize, ptr, sm );
      }
      
      while (!dynQueue.empty())
      {
        //Pop first dynamic info off the stack
        DynPtrInfo di = dynQueue.front();
        dynQueue.pop_front();

        //A pointer to dynamically allocated memory
        void *ptr = *((void**)di.ptr);
        
        //Copy dynamic data
        store( ptr, di.size );
      }
    }
  }
  */
  /*
  --------------------------------------------------
  Loading state
  --------------------------------------------------*/
  /*
  void SerializeManager::StateLoad::run( ClassPtr rcls, void **rptr )
  {
    //Push root info to queue
    rootPtr( rcls, rptr );
    while( !resQueue.empty() )
    {
      //Pop first resource info off the stack
      ResPtrInfo ri = resQueue.front();
      resQueue.pop_front();
      current = &ri;

      if (ri.isptrarray)
      {
        //Allocate memory for the array of pointers
        void **pptr = (void**) std::malloc( ri.count * sizeof(void*) );

        //Adjust the pointer to the array
        *((void***)ri.ptr) = pptr;

        //Walk the array of pointers to resources
        for( UintSize p=0; p<ri.count; ++p )
        {
          //Enqueue the pointer to resource
          arrayMemberPtr( ri.cls, pptr, 0 );

          //Next pointer in the array
          Util::PtrAdd( &pptr, sizeof(void*) );
        }
      }
      else if (ri.isptr && ri.isarray)
      {
        //Allocate memory for the array of objects
        void *ptr = std::malloc( ri.count * ri.cls->getSize() );

        //Adjust the pointer to the array
        *((void**)ri.ptr) = ptr;

        //Walk the array of objects
        for (UintSize c=0; c<ri.count; ++c)
        {
          //Initialize the object in-place
          ri.cls->newSerialInPlace( ptr, sm );

          //Get new members to serialize from the object
          ri.cls->invokeCallback( ClassEvent::Serialize, ptr, sm );

          //Next object in the array
          Util::PtrAdd( &ptr, ri.cls->getSize() );
        }
      }
      else if (ri.isptr)
      {
        //Allocate a new object instance
        void *ptr = ri.cls->newSerialInstance( sm );

        //Adjust the pointer to object
        *((void**)ri.ptr) = ptr;

        //Get new members to serialize from the object
        ri.cls->invokeCallback( ClassEvent::Serialize, ptr, sm );
      }
      else
      {
        //An automatic object variable
        void *ptr = ri.ptr;

        //Get new members to serialize from the object
        ri.cls->invokeCallback( ClassEvent::Serialize, ptr, sm );
      }
      
      while( !dynQueue.empty() )
      {
        //Pop first dynamic info off the stack
        DynPtrInfo di = dynQueue.front();
        dynQueue.pop_front();

        //Allocate memory buffer dynamically
        void *ptr = std::malloc( di.size );

        //Adjust the pointer to buffer
        *((void**)di.ptr) = ptr;
        
        //Copy dynamic data
        load( ptr, di.size );
      }
    }
  }
*/
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
