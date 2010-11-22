#include "util/geUtil.h"

namespace GE
{
#if(0)
  class Object
  {
    friend class Serializer;
    UnitSize serialID;
    virtual ClassID uuid() = 0;
    virtual void serialize (Serializer *s) = 0;
    virtual Uint version () { return 1; }

  public:
    virtual ~Object() {}
  };

  class A : public B
  {
    static Object* factory () { return new A };
    virtual ClassID uuid () { return ClassID(0,0,0,0); }
    virtual void serialize (Serializer *s, Uint v) { B::serialize(); }
  };
#endif

  /*
  ---------------------------------------------------------
  Data management
  ---------------------------------------------------------*/

  void Serializer::State::skip (UintSize size)
  {
    //Advance copy offset
    offset += size;
  }

  void Serializer::State::store (void *from, UintSize to, UintSize size)
  {
    if (!simulate)
    {
      //Copy [size] bytes of data to buffer at given offset
      std::memcpy( buffer + to, from, size );
    }
  }

  void Serializer::State::store (void *from, UintSize size)
  {
    if (!simulate)
    {
      //Copy [size] bytes of data to buffer at copy offset
      std::memcpy( buffer + offset, from, size );
    }
    
    //Advance copy offset
    offset += size;
  }

  void Serializer::State::load (void *to, UintSize size)
  {
    if (!simulate)
    {
      //Copy [size] bytes of data from buffer at copy offset
      std::memcpy( to, buffer + offset, size );
    }

    //Advance copy offset
    offset += size;
  }

  void* Serializer::State::pointer ()
  {
    //Pointer to buffer at current offset
    return buffer + offset;
  }

  /*
  ---------------------------------------------------------
  Base Callbacks
  ---------------------------------------------------------*/

  void Serializer::data (void *p, UintSize size)
  { state->data( p, size ); }

  void Serializer::dataPtr (void **pp, UintSize *size)
  { state->dataPtr( pp, size ); }

  void Serializer::dataArray (GenericArrayList *a)
  { state->dataArray( a ); }

  void Serializer::objectPtr (Object **pp)
  { state->objectPtr( pp ); }

  void Serializer::objectRef (Object **pp)
  { state->objectRef( pp ); }

  void Serializer::objectPtrArray (GenericArrayList *a)
  { state->objectPtrArray( a ); }

  void Serializer::objectRefArray (GenericArrayList *a)
  { state->objectRefArray( a ); }

  /*
  ---------------------------------------------------------
  Save Callbacks
  ---------------------------------------------------------*/

  void Serializer::StateSave::data (void *p, UintSize size)
  {
    //Store data from the pointed variable
    store( p, size );
  }

  void Serializer::StateSave::dataPtr (void **pp, UintSize *size)
  {
    //Store data size
    store( size, sizeof(UintSize) );

    //Store data from the buffer
    store( *pp, *size );
  }

  void Serializer::StateSave::dataArray (GenericArrayList *a)
  {
    //Store array size
    UintSize size = a->size();
    store( &size, sizeof( UintSize ));

    //Store array elements
    for (UintSize s=0; s<a->size(); ++s)
      store( a->at( s ), a->elementSize() );
  }

  void Serializer::StateSave::objectPtr (Object **pp)
  {
    //Store class ID
    ClassID cid = (*pp)->uuid();
    store( &cid, sizeof( ClassID ));

    //Store object version
    Uint version = (*pp)->version();
    store( &version, sizeof( Uint ));

    //Leave space for serial ID
    UintSize idOffset = offset;
    skip( sizeof( UintSize ));

    //Leave space for object size
    UintSize szOffset = offset;
    skip( sizeof( UintSize ));

    //Enqueue object
    SaveObjNode o;
    o.p = *pp;
    o.idOffset = idOffset;
    o.szOffset = szOffset;
    o.offset = offset;
    o.done = false;
    objs.pushBack( o );
  }

  void Serializer::StateSave::objectRef (Object **pp)
  {
    //Enqueue reference
    SaveRefNode r;
    r.p = *pp;
    r.offset = offset;

    //Leave space for serial id
    skip( sizeof( UintSize ));
  }

  void Serializer::StateSave::objectPtrArray (GenericArrayList *a)
  {
    //Store array size
    UintSize size = a->size();
    store( &size, sizeof( UintSize ));

    //Store array elements
    for (UintSize s=0; s<a->size(); ++s)
      objectPtr( (Object**) a->at(s) );
  }

  void Serializer::StateSave::objectRefArray (GenericArrayList *a)
  {
    //Store array size
    UintSize size = a->size();
    store( &size, sizeof( UintSize ));

    //Store array elements
    for (UintSize s=0; s<a->size(); ++s)
      objectRef( (Object**) a->at(s) );
  }

  /*
  ---------------------------------------------------------
  Load Callbacks
  ---------------------------------------------------------*/

  void Serializer::StateLoad::data (void *p, UintSize size)
  {
    //Load data into variable
    load( p, size );
  }

  void Serializer::StateLoad::dataPtr (void **pp, UintSize *size)
  {
    //Load data size
    load( size, sizeof( UintSize ));

    //Allocate a buffer for data
    *pp = std::malloc( *size );
    
    //Load data into buffer
    load( *pp, *size );
  }

  void Serializer::StateLoad::dataArray (GenericArrayList *a)
  {
    //Load array size
    UintSize size = 0;
    load( &size, sizeof( UintSize ));

    //Load array elements
    a->reserve( size );
    for (UintSize s=0; s<size; ++s) {
      a->pushBack( pointer() );
      skip( a->elementSize() );
    }
  }

  void Serializer::StateLoad::objectPtr (Object **pp)
  {
    //Load class ID
    ClassID cid;
    load( &cid, sizeof( ClassID ));

    //Load object version
    Uint version = 0;
    load( &version, sizeof( Uint ));

    //Load serial ID
    UintSize sid = 0;
    load( &sid, sizeof( UintSize ));

    //Load object size
    UintSize size = 0;
    load( &size, sizeof( UintSize ));

    //Instantiate object
    Object *obj = ClassFactory::Produce( cid );
    *pp = obj;

    //Check if object valid
    if (obj != NULL)
    {
      //Enqueue object
      LoadObjNode o;
      o.p = obj;
      o.id = sid;
      objs.pushBack( o );
    }
    else
    {
      //Skip object
      skip( size );
    }
  }

  void Serializer::StateLoad::objectRef (Object **pp)
  {
    //Load serial ID
    UintSize id = 0;
    load( &id, sizeof( UintSize ));

    //Enqueue reference
    LoadRefNode r;
    r.pp = pp;
    r.id = id;
  }

  void Serializer::StateLoad::objectPtrArray (GenericArrayList *a)
  {
    //Load array size
    UintSize size = 0;
    load( &size, sizeof( UintSize ));

    //Store array elements
    a->resize( size );
    for (UintSize s=0; s<size; ++s) {
      objectPtr( (Object**) a->at(s) );
    }
  }

  void Serializer::StateLoad::objectRefArray (GenericArrayList *a)
  {
    //Load array size
    UintSize size = 0;
    load( &size, sizeof( UintSize ));

    //Store array elements
    a->resize( size );
    for (UintSize s=0; s<size; ++s)
      objectRef( (Object**) a->at(s) );
  }

  /*
  ---------------------------------------------------------
  Save Control
  ---------------------------------------------------------*/

  void Serializer::StateSave::reset (bool realRun)
  {
    objs.clear ();
    refs.clear();
    objects.clear();

    offset = 0;
    simulate = !realRun;
  }

  void Serializer::StateSave::run (Object **ppRoot)
  {
    //Start with root object
    objectPtr( ppRoot );

    //Process objects
    while (!objs.empty())
    {
      //Pop object node off the stack
      SaveObjNode o = objs.last();
      objs.popBack();

      //Check if done
      if (o.done)
      {
        //Store object size now that it's known
        UintSize size = offset - o.offset;
        store( &size, o.szOffset, sizeof(UintSize) );
        continue;
      }

      //Enqueue object done node
      SaveObjNode oDone = o;
      oDone.done = true;
      objs.pushBack( oDone );

      //Store object serial ID now that it's known
      o.p->serialID = objects.size();
      store( &o.p->serialID, o.idOffset, sizeof(UintSize) );
      objects.pushBack( o.p );

      //Serialize object
      o.p->serialize();
    }

    //Process references
    while (!refs.empty())
    {
      //Pop reference off the stack
      SaveRefNode r = refs.last();
      refs.popBack();

      //Store reference serial ID
      UintSize serialID = r.p->serialID;
      store( &serialID, r.offset, sizeof(UintSize) );
    }
  }

  /*
  ---------------------------------------------------------
  Load Control
  ---------------------------------------------------------*/

  void Serializer::StateLoad::reset (bool realRun)
  {
    objs.clear ();
    refs.clear();
    objects.clear();

    offset = 0;
    simulate = !realRun;
  }

  void Serializer::StateLoad::run (Object **ppRoot)
  {
    //Start with root object
    objectPtr( ppRoot );

    //Process objects
    while (!objs.empty())
    {
      //Pop object off the stack
      LoadObjNode o = objs.last();
      objs.popBack();

      //Map object to serial ID
      objects.resizeAndCopy( o.id );
      objects[ o.id ] = o.p;

      //Serialize object
      o.p->serialize();
    }

    //Process references
    while (!refs.empty())
    {
      //Pop reference off the stack
      LoadRefNode r = refs.last();
      refs.popBack();

      //Check for invalid serial ID
      if (r.id < objects.size())
        *r.pp = NULL;

      //Map serial ID to object
      *r.pp = objects[ r.id ];
    }
  }

}//namespace GE
