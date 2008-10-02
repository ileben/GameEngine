#define GE_API_EXPORT
#include "geEngine.h"
using namespace OCC;
using namespace OCC::TextParserCommon;

namespace GE
{
  CTable* IClass::classes = NULL;
  ITable* IClass::classesByID = NULL;
  
  IClass::IClass ()
  {
  }

  const ClassID& IClass::getID ()
  {
    return id;
  }

  const char* IClass::getString ()
  {
    return name.buffer();
  }
  
  PTable* IClass::getProperties ()
  {
    return &properties;
  }
  
  void IClass::Classify (ClassPtr cls)
  {
    //Make sure CTable is initialised
    if (IClass::classes == NULL)
      classes = new CTable;
    
    //Check that the name is not registered yet
    const char *cname = cls->name.buffer ();
    CTable::iterator it = IClass::classes->find (cname);
    ASSERT (it == IClass::classes->end());
    
    //Map name string to class pointer
    (*IClass::classes) [cname] = cls;
    
    if (cls->id != 0)
    {
      //Make sure ITable is initialised
      if (IClass::classesByID == NULL)
        classesByID = new ITable;

      //Check that the id is not registered yet
      ITable::iterator it = IClass::classesByID->find (cls->id);
      ASSERT (it == IClass::classesByID->end());
      
      //Map id to class pointer
      (*IClass::classesByID) [cls->id] = cls;
    }
  }
  
  ClassPtr IClass::FromString (const char *name)
  {
    //Make sure CTable is initialised
    if (IClass::classes == NULL)
      classes = new CTable;
    
    //Find a class descriptor matching the string name
    CTable::iterator it = IClass::classes->find (name);
    if (it == IClass::classes->end()) return NULL;
    return it->second;
  }

  ClassPtr IClass::FromID (ClassID id)
  {
    //Make sure ITable is initialised
    if (IClass::classesByID == NULL)
      classesByID = new ITable;
    
    //Find a class descriptor matching the id
    ITable::iterator it = IClass::classesByID->find (id);
    if (it == IClass::classesByID->end()) return NULL;
    return it->second;
  }

  void*
  IClass::Safecast (ClassPtr to, ClassPtr from, void *instance)
  {
    ClassPtr super = from;
    
    //Return early if same class
    if (to == from)
      return instance;
    
    do
    {
      //Move one level higher
      from = super;
      super = SuperOf (from);
      
      //Check whether [to] is [from]'s superclass
      if (super == to) return instance;
    }
    //Stop if we hit the base
    while (super != from);
    
    //Cannot cast to non-super class
    return NULL;
  }
  
  void
  IClass::SaveText (const ObjectPtr &ptr, OCC::ByteString &buf)
  {
    PTable *props = ptr.cls->getProperties ();
    
    for (int p=0; p<props->size(); ++p)
    {
      buf += props->at(p)->getName();
      buf += " = ";
      props->at(p)->saveText (ptr.obj, buf);
      buf += "\n";
    }
  }
  
  void
  IClass::SaveBinary (const ObjectPtr &ptr, OCC::ByteString &buf)
  {
    PTable *props = ptr.cls->getProperties ();
    
    for (int p=0; p<props->size(); ++p)
    {
      props->at(p)->saveBinary (ptr.obj, buf);
    }
  }
  
  typedef TextParser <ByteString> ByteParser;
  
  int
  IClass::LoadText (const ObjectPtr &ptr, const OCC::ByteString &buf, int index)
  {
    ByteParser parser;
    parser.begin (&buf, index);
    
    while (! parser.end ())
    {
      //Find next non-whitespace character
      parser.jump <SNR> ();
      if (parser.end ()) return parser.getIndex ();
      
      //Mark token start
      parser.beginToken ();
      
      //Find next character not part of a var name
      parser.jump <Var> ();
      if (parser.end ()) return parser.getIndex();

      //Property name is inbetween
      parser.endToken ();
      
      //Jump whitespace, check for '='
      parser.jump <SNR> ();
      if (parser.at() == '=')
      {
        //Skip '=', jump whitespace
        parser.fw (1);
        parser.jump <SNR> ();
        
        ByteString propName = parser.getToken ();
        printf ("Loading var: '%s'\n", propName.buffer());
        
        //Search for the property with same name
        PTable *props = ptr.cls->getProperties();
        for (int p=0; p < props->size(); ++p) {
          if (parser.compareToken (props->at(p)->getName())) {
            
            //Let the property load itself
            printf ("Found property '%s'\n", propName.buffer());
            int propEnd = props->at(p)->loadText (ptr.obj, buf, parser.getIndex());
            parser.fw (propEnd - parser.getIndex());
            break; }}
      }
      
      //Find next semicolon
      parser.jump <Char<';'> > (false);
      parser.fw (1);
    }
    
    return index;
  }
  
  int
  IClass::LoadBinary (const ObjectPtr &ptr, const OCC::ByteString &buf, int index)
  {
    return index;
  }
  
  void
  IClass::Create (const ObjectPtr &ptr, void *data, int size)
  {
    ptr.cls->invokeCallback (CLSEVT_CREATE, ptr.obj, data);
  }
  
}//namespace GE
