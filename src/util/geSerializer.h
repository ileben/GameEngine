#ifndef __GESERIALIZER_H
#define __GESERIALIZER_H 1

namespace GE
{
  /////////////////////////////////////////////////////////////////

  class Serializer;

  class MyObject
  {
    friend class Serializer;

  private:
    UintSize serialID;

  public:
    virtual ClassID uuid () = 0;
    virtual Uint version () { return 1; }
    virtual void serialize (Serializer *serializer, Uint version) = 0;
  };

  class BaseFactory
  {
  public:
    virtual MyObject* produce() = 0;
  };

  template <class C>
  class Factory : public BaseFactory
  {
  public:
    virtual MyObject* produce() { return new C(); }
  };

  class ClassFactory
  {
    static std::map< ClassID, BaseFactory* > factories;

  public:

    template <class C> static void Register ()
    {
      factories[ C::Uuid() ] = new Factory<C>;
    }

    static MyObject* Produce (const ClassID &id)
    {
      return factories[ id ]->produce();
    }
  };

  /////////////////////////////////////////////////////////////////

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
      void store (void *from, UintSize to, UintSize size);
      void store (void *from, UintSize size);
      void load (void *to, UintSize size);
      void* pointer();

      virtual void data (void *p, UintSize size) {}
      virtual void dataPtr (void **pp, UintSize *size) {}
      virtual void dataArray (GenericArrayList *a) {}

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

    void objectPtr (MyObject **pp);
    void objectRef (MyObject **pp);
    void objectPtrArray (GenericArrayList *a);
    void objectRefArray (GenericArrayList *a);

    void serialize (MyObject *root, void **outData, UintSize *outSize);
    MyObject* deserialize (const void *data);
  };

}//namespace GE
#endif//__GESERIALIZER_H
