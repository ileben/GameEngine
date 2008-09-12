
class Property
{
  virtual void *getAddress (void *obj) = 0;
  virtual void *saveText ();
};
  
class IntProperty : public Property
{
  void saveText () { int *val = (int*)getAddress(); /* ... */ }
};
  
typedef HashTable <CharString, Property*> PropTable;

class MyClass
{   
  static PropTable* GetPropTable ()
  {
    static PropTable pt;
    return &pt;
  }
  
  static Property* GetProperty (const char *name)
  {
    return MyClass::GetPropTable () ->get (name);
  }
  
  static void SetProperty (const char *name, Property *p)
  {
    MyClass::GetPropTable () ->set (name, p);
  }
  
  /*
  getAddress() obtains the address of the value based
  on the address of the object instance.
  */
  
  class Property_a : public IntProperty //Per-property
  {
    virtual void* getAddress (void *obj)
    {
      return & ((MyClass*) obj) -> a;
    }
  };
  
  /*
  The BIG QUESTION is: how to list something in a class
  outside any function (like a list of saveable properties)
  and then have a function iterate over all the items
  listed before??? The goal is to get rid of InitProperties()
  and only have one macro required for each property (also get
  rid of begin / end macros to insert the opening / closing
  brackets)
  */
  
  static void InitProperties () //Per-class
  {
    SetProperty ("a", new Property_a); //Per-property
    SetProperty ("b", new Property_b);
  }
  
  void SaveMe ()
  {
    MyClass::GetPropTable ();
    
    for (PropTable::Iter a(pt); !a.end(); ++a)
    {
      a->saveText ();
    }
  }
};

int main (int argc, char **argv)
{
  InitProperties (MyClass);
}

//MACROED-UP EXAMPLE
///////////////////////////////////

class MyClass
{
  RegisterClass (MyClass, MyBaseClass);
  RegisterProperty (a);
  RegisterProperty (b);
}

int main (int argc, char **argv)
{
  InitProperty (MyClass, a);
  InitProperty (MyClass, b);
}
