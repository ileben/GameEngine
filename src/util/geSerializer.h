#ifndef __GESERIALIZER_H
#define __GESERIALIZER_H 1

namespace GE
{
  class Serializer
  {
  private:

    struct SaveObjNode
    {
      MyObject *p;
      UintSize offset;
      UintSize szOffset;
      bool done;
    };

    struct SaveRefNode
    {
      MyObject *p;
      UintSize offset;
    };

    struct LoadObjNode
    {
    };

    struct LoadRefNode
    {
      MyObject **pp;
      UintSize id;
    };

    class State
    {
    public:
      Serializer *serializer;
      Uint8 *buffer;
      UintSize offset;
      bool simulate;
      ArrayList< MyObject* > objects;

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

      virtual void objectPtr (MyObject **pp) {}
      virtual void objectRef (MyObject **pp) {}
      virtual void objectPtrArray (GenericArrayList *a) {}
      virtual void objectRefArray (GenericArrayList *a) {}

      virtual void reset (bool simulation) {}
      virtual void run (MyObject **ppRoot) {}
    };

    class StateSave : public State
    {
    public:
      ArrayList< SaveObjNode > objs;
      ArrayList< SaveRefNode > refs;

      virtual void data (void *p, UintSize size);
      virtual void dataPtr (void **pp, UintSize *size);
      virtual void dataArray (GenericArrayList *a);

      virtual void string (CharString *s);
      virtual void stringArray (ArrayList< CharString > *a);

      virtual void objectPtr (MyObject **pp);
      virtual void objectRef (MyObject **pp);
      virtual void objectPtrArray (GenericArrayList *a);
      virtual void objectRefArray (GenericArrayList *a);

      virtual void reset (bool simulation);
      virtual void run (MyObject **ppRoot);
    };

    class StateLoad : public State
    {
    public:
      ArrayList< LoadObjNode > objs;
      ArrayList< LoadRefNode > refs;

      virtual void data (void *p, UintSize size);
      virtual void dataPtr (void **pp, UintSize *size);
      virtual void dataArray (GenericArrayList *a);

      virtual void string (CharString *s);
      virtual void stringArray (ArrayList< CharString > *a);

      virtual void objectPtr (MyObject **pp);
      virtual void objectRef (MyObject **pp);
      virtual void objectPtrArray (GenericArrayList *a);
      virtual void objectRefArray (GenericArrayList *a);

      virtual void reset (bool simulation);
      virtual void run (MyObject **ppRoot);
    };

    StateSave stateSave;
    StateLoad stateLoad;
    State *state;

  public:

    void data (void *p, UintSize size);
    void dataPtr (void **pp, UintSize *size);
    void dataArray (GenericArrayList *a);

    void string (CharString *s);
    void stringArray (ArrayList< CharString > *a);

    void objectPtr (MyObject **pp);
    void objectRef (MyObject **pp);
    void objectPtrArray (GenericArrayList *a);
    void objectRefArray (GenericArrayList *a);

    void serialize (MyObject *root, void **outData, UintSize *outSize);
    MyObject* deserialize (const void *data);

    template <class T>
    void data (const T *p) {
      data( (void*) p, sizeof(T) ); }

    template <class T>
    void objectPtr (T **pp) {
      objectPtr( (MyObject**) pp ); }

    template <class T>
    void objectRef (T **pp) {
      objectRef( (MyObject**) pp ); }

  private:

    //Maps object UUID to factory
    static std::map< ClassID, BaseFactory* > factories;

  public:

    //Register object by UUID for production
    template <class C> static void Register () {
      factories[ C::Uuid() ] = new Factory<C>;
    }

    //Produce object matching given UUID
    static MyObject* Produce (const ClassID &id) {
      return factories[ id ]->produce();
    }
  };

}//namespace GE
#endif//__GESERIALIZER_H
