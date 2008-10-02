#define GE_API_EXPORT
#include "geEngine.h"

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
  pointers or are calculated from the serialized offset
  of the currently processed resource and the offset of the
  pointer member variable within its structure.

  There are some exceptions which cannot be serialized
  though:

  - Automatic resource member variable. Even if we did
    recurse into it at serialization stage, we don't have
    any control over its construction when unserializing.
    Only the default constructor will be called so the
    resource itself will have no idea whether it's been
    constructed on the heap or in-place.

  - Pointer to an array of resources. This is invalid
    only when save-ing / load-ing, since the array
    version of the new operator always invokes the default
    constructor. When serializing however, we could use
    multiple in-place constructions to work around it.
    To avoid the differences in usage though, this
    variant is not supported in any state. An array of
    pointers to resources is much more common anyway.

  ---------------------------------------------------------*/
  
  void SerializeManager::memberData
    (void *ptr, UintP size)
  {
    state->memberData (ptr, size);
  }

  void SerializeManager::resourcePtr
    (ClassPtr cls, void **pptr)
  {
    state->resourcePtr (cls, pptr);
  }

  void SerializeManager::resourcePtrPtr
    (ClassPtr cls, void ***pptr, UintP count)
  {
    state->resourcePtrPtr (cls, pptr, count);
  }
  
  void SerializeManager::dynamicPtr
    (void **pptr, UintP size)
  {
    state->dynamicPtr (pptr, size);
  }

  /*
  ---------------------------------------------------------
  State utilities
  ---------------------------------------------------------*/

  void SerializeManager::State::reset (UintP startOffset, bool realRun)
  {
    resQueue.clear ();
    dynQueue.clear ();
    ptrList.clear ();
    clsList.clear ();
    offset = startOffset;
    simulate = !realRun;
  }

  void SerializeManager::State::store (void *ptr, UintP size)
  {
    if (!simulate)
    {
      //Copy [size] bytes of data to buffer at copy offset
      std::memcpy (data + offset, ptr, size);
    }
    
    //Advance copy offset
    offset += size;
  }

  void SerializeManager::State::load (void *ptr, UintP size)
  {
    if (!simulate)
    {
      //Copy [size] bytes of data from buffer at copy offset
      std::memcpy (ptr, data + offset, size);
    }

    //Advance copy offset
    offset += size;
  }

  void SerializeManager::State::rootPtr (ClassPtr cls, void *ptr)
  {
    ResPtrInfo riroot;
    riroot.cls = cls;
    riroot.ptr = ptr;
    riroot.count = 1;
    riroot.offset = 0;
    riroot.isptrptr = false;
    riroot.detached = true;
    riroot.ptroffset = 0;
    resQueue.pushBack (riroot);
  }

  /*
  -----------------------------------------------------
  Serialization state
  -----------------------------------------------------*/

  void SerializeManager::StateSerial::memberData
    (void *ptr, UintP size)
  {
    //No-op
  }

  void SerializeManager::StateSerial::resourcePtr
    (ClassPtr cls, void **pptr)
  {
    ResPtrInfo ri;
    ri.count = 1;
    ri.offset = 0;
    ri.cls = cls;
    ri.ptr = *pptr;
    ri.isptrptr = false;
    ri.detached = false;
    ri.ptroffset = current->offset + Util::PtrDist (current->ptr, pptr);
    resQueue.pushBack (ri);
  }

  void SerializeManager::StateSerial::resourcePtrPtr
    (ClassPtr cls, void ***pptr, UintP count)
  {
    ResPtrInfo ri;
    ri.count = count;
    ri.offset = 0;
    ri.cls = cls;
    ri.ptr = *pptr;
    ri.isptrptr = true;
    ri.detached = false;
    ri.ptroffset = current->offset + Util::PtrDist (current->ptr, pptr);
    resQueue.pushBack (ri);
  }
  
  void SerializeManager::StateSerial::dynamicPtr
    (void **pptr, UintP size)
  {
    DynPtrInfo di;
    di.ptr = *(pptr);
    di.size = size;
    di.ptroffset = current->offset + Util::PtrDist (current->ptr, pptr);
    dynQueue.pushBack (di);
  }

 
  /*
  -----------------------------------------------------
  Saving state
  -----------------------------------------------------*/

  void SerializeManager::StateSave::memberData
    (void *ptr, UintP size)
  {
    //Copy data from resource to buffer
    store (ptr, size);
  }
  
  void SerializeManager::StateSave::resourcePtr
    (ClassPtr cls, void **pptr)
  {
    //Push the resource info on the queue
    StateSerial::resourcePtr (cls, pptr);
  }

  void SerializeManager::StateSave::resourcePtrPtr
    (ClassPtr cls, void ***pptr, UintP count)
  {
    //Push the resource info on the queue
    StateSerial::resourcePtrPtr (cls, pptr, count);
  }
  
  void SerializeManager::StateSave::dynamicPtr
    (void **pptr, UintP size)
  {
    //Push the data info on the queue
    StateSerial::dynamicPtr (pptr, size);
  }

  /*
  -----------------------------------------------------
  Loading state
  -----------------------------------------------------*/

  void SerializeManager::StateLoad::memberData
    (void *ptr, UintP size)
  {
    //Copy data from buffer to resource
    load (ptr, size);
  }
  
  void SerializeManager::StateLoad::resourcePtr
    (ClassPtr cls, void **pptr)
  {
    //Create a new resource instance and adjust the pointer
    void *pres = cls->newSerialInstance (sm);
    *pptr = (void*) pres;
    
    //Push the resource info on the queue
    StateSerial::resourcePtr (cls, pptr);
  }
  
  void SerializeManager::StateLoad::resourcePtrPtr
    (ClassPtr cls, void ***pptr, UintP count)
  {
    //Allocate array of pointers and adjust the pointer to it
    void *pdata = std::malloc (count * sizeof (void*));
    *pptr = (void**) pdata;
    
    //Push the resource info on the queue
    StateSerial::resourcePtrPtr (cls, pptr, count);
  }
  
  void SerializeManager::StateLoad::dynamicPtr
    (void **pptr, UintP size)
  {
    //Allocate data on the heap and adjust the pointer to it
    void *pdata = std::malloc (size);
    *pptr = (void*) pdata;
    
    //Push the data info on the queue
    StateSerial::dynamicPtr (pptr, size);
  }
  
  /*
  --------------------------------------------------
  Serialization state
  --------------------------------------------------*/
  
  void SerializeManager::StateSerial::adjust (UintP ptrOffset)
  {
    if (!simulate)
    {
      //Make pointer at given offset point to copy offset
      void *pptr = data + ptrOffset;
      Util::PtrSet (pptr, offset);
    }
    
    //Add to the list of adjusted pointers
    PtrHeader ph;
    ph.offset = ptrOffset;
    ptrList.pushBack (ph);
  }
  
  void SerializeManager::StateSerial::run
    (ClassPtr rcls, void *rptr)
  {
    //Push root info to queue
    rootPtr (rcls, rptr);
    while (!resQueue.empty())
    {
      //Pop first resource info off the stack
      ResPtrInfo ri = resQueue.first();
      resQueue.popFront();
      current = &ri;

      if (ri.isptrptr)
      {
        //Adjust the in-data pointer to the resource
        if (!ri.detached) adjust (ri.ptroffset);
        
        //Walk the array of pointers to resources
        void **pptr = (void**)ri.ptr;
        for (UintP p=0; p<ri.count; ++p)
        {
          //Enqueue each pointer to resource
          ResPtrInfo rinew;
          rinew.cls = ri.cls;
          rinew.ptr = pptr[p];
          rinew.count = 1;
          rinew.offset = 0;
          rinew.isptrptr = false;
          rinew.detached = false;
          rinew.ptroffset = offset;
          resQueue.pushBack (rinew);
          
          //Leave space for in-data pointer
          offset += sizeof (void*);
        }
      }
      else
      {
        //Add to list of classes
        ClsHeader ch;
        ch.id = ri.cls->getID();
        ch.count = ri.count;
        ch.offset = offset;
        clsList.pushBack (ch);
        
        //Set the resource offset
        ri.offset = offset;
        
        //Adjust the in-data pointer to resource
        if (!ri.detached) adjust (ri.ptroffset);
        
        //Copy resource data
        store (ri.ptr, ri.cls->getSize());
        
        //Get new pointers from the resource
        ri.cls->invokeCallback (CLSEVT_SERIALIZE, ri.ptr, this);
      }
      
      while (!dynQueue.empty())
      {
        //Pop first dynamic info off the stack
        DynPtrInfo di = dynQueue.first();
        dynQueue.popFront();
        
        //Adjust the in-data pointer to dynamic data
        adjust (di.ptroffset);
        
        //Copy dynamic data
        store (di.ptr, di.size);
      }
    }
  }

  /*
  --------------------------------------------------
  Saving state
  --------------------------------------------------*/

  void SerializeManager::StateSave::run
    (ClassPtr rcls, void *rptr)
  {
    //Push root info to queue
    rootPtr (rcls, rptr);
    while (!resQueue.empty())
    {
      //Pop first resource info off the stack
      ResPtrInfo ri = resQueue.first();
      resQueue.popFront();
      current = &ri;
      
      if (ri.isptrptr)
      {
        //Walk the array of pointers to resources
        void **pptr = (void**)ri.ptr;
        for (UintP p=0; p<ri.count; ++p)
        {
          //Enqueue each pointer to resource
          //...
        }
      }
      else
      {
        //Get new pointers from the resource
        //...
      }

      while (!dynQueue.empty())
      {
        //Copy dynamic data
        //...
      }
    }
  }

  /*
  --------------------------------------------------
  Loading state
  --------------------------------------------------*/

  void SerializeManager::StateLoad::run
    (ClassPtr rcls, void *rptr)
  {
    //Push root info to queue
    rootPtr (rcls, rptr);
    while (!resQueue.empty())
    {
      //Pop first resource info off the stack
      ResPtrInfo ri = resQueue.first();
      resQueue.popFront();
      current = &ri;

      if (ri.isptrptr)
      {
      }
      else
      {
      }
    }
  }

  /*
  --------------------------------------------------
  Serialization start
  --------------------------------------------------*/
  
  void SerializeManager::serialize (ClassPtr cls, void *root, void **outData, UintP *outSize)
  {
    //Enter serialization state
    state = &stateSerial;
    state->sm = this;
    
    //Simulation run
    state->reset (0, false);
    state->run (cls, root);
    
    //Calculate the size of pointer and class table
    UintP introSize =
      (sizeof (UintP) +
       sizeof (PtrHeader) * state->ptrList.size() +
       sizeof (UintP) +
       sizeof (ClsHeader) * state->clsList.size());
    
    //Allocate data
    UintP dataSize = introSize + state->offset;
    state->data = (Uint8*) std::malloc (dataSize);
    *outData = state->data;
    *outSize = dataSize;
    
    //Real run
    state->reset (introSize, true);
    state->run (cls, root);
    
    //Back to start of data
    state->reset (0, true);
    
    //Store pointer list
    UintP numPointers = state->ptrList.size();
    state->store (&numPointers, sizeof (UintP));
    state->store (state->ptrList.buffer(),
                  state->ptrList.size() * sizeof (PtrHeader));
    
    //Store class list
    UintP numClasses = state->clsList.size();
    state->store (&numClasses, sizeof (UintP));
    state->store (state->clsList.buffer(),
                  state->clsList.size() * sizeof (ClsHeader));
  }
  
  void* SerializeManager::deserialize (void *data)
  {
    void *root = NULL;
    
    //Get the number of pointers
    UintP *numPointers = (UintP*) (data);
    PtrHeader *ptrList = (PtrHeader*) (numPointers + 1);
    
    for (UintP p=0; p<*numPointers; ++p)
    {
      //Get the pointer to the pointer
      void *pptr = Util::PtrOff (data, ptrList[p].offset);
      
      //Add the base data address to it
      Util::PtrAdd (pptr, (UintP)data);
    }

    //Get the number of resources
    UintP *numClasses = (UintP*) (ptrList + *numPointers);
    ClsHeader *clsList = (ClsHeader*) (numClasses + 1);
    
    //Get the list of class headers
    for (UintP c=0; c<*numClasses; ++c)
    {
      //Get the pointer to the resource
      void *pres = Util::PtrOff (data, clsList[c].offset);
      if (c == 0) root = pres;
      
      //Construct resource (array) in-place
      ClassPtr cls = ClassFromID (clsList[c].id);
      for (UintP i=0; i<clsList[c].count; ++i)
      {
        cls->newSerialInPlace (pres, this);
        Util::PtrAdd (pres, cls->getSize());
      }
    }
    
    return root;
  }

  /*
  --------------------------------------------------
  Saving start
  --------------------------------------------------*/

  /*
  --------------------------------------------------
  Loading start
  --------------------------------------------------*/

}//namespace GE
