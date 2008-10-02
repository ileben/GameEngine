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

  There is an exception which cannot be serialized
  directly though:

  - Automatic resource member variable. Even if we did
    recurse into it at serialization stage, we don't have
    any control over its construction when unserializing.
    Only the default constructor will be called so the
    resource itself will have no idea whether it's been
    constructed on the heap or in-place.

  ---------------------------------------------------------*/
  

  void SerializeManager::resourcePtr (ClassPtr cls, void **pptr, UintP count)
  {
    ResPtrInfo ri;
    ri.count = count;
    ri.offset = 0;
    ri.cls = cls;
    ri.ptr = *pptr;
    ri.isptrptr = false;
    ri.detached = false;
    ri.ptroffset = current->offset + Util::PtrDist (current->ptr, pptr);
    resQueue.pushBack (ri);
  }

  void SerializeManager::resourcePtrPtr (ClassPtr cls, void ***pptr, UintP count)
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
  
  void SerializeManager::dynamicPtr (void **pptr, UintP size)
  {
    DynPtrInfo di;
    di.ptr = *(pptr);
    di.size = size;
    di.ptroffset = current->offset + Util::PtrDist (current->ptr, pptr);
    dynQueue.pushBack (di);
  }

  /*
  --------------------------------------------------
  These are the functions that form the core of the
  serialization algorithm.
  --------------------------------------------------*/

  void SerializeManager::copy (void *ptr, UintP size)
  {
    if (!simulate)
    {
      //Copy [size] bytes of data to buffer at copy offset
      std::memcpy (data + offset, ptr, size);
    }
    
    //Advance copy offset
    offset += size;
  }
  
  void SerializeManager::adjust (UintP ptrOffset)
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
  
  void SerializeManager::run (ClassPtr rootCls, void *rootPtr)
  {
    //Push root info to queue
    ResPtrInfo riroot;
    riroot.cls = rootCls;
    riroot.ptr = rootPtr;
    riroot.count = 1;
    riroot.offset = 0;
    riroot.isptrptr = false;
    riroot.detached = true;
    riroot.ptroffset = 0;
    resQueue.pushBack (riroot);
    
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
        //Get class info
        ClassID clsID = ri.cls->getID();
        UintP clsSize = ri.cls->getSize();
        
        //Add to list of classes
        ClsHeader ch;
        ch.id = clsID;
        ch.count = ri.count;
        ch.offset = offset;
        clsList.pushBack (ch);
        
        //Set the resource offset
        ri.offset = offset;
        
        //Adjust the in-data pointer to resource
        if (!ri.detached) adjust (ri.ptroffset);
        
        //Copy resource data
        copy (ri.ptr, ri.count * clsSize);
        
        //Walk the array of resources
        for (UintP r=0; r<ri.count; ++r)
        {
          //Get new pointers from each resource
          ri.cls->invokeCallback (CLSEVT_SERIALIZE, ri.ptr, this);
          Util::PtrAdd (&ri.ptr, clsSize);
          ri.offset += clsSize;
        }
      }
      
      while (!dynQueue.empty())
      {
        //Pop first dynamic info off the stack
        DynPtrInfo di = dynQueue.first();
        dynQueue.popFront();
        
        //Adjust the in-data pointer to dynamic data
        adjust (di.ptroffset);
        
        //Copy dynamic data
        copy (di.ptr, di.size);
      }
    }
  }
  
  void SerializeManager::serialize (ClassPtr cls, void *root, void **outData, UintP *outSize)
  {
    //Reset
    simulate = true;
    resQueue.clear ();
    dynQueue.clear ();
    ptrList.clear ();
    clsList.clear ();
    offset = 0;
    
    //Simulation run
    run (cls, root);
    
    //Allocate data
    UintP introSize =
      (sizeof(UintP) + clsList.size() * sizeof (ClsHeader) +
       sizeof(UintP) + ptrList.size() * sizeof (PtrHeader));
    UintP dataSize = introSize + offset;
    data = (Uint8*) std::malloc (dataSize);
    *outData = data;
    *outSize = dataSize;
    
    //Reset
    resQueue.clear ();
    dynQueue.clear ();
    ptrList.clear ();
    clsList.clear ();
    simulate = false;
    offset = introSize;
     
    //Real run
    run (cls, root);
    
    //Back to start of data
    offset = 0;

    //Store pointer list
    UintP numPointers = ptrList.size();
    copy (&numPointers, sizeof (UintP));
    copy (ptrList.buffer(), ptrList.size() * sizeof (PtrHeader));
    
    //Store class list
    UintP numClasses = clsList.size();
    copy (&numClasses, sizeof (UintP));
    copy (clsList.buffer(), clsList.size() * sizeof (ClsHeader));
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
        cls->newDeserialized (pres, this);
        Util::PtrAdd (pres, cls->getSize());
      }
    }
    
    return root;
  }

}//namespace GE
