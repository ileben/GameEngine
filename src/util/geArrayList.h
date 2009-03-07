#ifndef __GEARRAYLIST_RES_H
#define __GEARRAYLIST_RES_H

namespace GE
{
  /*
  =======================================================
  A serializable list of structures of undefined class.
  The size of the elements must be passed as an argument
  to the constructor.
  =======================================================*/
  
  class GE_API_ENTRY GenericArrayList
  {
    DECLARE_SERIAL_CLASS( GenericArrayList );
    DECLARE_CALLBACK( ClassEvent::Serialize, serialize );
    DECLARE_END;
    
  protected:
    Uint32 sz;
    Uint32 cap;
    Uint32 eltSize;
    ClassID eltClsID;
    ClassPtr eltCls;
    Uint8 *elements;
    
  public:
    
    /*
    ------------------------------------------------------
    Serialization
    ------------------------------------------------------*/
    
    GenericArrayList (SerializeManager *sm) : eltClsID (sm)
    {
      //Make sure it is re-serializable
      if (sm->isDeserializing())
        eltCls = IClass::FromID( eltClsID );
    }
    
    virtual void serialize (void *param)
    {
      SerializeManager *sm = (SM*)param;
      sm->dataVar( &sz );
      sm->dataVar( &eltSize );
      sm->dataVar( &eltClsID );
      
      //Make sure it is modifiable after loading
      //(deserialized is non-modifiable so cap and class don't matter)
      if (sm->isLoading()) {
        eltCls = IClass::FromID( eltClsID );
        cap = sz;
      }
      
      //Assume if class given the elements are pointers
      //(otherwise the array is not serializable anyway)
      if (sz > 0) {
        if (eltCls != NULL)
          sm->objectArray( eltCls, (void***)&elements, sz );
        else sm->dataPtr( &elements, sz * eltSize );
      }
      
      //Make sure it is modifiable after loading
      if (sm->isLoading() && sz == 0) {
        elements = (Uint8*) std::malloc( eltSize );
        cap = 1;
      }
    }
    
  protected:
    
    /*
    -----------------------------------------------------
    Constructs n elements at given address
    -----------------------------------------------------*/
    
    virtual void construct( void *dst, UintSize n ){
      //Implemented in ArrayList<T>
    }
    
    /*
    -----------------------------------------------------
    Destructs n elements at given address
    -----------------------------------------------------*/
    
    virtual void destruct( void *dst, UintSize n ){
      //Implemented in ArrayList<T>
    }
    
    /*
    -----------------------------------------------------
    Copies n elements from source to destination buffer
    -----------------------------------------------------*/
    
    virtual void copy( void *dst, const void *src, UintSize n ){
      std::memcpy( dst, src, n * eltSize );
    }
    
  public:
    
    /*
    ---------------------------------------------
    Default constructor. Assumes Uint8 elements.
    We shouldn't allow this really.
    ---------------------------------------------*/
    
    GenericArrayList()
    {
      this->eltSize = sizeof( Uint8 );
      this->eltClsID = ClassID();
      this->eltCls = NULL;
      
      sz = 0;
      cap = 1;
      elements = (Uint8*) std::malloc( cap * eltSize );
    }
    
    /*
    --------------------------------------
    Simple constructor. Initializes the
    array with storage for 1 element
    --------------------------------------*/
    
    GenericArrayList( UintSize eltSize, ClassPtr eltCls )
    {
      this->eltSize = (Uint32) eltSize;
      this->eltClsID = ( eltCls ? eltCls->getID() : ClassID() );
      this->eltCls = eltCls;
      
      sz = 0;
      cap = 1;
      elements = (Uint8*) std::malloc( cap * eltSize );
    }

    /*
    -----------------------------------
    Constructor to initialize the
    array with given capacity.
    -----------------------------------*/
    
    GenericArrayList( UintSize newCap, UintSize eltSize, ClassPtr eltCls )
    {
      this->eltSize = (Uint32) eltSize;
      this->eltClsID = ( eltCls ? eltCls->getID() : ClassID() );
      this->eltCls = eltCls;
      
      sz = 0;
      cap = (Uint32) (newCap > 0 ? newCap : 1);
      elements = (Uint8*) std::malloc( cap * eltSize );
    }
      
    /*
    -------------------------------------
    Dtor
    -------------------------------------*/
		
    virtual ~GenericArrayList()
    {
      //Can't call virtuals in destructor!
      //Implemented in ArrayList<T>.
      //destruct( elements, sz );
      std::free( elements );
    }
    
    /*
    ---------------------------------------
    Invalidates all the items and resets
    array size to 0.
    ---------------------------------------*/
    
    void clear()
    {
      destruct( elements, sz );
      sz = 0;
    }
      
    /*
    ---------------------------------------------
    Assures that the capacity of the array
    is at least [n]. All the elements become
    invalid and size of the array is reset to 0.
    ---------------------------------------------*/

    void reserve( UintSize n )
    {
      //Clear existing elements
      destruct( elements, sz );
      sz = 0;
      
      //Return if enough capacity
      if( n <= cap ) return;
      
      //Free old memory and allocate more
      std::free( elements );
      elements = (Uint8*) std::malloc( n * eltSize );
      cap = (Uint32) n;
    }

    /*
    ------------------------------------------
    Assures that the capacity of the array
    is at least [n]. In case of reallocation
    the existing elements are copied and size
    is always preserved.
    ------------------------------------------*/

    void reserveAndCopy( UintSize n )
    {
      //No-op if enough capacity
      if( n <= cap ) return;
      
      //Allocate array for new elements,
      //construct and copy existing ones
      void* newElements = std::malloc( n * eltSize );
      construct( newElements, sz );
      copy( newElements, elements, sz );
      
      //Destruct and free old elements
      destruct( elements, sz );
      std::free( elements );
      
      elements = (Uint8*) newElements;
      cap = (Uint32) n;
    }
    
    /*
    ------------------------------------------------------
    Enlarges the array capacity at element insertion by
    the factor of 2. This yields an ammortized complexity
    of O(1) for element insertion.
    ------------------------------------------------------*/
    
    void insertAt( UintSize index, const void *newElt )
    {
      void *copyElt = (void*) newElt;      
      bool cloned = false;
      
      //Clamp insertion point to size
      if( index > sz ) index = sz;
      
      //Will any moving take place?
      if( sz == cap || index < sz )
      {
        //It might be a reference to our own element
        //so better clone it before reallocation
        copyElt = std::malloc( eltSize );
        construct( copyElt, 1 );
        copy( copyElt, newElt, 1 );
        cloned = true;
      }
      
      //Make sure we got enough space
      if( sz == cap ) reserveAndCopy( cap * 2 );
      
      //Construct an element at the back
      construct( elements + sz*eltSize, 1 );
      
      //Shift forward the elements above the index
      if( index < sz )
        for( UintSize i=sz-1; i>=index; --i )
          copy( elements + (i+1)*eltSize, elements + i*eltSize, 1 );
      
      //Copy the new element at [index]
      copy( elements + index*eltSize, copyElt, 1 );
      sz++;
      
      //Delete the clone
      if( cloned ){
        destruct( copyElt, 1 );
        std::free( copyElt );
      }
    }
    
    void removeAt( UintSize index )
    {
      //Prevent invalid removal
      if( index >= sz ) return;
      
      //Shift backwards the elements above the index
      for( UintSize i=index; i<sz-1; i++ )
        copy( elements + i*eltSize, elements + (i+1)*eltSize, 1 );
      
      //Destruct the last element
      destruct( elements + sz*eltSize, 1 );
      sz--;
    }
    
    void setAt( UintSize index, const void *newElt )
    {
      copy( elements + index, newElt, 1 );
    }
    
    void pushBack( const void *newElt )
    {
      insertAt( sz, newElt );
    }
    
    void popBack()
    {
      removeAt( sz-1 );
    }

    void* at( UintSize index ) const
    {
      return elements + index * eltSize;
    }
    
    void* operator[]( UintSize index ) const
    {
      return elements + index * eltSize;
    }
    
    UintSize capacity() const
    {
      return cap;
    }
    
    UintSize size() const
    {
      return sz;
    }
    
    bool empty() const
    {
      return sz == 0;
    }
    
    void* buffer() const
    {
      return elements;
    }
    
    UintSize elementSize() const
    {
      return eltSize;
    }
  };
  
  /*
  ======================================================
  A non-serializable templated list
  ======================================================*/
  
  template <class T> class ArrayList : public GenericArrayList
  {
  public:

    ArrayList (SerializeManager *sm)
      : GenericArrayList (sm)
      {}
    
    ArrayList()
      : GenericArrayList( sizeof(T), NULL )
      {}
    
    ArrayList( UintSize newCap )
      : GenericArrayList( newCap, sizeof(T), NULL )
      {}
    
    ArrayList( UintSize eltSize, ClassPtr eltCls )
      : GenericArrayList( eltSize, eltCls )
      {}
    
    ArrayList( UintSize newCap, UintSize eltSize, ClassPtr eltCls )
      : GenericArrayList( newCap, eltSize, eltCls )
      {}
    
    ~ArrayList()
    {
      destruct( this->elements, this->sz );
    }
    
    virtual void construct( void *dst, UintSize n )
    {
      T *tdst = (T*)dst;
      for( UintSize i=0; i<n; ++i )
        new( &tdst[i] )T;
    }
    
    virtual void destruct( void *dst, UintSize n )
    {
      T *tdst = (T*)dst;
      for( UintSize i=0; i<n; ++i )
        tdst[i].~T();
    }
    
    virtual void copy( void *dst, const void *src, UintSize n )
    {
      T *tdst = (T*)dst;
      T *tsrc = (T*)src;
      for( UintSize i=0; i<n; ++i )
        tdst[i] = tsrc[i];
    }

    void insertAt( UintSize index, const T &newElt )
    {
      GenericArrayList::insertAt( index, &newElt );
    }
    
    void pushBack( const T &newElt )
    {
      GenericArrayList::pushBack( &newElt );
    }
    
    void setAt( UintSize index, const T &newElt )
    {
      GenericArrayList::setAt( index, &newElt );
    }
    
    T& first() const
    {
      return ((T*)elements)[ 0 ];
    }
    
    T& last() const
    {
      return ((T*)elements)[ sz-1 ];
    }
    
    T& elementAt( UintSize index ) const
    {
      return ((T*)elements)[ index ];
    }
    
    T& at( UintSize index ) const
    {
      return ((T*)elements)[ index ];
    }
    
    T& operator[]( UintSize index ) const
    {
      return ((T*)elements)[ index ];
    }
    
    T* buffer() const
    {
      return (T*)elements;
    }
    
    bool contains( const T &el ) const
    {
      return (indexOf( el ) != -1);
    }
    
    UintSize indexOf( const T &el ) const
    {
      for( UintSize i=0; i<sz; i++ )
        if( ((T*)elements) [i] == el )
          return i;
      
      return -1;
    }
    
    void remove( const T &el )
    {
      UintSize i = indexOf( el );
      if( i > -1 ) removeAt( i );
    }
  };

  
  /*
  ========================================================
  A serializable list of pointers to serializable classes
  ========================================================*/
  
  template <class T>
    class ClassArrayList : public ArrayList <T*>
  {
  public:
    
    ClassArrayList (SerializeManager *sm)
      : ArrayList <T*> (sm)
     {}

    ClassArrayList()
      : ArrayList <T*> ( sizeof(T*), Class(T) )
      {}
    
    ClassArrayList( UintSize newCap )
      : ArrayList <T*> ( newCap, sizeof(T*), Class(T) )
      {}
  };
  
  
}//namespace GE
#endif //__GEARRAYLIST_RES_H
