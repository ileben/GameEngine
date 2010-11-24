#include "util/geUtil.h"

namespace GE
{
  std::map< ClassID, BaseFactory* > ClassFactory::factories;

#if(0)

  //////////////////////////////////////////////
  //Base

  class Factory
  {
    ClassID id;
    
  public:
    virtual ClassID uuid() { return id; }
    virtual Uint version () { return 1; }
    
    virtual Object* create() = 0;
    virtual void serialize (Serializer *s) = 0;
  };

  class Object
  {
    friend class Serializer;
    UnitSize serialID;

  public:
    virtual Factory *factory() = 0;
    virtual ~Object() {}
  };

  //////////////////////////////////////////////
  //User with Factory


  virtual Object* create() { return new A };

  class TriMesh : public Mesh
  {
    struct Factory : public Factory< TriMesh > {
      virtual void serialize (Serializer *s, Uint v) {
        Mesh::Factory::serialize();
        //...
      }}

    virtual void Factory* factory()
    { const static AFactory f; return f; }

    //static Object* factory () { return new A };
    //virtual ClassID uuid () { return ClassID(0,0,0,0); }
    //virtual void serialize (Serializer *s, Uint v) { B::serialize(); }
  };

  //////////////////////////////////////////////
  //User inside class

  class A : public B
  {
    virtual ClassID uuid ()
    { return ClassID (0xc0db7169u, 0x65dd, 0x4375, 0xa4b2d9a505703db8ull ); }

    static Object* factory ()
    { return new A };

    virtual void serialize (Serializer *s, Uint v) {
      B::serialize();
      //...
    }
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

  void Serializer::objectPtr (MyObject **pp)
  { state->objectPtr( pp ); }

  void Serializer::objectRef (MyObject **pp)
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
    for (UintSize s=0; s<size; ++s)
      store( a->at( s ), a->elementSize() );
  }
/*
  void Serializer::StateSave::objectPtr (MyObject **pp)
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
*/

  void Serializer::StateSave::objectPtr (MyObject **pp)
  {
    //Enqueue reference
    objectRef (pp);

    //Enqueue object
    SaveObjNode o;
    o.p = *pp;
    o.done = false;
    objs.pushBack( o );
  }

  void Serializer::StateSave::objectRef (MyObject **pp)
  {
    //Enqueue reference
    SaveRefNode r;
    r.p = *pp;
    r.offset = offset;
    refs.pushBack( r );

    //Leave space for serial id
    skip( sizeof( UintSize ));
  }

  void Serializer::StateSave::objectPtrArray (GenericArrayList *a)
  {
    //Store array size
    UintSize size = a->size();
    store( &size, sizeof( UintSize ));

    //Store array elements
    for (UintSize s=0; s<size; ++s)
      objectPtr( (MyObject**) a->at(s) );
  }

  void Serializer::StateSave::objectRefArray (GenericArrayList *a)
  {
    //Store array size
    UintSize size = a->size();
    store( &size, sizeof( UintSize ));

    //Store array elements
    for (UintSize s=0; s<size; ++s)
      objectRef( (MyObject**) a->at(s) );
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
/*
  void Serializer::StateLoad::objectPtr (MyObject **pp)
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
    MyObject *obj = ClassFactory::Produce( cid );
    *pp = obj;

    //Check if object valid
    if (obj != NULL)
    {
      //Enqueue object
      LoadObjNode o;
      o.p = obj;
      o.id = sid;
      o.version = version;
      objs.pushBack( o );
    }
    else
    {
      //Skip object
      skip( size );
    }
  }
*/
  void Serializer::StateLoad::objectPtr (MyObject **pp)
  {
    //Enqueue reference
    objectRef( pp );

    //Enqueue object
    LoadObjNode o;
    objs.pushBack( o );
  };

  void Serializer::StateLoad::objectRef (MyObject **pp)
  {
    //Load serial ID
    UintSize id = 0;
    load( &id, sizeof( UintSize ));

    //Enqueue reference
    LoadRefNode r;
    r.pp = pp;
    r.id = id;
    refs.pushBack( r );
  }

  void Serializer::StateLoad::objectPtrArray (GenericArrayList *a)
  {
    //Load array size
    UintSize size = 0;
    load( &size, sizeof( UintSize ));

    //Store array elements
    a->resize( size );
    for (UintSize s=0; s<size; ++s) {
      objectPtr( (MyObject**) a->at(s) );
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
      objectRef( (MyObject**) a->at(s) );
  }

  /*
  ---------------------------------------------------------
  Save Control
  ---------------------------------------------------------*/

  void Serializer::StateSave::reset (bool simulation)
  {
    objs.clear ();
    refs.clear();
    objects.clear();

    offset = 0;
    simulate = simulation;
  }

  void Serializer::StateSave::run (MyObject **ppRoot)
  {
    //Start with root object
    objectPtr( ppRoot );

    //Process objects
    while (!objs.empty())
    {
      //Pop object node off the stack
      SaveObjNode o = objs.last();
      objs.popBack();

      //Insert object size when done
      if (o.done) {
        UintSize size = offset - o.offset;
        store( &size, o.szOffset, sizeof(UintSize) );
        continue;
      }

      //Map object to serial ID
      o.p->serialID = objects.size();
      objects.pushBack( o.p );

      //Store class ID
      ClassID cid = o.p->uuid();
      store( &cid, sizeof( ClassID ));

      //Store object version
      Uint version = o.p->version();
      store( &version, sizeof( Uint ));

      //Store serial ID
      UintSize sid = o.p->serialID;
      store( &sid, sizeof( UintSize ));

      //Leave space for object size
      UintSize szOffset = offset;
      skip( sizeof( UintSize ));

      //Enqueue object done node
      SaveObjNode oDone = o;
      oDone.done = true;
      oDone.szOffset = szOffset;
      oDone.offset = offset;
      objs.pushBack( oDone );

      //Serialize object members
      o.p->serialize( serializer, version );
    }

    //Process references
    while (!refs.empty())
    {
      //Pop reference node off the stack
      SaveRefNode r = refs.last();
      refs.popBack();

      //Insert object serial ID
      UintSize serialID = r.p->serialID;
      store( &serialID, r.offset, sizeof(UintSize) );
    }
  }

  /*
  ---------------------------------------------------------
  Load Control
  ---------------------------------------------------------*/

  void Serializer::StateLoad::reset (bool simulation)
  {
    objs.clear ();
    refs.clear();
    objects.clear();

    offset = 0;
    simulate = simulation;
  }

  void Serializer::StateLoad::run (MyObject **ppRoot)
  {
    //Start with root object
    objectPtr( ppRoot );

    //Process objects
    while (!objs.empty())
    {
      //Pop object off the stack
      LoadObjNode o = objs.last();
      objs.popBack();

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
      MyObject *p = ClassFactory::Produce( cid );

      //Skip invalid object
      if (p == NULL) {
        skip( size );
        continue;
      }

      //Map object to serial ID
      objects.resizeAndCopy( sid + 1 );
      objects[ sid ] = p;

      //Serialize object members
      p->serialize( serializer, version );
    }

    //Process references
    while (!refs.empty())
    {
      //Pop reference off the stack
      LoadRefNode r = refs.last();
      refs.popBack();

      //Check for invalid serial ID
      if (r.id >= objects.size()) {
        *r.pp = NULL;
        continue;
      }

      //Map serial ID to object
      *r.pp = objects[ r.id ];
    }
  }

  /*
  ---------------------------------------------------------
  High-Level Control
  ---------------------------------------------------------*/

  void Serializer::serialize (MyObject *root, void **outData, UintSize *outSize)
  {
    //Enter saving state
    state = &stateSave;
    state->serializer = this;
    
    //Simulation run
    state->reset( true );
    state->run( &root );
    
    //Allocate buffer
    UintSize bufSize = state->offset;
    state->buffer= (Uint8*) std::malloc( bufSize );
    *outData = state->buffer;
    *outSize = bufSize;
    
    //Real run
    state->reset( false );
    state->run( &root );
  }

  MyObject* Serializer::deserialize (const void *data)
  {
    //Enter loading state
    state = &stateLoad;
    state->serializer = this;
    state->buffer = (Uint8*) data;

    //Run
    MyObject *rootPtr = NULL;
    state->reset( false );
    state->run( &rootPtr );

    return rootPtr;
  }

}//namespace GE
