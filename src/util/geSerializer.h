#ifndef __GESERIALIZER_H
#define __GESERIALIZER_H 1

namespace GE
{
  class Serializer
  {
  private:

    struct SaveObjNode
    {
      Object *p;
      UintSize idOffset;
      UintSize szOffset;
      UintSize offset;
      bool done;
    };

    struct SaveRefNode
    {
      Object *p;
      UintSize offset;
    };

    struct LoadObjNode
    {
      Object *p;
      UintSize id;
    };

    struct LoadRefNode
    {
      Object **pp;
      UintSize id;
    };

    class State
    {
    public:
      Uint8 *buffer;
      UintSize offset;
      bool simulate;
      ArrayList< Object* > objects;

      void skip (UintSize size);
      void store (void *from, UintSize to, UintSize size);
      void store (void *from, UintSize size);
      void load (void *to, UintSize size);
      void* pointer();

      virtual void data (void *p, UintSize size) {}
      virtual void dataPtr (void **pp, UintSize *size) {}
      virtual void dataArray (GenericArrayList *a) {}

      virtual void objectPtr (Object **pp) {}
      virtual void objectRef (Object **pp) {}
      virtual void objectPtrArray (GenericArrayList *a) {}
      virtual void objectRefArray (GenericArrayList *a) {}

      virtual void reset (bool simulation) {}
      virtual void run (Object **ppRoot) {}
    };

    class StateSave : public State
    {
    public:
      ArrayList< SaveObjNode > objs;
      ArrayList< SaveRefNode > refs;

      virtual void data (void *p, UintSize size);
      virtual void dataPtr (void **pp, UintSize *size);
      virtual void dataArray (GenericArrayList *a);

      virtual void objectPtr (Object **pp);
      virtual void objectRef (Object **pp);
      virtual void objectPtrArray (GenericArrayList *a);
      virtual void objectRefArray (GenericArrayList *a);

      virtual void reset (bool simulation);
      virtual void run (Object **ppRoot);
    };

    class StateLoad : public State
    {
    public:
      ArrayList< LoadObjNode > objs;
      ArrayList< LoadRefNode > refs;

      virtual void data (void *p, UintSize size);
      virtual void dataPtr (void **pp, UintSize *size);
      virtual void dataArray (GenericArrayList *a);

      virtual void objectPtr (Object **pp);
      virtual void objectRef (Object **pp);
      virtual void objectPtrArray (GenericArrayList *a);
      virtual void objectRefArray (GenericArrayList *a);

      virtual void reset (bool simulation);
      virtual void run (Object **ppRoot);
    };

    StateSave stateSave;
    StateLoad stateLoad;
    State *state;

  public:
    void data (void *p, UintSize size);
    void dataPtr (void **pp, UintSize *size);
    void dataArray (GenericArrayList *a);

    void objectPtr (Object **pp);
    void objectRef (Object **pp);
    void objectPtrArray (GenericArrayList *a);
    void objectRefArray (GenericArrayList *a);

    void serialize (Object *root, void **outData, UintSize *outSize);
    Object* deserialize (const void *data);
  };

}//namespace GE
#endif//__GESERIALIZER_H
