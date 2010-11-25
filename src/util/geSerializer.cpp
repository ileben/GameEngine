#include "util/geUtil.h"

namespace GE
{
  /*
  ---------------------------------------------------------
  Static members
  ---------------------------------------------------------*/

  Serializer::ClassMap Serializer::classes;

  /*
  ---------------------------------------------------------
  Data management
  ---------------------------------------------------------*/

  void Serializer::State::skip (UintSize size)
  {
    //Advance copy offset
    offset += size;
  }

  void Serializer::State::store (const void *from, UintSize to, UintSize size)
  {
    if (!simulate)
    {
      //Copy [size] bytes of data to buffer at given offset
      std::memcpy( buffer + to, from, size );
    }
  }

  void Serializer::State::store (const void *from, UintSize size)
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

  void Serializer::string (CharString *s)
  { state->string( s ); }

  void Serializer::stringArray (ArrayList< CharString > *a)
  { state->stringArray( a ); }

  void Serializer::object (Object *p)
  { state->object( p ); }

  void Serializer::objectPtr (Object **pp)
  { state->objectPtr( pp ); }

  void Serializer::objectRef (Object **pp)
  { state->objectRef( pp ); }

  void Serializer::objectArray (GenericArrayList *a)
  { state->objectArray( a ); }

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

  void Serializer::StateSave::string (CharString *s)
  {
    //Store string length
    UintSize len = s->length();
    store( &len, sizeof( UintSize ));

    //Store string buffer
    store( s->buffer(), len * sizeof(char) );
  }

  void Serializer::StateSave::stringArray (ArrayList< CharString > *a)
  {
    //Store array size
    UintSize size = a->size();
    store( &size, sizeof( UintSize ));

    //Store array elements
    for (UintSize s=0; s<size; ++s)
      string( &a->at( s ) );
  }

  void Serializer::StateSave::objectPtr (Object **pp)
  {
    //Enqueue reference
    objectRef (pp);

    //Enqueue object
    SaveObjNode o;
    o.p = *pp;
    o.done = false;
    objStack.pushBack( o );
  }

  void Serializer::StateSave::objectRef (Object **pp)
  {
    //Enqueue reference
    SaveRefNode r;
    r.p = *pp;
    r.offset = offset;
    refStack.pushBack( r );

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
      objectPtr( (Object**) a->at(s) );
  }

  void Serializer::StateSave::objectRefArray (GenericArrayList *a)
  {
    //Store array size
    UintSize size = a->size();
    store( &size, sizeof( UintSize ));

    //Store array elements
    for (UintSize s=0; s<size; ++s)
      objectRef( (Object**) a->at(s) );
  }

  void Serializer::StateSave::object (Object *p)
  {
    //Store object version
    Uint version = p->version();
    store( &version, sizeof( Uint ));

    //Record all the objects
    serializer->objects.pushBack( p );

    //Serialize object members
    p->serialize( serializer, version );
  }

  void Serializer::StateSave::objectArray (GenericArrayList *a)
  {
    //Store array size
    UintSize size = a->size();
    store( &size, sizeof( UintSize ));

    //Store array elements
    for (UintSize s=0; s<size; ++s)
      object( (Object*) a->at(s) );
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
    a->resize( size );
    for (UintSize s=0; s<size; ++s)
      data( a->at(s), a->elementSize() );
  }

  void Serializer::StateLoad::string (CharString *s)
  {
    //Load string length
    UintSize len = 0;
    load( &len, sizeof( UintSize ));

    //Load string buffer
    s->assign( (char*) pointer(), len );
    skip( len * sizeof( char ));
  }

  void Serializer::StateLoad::stringArray (ArrayList< CharString > *a)
  {
    //Load array size
    UintSize size = 0;
    load( &size, sizeof( UintSize ));

    //Load array elements
    a->resize( size );
    for (UintSize s=0; s<size; ++s)
      string( &a->at(s) );
  }

  void Serializer::StateLoad::objectPtr (Object **pp)
  {
    //Enqueue reference
    objectRef( pp );

    //Enqueue object
    LoadObjNode o;
    objStack.pushBack( o );
  };

  void Serializer::StateLoad::objectRef (Object **pp)
  {
    //Load serial ID
    UintSize id = 0;
    load( &id, sizeof( UintSize ));

    //Enqueue reference
    LoadRefNode r;
    r.pp = pp;
    r.id = id;
    refStack.pushBack( r );
  }

  void Serializer::StateLoad::objectPtrArray (GenericArrayList *a)
  {
    //Load array size
    UintSize size = 0;
    load( &size, sizeof( UintSize ));

    //Load array elements
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

    //Load array elements
    a->resize( size );
    for (UintSize s=0; s<size; ++s)
      objectRef( (Object**) a->at(s) );
  }

  void Serializer::StateLoad::object (Object *p)
  {
    //Load object version
    Uint version = 0;
    load( &version, sizeof( Uint ));

    //Record all the objects
    serializer->objects.pushBack( p );

    //Serialize object members
    p->serialize( serializer, version );
  }

  void Serializer::StateLoad::objectArray (GenericArrayList *a)
  {
    //Load array size
    UintSize size = 0;
    load( &size, sizeof( UintSize ));

    //Load array elements
    a->resize( size );
    for (UintSize s=0; s<size; ++s)
      object( (Object*) a->at(s) );
  }

  /*
  ---------------------------------------------------------
  Save Control
  ---------------------------------------------------------*/

  void Serializer::StateSave::reset (bool simulation)
  {
    serializer->objects.clear();

    objStack.clear ();
    refStack.clear();
    objMap.clear();

    offset = 0;
    simulate = simulation;
  }

  void Serializer::StateSave::run (Object **ppRoot)
  {
    //Start with root object
    objectPtr( ppRoot );

    //Process objects
    while (!objStack.empty())
    {
      //Pop object node off the stack
      SaveObjNode o = objStack.last();
      objStack.popBack();

      //Insert object size when done
      if (o.done) {
        UintSize size = offset - o.offset;
        store( &size, o.szOffset, sizeof(UintSize) );
        continue;
      }

      //Map object to serial ID
      o.p->serialID = objMap.size();
      objMap.pushBack( o.p );

      //Store class ID
      UUID cid = o.p->getClass()->uuid();
      store( &cid, sizeof( UUID ));

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
      objStack.pushBack( oDone );

      //Record all the objects
      serializer->objects.pushBack( o.p );

      //Serialize object members
      o.p->serialize( serializer, version );
    }

    //Process references
    while (!refStack.empty())
    {
      //Pop reference node off the stack
      SaveRefNode r = refStack.last();
      refStack.popBack();

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
    serializer->objects.clear();
    
    objStack.clear ();
    refStack.clear();
    objMap.clear();

    offset = 0;
    simulate = simulation;
  }

  void Serializer::StateLoad::run (Object **ppRoot)
  {
    //Start with root object
    objectPtr( ppRoot );

    //Process objects
    while (!objStack.empty())
    {
      //Pop object off the stack
      LoadObjNode o = objStack.last();
      objStack.popBack();

      //Load class ID
      UUID cid;
      load( &cid, sizeof( UUID ));

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
      Object *p = Serializer::Produce( cid );

      //Skip invalid object
      if (p == NULL) {
        skip( size );
        continue;
      }

      //Map object to serial ID
      objMap.resizeAndCopy( sid + 100 );
      objMap[ sid ] = p;

      //Record all the objects
      serializer->objects.pushBack( p );

      //Serialize object members
      p->serialize( serializer, version );
    }

    //Process references
    while (!refStack.empty())
    {
      //Pop reference off the stack
      LoadRefNode r = refStack.last();
      refStack.popBack();

      //Check for invalid serial ID
      if (r.id >= objMap.size()) {
        *r.pp = NULL;
        continue;
      }

      //Map serial ID to object
      *r.pp = objMap[ r.id ];
    }
  }

  /*
  ---------------------------------------------------------
  High-Level Control
  ---------------------------------------------------------*/

  void Serializer::serialize (Object *root, void **outData, UintSize *outSize)
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

  Object* Serializer::deserialize (const void *data)
  {
    //Enter loading state
    state = &stateLoad;
    state->serializer = this;
    state->buffer = (Uint8*) data;

    //Run
    Object *rootPtr = NULL;
    state->reset( false );
    state->run( &rootPtr );

    return rootPtr;
  }

}//namespace GE
