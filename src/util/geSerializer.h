#ifndef __GESERIALIZER_H
#define __GESERIALIZER_H 1

namespace GE
{
  class Serializer
  {
    friend class State;
    friend class SaveState;
    friend class LoadState;

  private:

    struct SaveObjNode
    {
      Object *p;
      UintSize offset;
      UintSize szOffset;
      bool done;
    };

    struct SaveRefNode
    {
      Object *p;
      UintSize offset;
    };

    struct LoadObjNode
    {
    };

    struct LoadRefNode
    {
      Object **pp;
      UintSize id;
    };

    class State
    {
    public:
      Serializer *serializer;
      Uint8 *buffer;
      UintSize offset;
      bool simulate;
      ArrayList< Object* > objMap;

      void skip (UintSize size);
      void store (const void *from, UintSize to, UintSize size);
      void store (const void *from, UintSize size);
      void load (void *to, UintSize size);
      void* pointer();

      virtual void data (void *p, UintSize size) {}
      virtual void dataPtr (void **pp, UintSize *size) {}
      virtual void dataArray (GenericArrayList *a) {}

      virtual void string (CharString *s) {}
      virtual void stringArray (ArrayList< CharString > *a) {}

      virtual void object (Object *p) {}
      virtual void objectPtr (Object **pp) {}
      virtual void objectRef (Object **pp) {}
      virtual void objectArray (GenericArrayList *a) {}
      virtual void objectPtrArray (GenericArrayList *a) {}
      virtual void objectRefArray (GenericArrayList *a) {}

      virtual void reset (bool simulation) {}
      virtual void run (Object **ppRoot) {}
    };

    class StateSave : public State
    {
    public:
      ArrayList< SaveObjNode > objStack;
      ArrayList< SaveRefNode > refStack;

      virtual void data (void *p, UintSize size);
      virtual void dataPtr (void **pp, UintSize *size);
      virtual void dataArray (GenericArrayList *a);

      virtual void string (CharString *s);
      virtual void stringArray (ArrayList< CharString > *a);

      virtual void object (Object *p);
      virtual void objectPtr (Object **pp);
      virtual void objectRef (Object **pp);
      virtual void objectArray (GenericArrayList *a);
      virtual void objectPtrArray (GenericArrayList *a);
      virtual void objectRefArray (GenericArrayList *a);

      virtual void reset (bool simulation);
      virtual void run (Object **ppRoot);
    };

    class StateLoad : public State
    {
    public:
      ArrayList< LoadObjNode > objStack;
      ArrayList< LoadRefNode > refStack;

      virtual void data (void *p, UintSize size);
      virtual void dataPtr (void **pp, UintSize *size);
      virtual void dataArray (GenericArrayList *a);

      virtual void string (CharString *s);
      virtual void stringArray (ArrayList< CharString > *a);

      virtual void object (Object *p);
      virtual void objectPtr (Object **pp);
      virtual void objectRef (Object **pp);
      virtual void objectArray (GenericArrayList *a);
      virtual void objectPtrArray (GenericArrayList *a);
      virtual void objectRefArray (GenericArrayList *a);

      virtual void reset (bool simulation);
      virtual void run (Object **ppRoot);
    };

    //State
    StateSave stateSave;
    StateLoad stateLoad;
    State *state;

    //Statistics
    ArrayList< Object* > objects;

  public:

    void data (void *p, UintSize size);
    void dataPtr (void **pp, UintSize *size);
    void dataArray (GenericArrayList *a);

    void string (CharString *s);
    void stringArray (ArrayList< CharString > *a);

    void object (Object *p);
    void objectPtr (Object **pp);
    void objectRef (Object **pp);
    void objectArray (GenericArrayList *a);
    void objectPtrArray (GenericArrayList *a);
    void objectRefArray (GenericArrayList *a);

    void serialize (Object *root, void **outData, UintSize *outSize);
    Object* deserialize (const void *data);

    template <class T>
    void data (const T *p) {
      data( (void*) p, sizeof(T) ); }

    template <class T>
    void objectPtr (T **pp) {
      objectPtr( (Object**) pp ); }

    template <class T>
    void objectRef (T **pp) {
      objectRef( (Object**) pp ); }

  private:

    //Maps UUID to class
    typedef std::map< UUID, IFactory* > ClassMap;
    typedef ClassMap::iterator ClassMapIter;
    static ClassMap classes;

  public:

    //Register factory by UUID for instantiation
    template <class C> static void Register ()
    {
      Class c = C::GetClass();
      classes[ c->uuid() ] = new Factory<C>;
    }

    //Produce object matching given UUID
    static Object* Produce (const UUID &id)
    {
      ClassMapIter i = classes.find( id );
      if (i == classes.end()) return NULL;
      return i->second->produce();
    }
  };

}//namespace GE
#endif//__GESERIALIZER_H
