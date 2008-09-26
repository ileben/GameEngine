#define GE_API_EXPORT
#include "geEngine.h"

namespace GE
{
  /*
  -----------------------------------------------------------
  These functions are to be called from within the resources
  getPointers () function. They mark other resources within
  the current one to be processed, or other external data
  held by this resource, such as a pointer to a resources,
  a pointer to an array of them, or general data on the heap.
  
  The byte offset to the serialized conterpart of the
  pointers or resources within the current resource
  structure are calculated from the serialized offset of the
  currently processed resource and the offset of the member
  variable within its structure.

  Note that a resource member variable doesn't have a
  pointer to it so it doesn't need to store its offset.
  On the other hand a resource pointer member variable
  can't have the serialized resource offset set at the
  time it is specified. It is only set later on when the
  resource is being processed and the current copy offset
  is known (but before new pointers are obtained from that
  resource).
  -----------------------------------------------------------*/

  void SerializeManager::resource (void *ptr)
  {
    ResPtrInfo ri;
    ri.count = 1;
    ri.offset = current->offset + Util::PtrDist (current->ptr, ptr);
    ri.ptr = (IResource*)ptr;
    ri.dynamic = false;
    ri.detached = false;
    ri.ptroffset = 0;
    resQueue.pushBack (ri);
  }
  
  void SerializeManager::resourcePtr (void *pptr)
  {
    ResPtrInfo ri;
    ri.count = 1;
    ri.offset = 0;
    ri.ptr = *((IResource**)pptr);
    ri.dynamic = true;
    ri.detached = false;
    ri.ptroffset = current->offset + Util::PtrDist (current->ptr, pptr);
    resQueue.pushBack (ri);
  }
  
  void SerializeManager::resourceArray (void *pptr, UintP count)
  {
    ResPtrInfo ri;
    ri.count = count;
    ri.offset = 0;
    ri.ptr = *((IResource**)pptr);
    ri.dynamic = true;
    ri.detached = false;
    ri.ptroffset = current->offset + Util::PtrDist (current->ptr, pptr);
    resQueue.pushBack (ri);
  }
  
  void SerializeManager::resourcePtrArray (void *pptr, UintP size)
  {
    DynPtrInfo di;
    di.ptr = *((void**)pptr);
    di.size = size;
    di.isarray = true;
    di.ptroffset = current->offset + Util::PtrDist (current->ptr, pptr);
    dynQueue.pushBack (di);
  }
  
  void SerializeManager::dynamicPtr (void *pptr, UintP size)
  {
    DynPtrInfo di;
    di.ptr = *((void**)pptr);
    di.size = size;
    di.isarray = false;
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
  
  void SerializeManager::run (IResource *root)
  {
    //Push root info to queue
    ResPtrInfo ri;
    ri.count = 1;
    ri.offset = 0;
    ri.ptr = (IResource*)root;
    ri.dynamic = true;
    ri.detached = true;
    ri.ptroffset = 0;
    resQueue.pushBack (ri);
    
    while (!resQueue.empty())
    {
      //Pop first resource info off the stack
      ResPtrInfo ri = resQueue.first();
      resQueue.popFront();
      current = &ri;
      
      //Get class info
      Uint32 clsID = ri.ptr->getID ();
      UintP clsSize = ri.ptr->getSize();
      
      //Is the resource on the heap?
      if (ri.dynamic)
      {
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
      }
      
      //Get new pointer info from the class
      for (UintP r=0; r<ri.count; ++r)
      {
        ri.ptr->getPointers (this);
        ri.offset += clsSize;
        Util::PtrAdd (&ri.ptr, clsSize);
      }
      
      while (!dynQueue.empty())
      {
        //Pop first dynamic info off the stack
        DynPtrInfo di = dynQueue.first();
        dynQueue.popFront();
        
        if (di.isarray)
        {
          //It points to an array of pointers to resources
          IResource **resources = (IResource**) di.ptr;
          UintP numResources = di.size;
          di.size *= sizeof (IResource*);
          
          //Add resource info for every pointer in the array
          for (UintP r=0; r<numResources; ++r) {
            ResPtrInfo ri;
            ri.count = 1;
            ri.offset = 0;
            ri.ptr = resources[r];
            ri.dynamic = true;
            ri.detached = false;
            ri.ptroffset = offset + r * sizeof (IResource*);
            resQueue.pushBack (ri);
          }
        }
        
        //Adjust the in-data pointer to dynamic data
        adjust (di.ptroffset);
        
        //Copy dynamic data
        copy (di.ptr, di.size);
      }
    }
  }
  
  void SerializeManager::serialize (IResource *root, void **outData, UintP *outSize)
  {
    //Reset
    simulate = true;
    resQueue.clear ();
    dynQueue.clear ();
    ptrList.clear ();
    clsList.clear ();
    offset = 0;
    
    //Simulation run
    run (root);
    
    //Allocate data
    UintP introSize =
      (sizeof(UintP) + clsList.size() * sizeof (ClsHeader) +
       sizeof(UintP) + ptrList.size() * sizeof (PtrHeader));
    UintP dataSize = introSize + offset;
    data = (Uint8*) std::malloc (offset);
    *outData = data;
    *outSize = offset;
    
    //Reset
    resQueue.clear ();
    dynQueue.clear ();
    ptrList.clear ();
    clsList.clear ();
    simulate = false;
    offset = introSize;
     
    //Real run
    run (root);
    
    //Back to start of data
    offset = 0;
    
    //Store class list
    UintP numClasses = clsList.size();
    copy (&numClasses, sizeof (UintP));
    copy (clsList.buffer(), clsList.size() * sizeof (ClsHeader));
    
    //Store pointer list
    UintP numPointers = ptrList.size();
    copy (&numPointers, sizeof (UintP));
    copy (ptrList.buffer(), ptrList.size() * sizeof (PtrHeader));
  }
  
  IResource* SerializeManager::load (void *data)
  {
    IResource *root = NULL;

    //Get the number of resources
    UintP *numClasses = (UintP*) data;
    ClsHeader *clsList = (ClsHeader*) (numClasses + 1);
    
    //Get the list of class headers
    for (UintP c=0; c<*numClasses; ++c)
    {
      //Get the pointer to the resource
      void *pres = Util::PtrOff (data, clsList[c].offset);
      if (c == 0) root = (IResource*) pres;
      
      //Construct resource (array) in-place
      switch (clsList[c].id) {
      case 0: new (pres) TestResource [clsList[c].count];
      }
    }
    
    //Get the number of pointers
    UintP *numPointers = (UintP*) (clsList + *numClasses);
    PtrHeader *ptrList = (PtrHeader*) (numPointers + 1);
    
    for (UintP p=0; p<*numPointers; ++p, ++ptrList)
    {
      //Get the pointer to the pointer
      void *pptr = Util::PtrOff (data, ptrList[p].offset);
      
      //Add the base data address to it
      Util::PtrAdd (pptr, (UintP)data);
    }

    return root;
  }

}//namespace GE
