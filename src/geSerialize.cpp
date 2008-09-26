#define GE_API_EXPORT
#include "geEngine.h"

namespace GE
{
  void SerializeManager::resource (void *ptr)
  {
    ResPtrInfo ri;
    ri.ptr = (IResource*)ptr;
    ri.dynamic = false;
    ri.detached = false;
    ri.ptroffset = 0;
    resQueue.pushBack (ri);
  }
  
  void SerializeManager::resourcePtr (void *pptr)
  {
    ResPtrInfo ri;
    ri.ptr = *((IResource**)pptr);
    ri.dynamic = true;
    ri.detached = false;
    ri.ptroffset = offset + Util::PtrDist (current, pptr);
    resQueue.pushBack (ri);
  }
  
  void SerializeManager::resourceArray (void *pptr, UintP size)
  {
    DynPtrInfo di;
    di.ptr = *((void**)pptr);
    di.size = size;
    di.isarray = true;
    di.ofptr = false;
    di.ptroffset = offset + Util::PtrDist (current, pptr);
    dynQueue.pushBack (di);
  }
  
  void SerializeManager::resourcePtrArray (void *pptr, UintP size)
  {
    DynPtrInfo di;
    di.ptr = *((void**)pptr);
    di.size = size;
    di.isarray = true;
    di.ofptr = true;
    di.ptroffset = offset + Util::PtrDist (current, pptr);
    dynQueue.pushBack (di);
  }
  
  void SerializeManager::dynamicPtr (void *pptr, UintP size)
  {
    DynPtrInfo di;
    di.ptr = *((void**)pptr);
    di.size = size;
    di.isarray = false;
    di.ofptr = false;
    di.ptroffset = offset + Util::PtrDist (current, pptr);
    dynQueue.pushBack (di);
  }

  void SerializeManager::copy (void *ptr, UintP size)
  {
    if (!simulate)
    {
      //Copy [size] bytes of data to buffer at current offset
      std::memcpy (data + offset, ptr, size);
    }
    
    //Advance current offset
    offset += size;
  }
  
  void SerializeManager::adjust (UintP ptrOffset)
  {
    if (!simulate)
    {
      //Make pointer at given offset point to current offset
      void *pptr = data + ptrOffset;
      Util::PtrSet (pptr, offset);
    }
    
    //Add to the list of adjusted pointers
    ptrList.pushBack (ptrOffset);
  }
  
  void SerializeManager::run (IResource *root)
  {
    //Push root info to queue
    ResPtrInfo ri;
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
      current = ri.ptr;
      
      //--- We are now at the header of the resource
      
      //Store the offset to resource header
      resList.pushBack (offset);
       
      //Copy resource ID
      Uint32 id = ri.ptr->getID();
      copy (&id, sizeof (Uint32));       
      
      //--- We are now at the actual start of the resource class
      
      //Get new pointer info from the class
      ri.ptr->getPointers (this);    
      
      //Is the resource on the heap?
      if (ri.dynamic)
      {
        //Adjust the in-data pointer to resource
        if (!ri.detached) adjust (ri.ptroffset);
        
        //Copy resource data
        copy (ri.ptr, ri.ptr->getSize());
      }
      
      while (!dynQueue.empty())
      {
        //Pop first dynamic info off the stack
        DynPtrInfo di = dynQueue.first();
        dynQueue.popFront();

        if (di.isarray)
        {
          if (di.ofptr)
          {
            //It points to an array of pointers to resources
            IResource **resources = (IResource**) di.ptr;
            int numResources = (int)di.size;
            di.size *= sizeof (IResource*);
            
            //Add resource info for every pointer in the array
            for (int r=0; r<numResources; ++r) {
              ResPtrInfo ri;
              ri.ptr = resources[r];
              ri.dynamic = true;
              ri.detached = false;
              ri.ptroffset = offset + r * sizeof (IResource*);
              resQueue.pushBack (ri);
            }

          }else{

            //It points to an array of resources.
            //Get size off the first resource.
            IResource *resources = (IResource*) di.ptr;
            UintP resSize = resources[0].getSize();
            int numResources = (int)di.size;
            di.size *= resSize;
            
            //Add resource info for every resource in the array
            for (int r=0; r<numResources; ++r) {
              ResPtrInfo ri;
              ri.ptr = (IResource*) Util::PtrOff (resources, r*resSize);
              ri.dynamic = false;
              ri.detached = false;
              ri.ptroffset = 0;
              resQueue.pushBack (ri);
            }
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
    resList.clear ();
    offset = 0;
    
    //Simulation run
    run (root);
    
    //Allocate data
    UintP introSize = (resList.size() + 1 + ptrList.size() + 1) * sizeof (UintP); 
    UintP dataSize = introSize + offset;
    data = (Uint8*) std::malloc (offset);
    *outData = data;
    *outSize = offset;
    
    //Reset
    resQueue.clear ();
    dynQueue.clear ();
    ptrList.clear ();
    resList.clear ();
    simulate = false;
    offset = introSize;

    //Real run
    run (root);
    
    //Back to start of data
    offset = 0;
    
    //Store resource list
    UintP numResources = resList.size();
    copy (&numResources, sizeof (numResources));
    copy (resList.buffer(), resList.size() * sizeof (UintP));
    
    //Store pointer list
    UintP numPointers = ptrList.size();
    copy (&numPointers, sizeof (numPointers));
    copy (ptrList.buffer(), ptrList.size() * sizeof (UintP));
  }

  IResource* SerializeManager::load (void *data)
  {
    IResource *root = NULL;

    //Get the number of resources
    UintP *resList = (UintP*) data;
    UintP numResources = *resList;
    ++resList;
    
    for (UintP r=0; r<numResources; ++r, ++resList)
    {
      //Get the pointer to the class ID and resource data
      UintP offset = *resList;
      Uint32 *pID = (Uint32*) Util::PtrOff (data, offset);
      IResource *pRes = (IResource*) Util::PtrOff (data, offset + 4);
      if (r == 0) root = pRes;

      //Construct resource in-place
      switch (*pID) {
      case 0: new (pRes) TestResource ();
      }
    }

    //Get the number of pointers
    UintP *ptrList = resList;
    UintP numPointers = *ptrList;
    ++ptrList;

    for (UintP p=0; p<numPointers; ++p, ++ptrList)
    {
      //Get the pointer to the pointer
      UintP offset = *ptrList;
      void *pptr = Util::PtrOff (data, offset);

      //Add the base data address to it
      Util::PtrAdd (pptr, (UintP)data);
    }

    return root;
  }

}//namespace GE
