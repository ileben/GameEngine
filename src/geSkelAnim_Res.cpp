#define GE_API_EXPORT
#include "geEngine.h"
using namespace OCC;

namespace GE
{
  /*
  struct ResourceArray
  {
    void                      *ptr;
    ArrayList<ResourceNode*>  *arr;
  };
  
  template <class N> class ResourceElement
  {
    virtual void copy (ResourceFactory *f) = 0;
    UintP size () { return sizeof (N); }
  };
  
  template <class R> class ResourceFactory
  {
  private:
    Uint8 *data;
    UintP offset;
    LinkedList <ResourceArray> arrayQueue;
    UintP structSize;
    bool sizeCountMode;

  public:
    virtual void copy (ResourceFactory *f) = 0;
    
    void copyBlank (UintP size)
    {
      //Advance the data offset
      offset += size;
    }
    
    void copyVar (void *p, UintP size)
    {
      //Copy [size] number of bytes from the given address
      if (!sizeCountMode) std::memcpy (data + offset, p, size);
      
      //Advance the data offset
      offset += size;
    }
    
    void copyArray (void *p, void *ptarget, UintP size)
    {
      //Insert a Uint32 for array size
      Uint32 asize = (Uint32) a->size ();
      copyVar (&asize, sizeof (asize));
      
      //Queue the array for traversal
      ResourceArray ra;
      ra.ptr = data + offset;
      ra.arr = a;
      arrayQueue.pushBack (ra);
      
      //Skip a blank for the pointer to array
      copyBlank (sizeof (void*));
    }
    
    void create (void **outMem, UintP *outSize)
    {
      //Reset
      offset = 0;
      arrayQueue.clear ();
      sizeCountMode = true; 
      
      //Count only size of array data
      copy ();
      processArrays ();
      
      //Allocate data
      data = (Uint8*) std::malloc (offset);

      //Reset
      offset = 0;
      arrayQueue.clear ();
      sizeCountMode = false;
      
      //Copy ourselves
      copy ();
      processArrays ();
    }
    
    void processArrays ()
    {
      //Copy queued arrays of nodes
      while (!arrayQueue.empty())
      {
        //Pop an aray from the queue
        ResourceArray ra = arrayQueue.first ();
        arrayQueue.popFront ();
        
        //Set its pointer to the current data offset
        Util::PtrSet (ra->ptr, data + offset);
        
        //Copy all of its elements
        for (int i=0; i<ra->size(); ++i)
          ra->at(i)->copy (this);
      }
    }
    

  };
  
  void SkelAnim_Factory::create (void **outMem, UintP *outSize)
  {
    int t;

    //Calculate sizes
    UintP dataSize = 0;

    UintP structSize = sizeof (SkelAnim_Res);
    dataSize += structSize;

    UintP tracksSize = tracks.size() * sizeof (SkelAnimTrack);
    dataSize += tracksSize;

    UintP *keySizes = new UintP [tracks.size()];
    for (t=0; t<tracks.size(); ++t)
    {
      keySizes [t] = tracks[t]->keys.size() * sizeof (SkelAnimKey);
      dataSize += keySizes [t];
    }
    
    *outSize = dataSize;
    
    //Allocate memory
    SkelAnim_Res *data = (SkelAnim_Res*) std::malloc (dataSize);
    *outMem = data;
    
    //Adjust pointers
    UintP dataOffset = (UintP)data;
    dataOffset += structSize;
    
    Util::PtrSet (&data->tracks, dataOffset);
    dataOffset += tracksSize;
    
    for (t=0; t<tracks.size(); ++t)
    {
      Util::PtrSet (&data->tracks[t].keys, dataOffset);
      dataOffset += keySizes [t];
    }
    
    //Copy data
    data->duration = duration;
    data->numTracks = tracks.size();
    
    for (t=0; t<tracks.size(); ++t)
    {
      data->tracks[t].numKeys = tracks[t]->keys.size();
      std::memcpy (data->tracks[t].keys, tracks[t]->keys.buffer(), keySizes [t]);
    }
    
    //Re-adjust pointers
    Util::PtrSub (&data->tracks, (UintP)data);

    for (t=0; t<tracks.size(); ++t)
      Util::PtrSub (&data->tracks[t].keys, (UintP)data);
  }
  */

}//namespace GE
