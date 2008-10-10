#ifndef __GESAVEOBJ_H
#define __GESAVEOBJ_H
#pragma warning(push)
#pragma warning(disable:4251)

namespace GE
{
  class GE_API_ENTRY Saver
  {
    DECLARE_ABSTRACT (Saver); DECLARE_END;

  protected:
    ArrayList<Object*> objects;

  public:
    void addObject(Object *o);
    virtual bool saveFile(const String &filename) = 0;
  };
    
  class GE_API_ENTRY SaverObj : public Saver
  {
    DECLARE_SUBCLASS (SaverObj, Saver); DECLARE_END;

    File file;

  private:
    void writeDynMesh (PolyMesh *mesh, TexMesh *umesh);
    void writeShape (PolyMeshActor *shape);

  public:
    bool saveFile(const String &filename);
  };

}/* namespace GE */
#pragma warning(pop)
#endif /* __GESAVEOBJ_H */
