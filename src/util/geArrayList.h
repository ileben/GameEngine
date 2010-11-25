#ifndef __GEARRAYLIST_RES_H
#define __GEARRAYLIST_RES_H

namespace GE
{
  /*
  ===========================================================
  A list of structures of undefined type. The size of the
  elements must be passed as an argument to the constructor.
  ===========================================================*/

  class GenericArrayList
  { 
  protected:
    Uint32 sz;
    Uint32 cap;
    Uint32 eltSize;
    Uint8 *elements;
    
  public:
    
    /*
    ---------------------------------------------
    Default constructor. Assumes Uint8 elements.
    We shouldn't allow this really.
    ---------------------------------------------*/
    /*
    GenericArrayList ()
    {
      this->eltSize = sizeof( Uint8 );
      
      sz = 0;
      cap = 1;
      elements = (Uint8*) std::malloc( cap * eltSize );
    }
    */

    /*
    --------------------------------------
    Simple constructor. Initializes the
    array with storage for 1 element
    --------------------------------------*/
    
    GenericArrayList (UintSize eltSize)
    {
      this->eltSize = (Uint32) eltSize;
      
      sz = 0;
      cap = 1;
      elements = (Uint8*) std::malloc( cap * eltSize );
    }

    /*
    -----------------------------------
    Constructor to initialize the
    array with given capacity.
    -----------------------------------*/
    
    GenericArrayList (UintSize newCap, UintSize eltSize)
    {
      this->eltSize = (Uint32) eltSize;
      
      sz = 0;
      cap = (Uint32) (newCap > 0 ? newCap : 1);
      elements = (Uint8*) std::malloc( cap * eltSize );
    }
      
    /*
    -------------------------------------
    Dtor
    -------------------------------------*/
		
    virtual ~GenericArrayList ()
    {
      //Can't call virtuals in destructor!
      std::free( elements );
    }

  protected:
    
    /*
    -----------------------------------------------------
    Constructs n elements at given address
    -----------------------------------------------------*/
    
    virtual void construct (void *dst, UintSize n) {}
    
    /*
    -----------------------------------------------------
    Destructs n elements at given address
    -----------------------------------------------------*/
    
    virtual void destruct (void *dst, UintSize n) {}
    
    /*
    -----------------------------------------------------
    Copies n elements from source to destination buffer
    -----------------------------------------------------*/
    
    virtual void copy (void *dst, const void *src, UintSize n) {
      std::memcpy( dst, src, n * eltSize );
    }
  
  public:
    
    /*
    ---------------------------------------
    Invalidates all the items and resets
    array size to 0.
    ---------------------------------------*/
    
    void clear ()
    {
      destruct( elements, sz );
      sz = 0;
    }

    /*
    -----------------------------------------------
    Changes the size of the elements. All existing
    data becomes invalid and size of the array is
    reset to 0.
    -----------------------------------------------*/

    void resetElementSize (UintSize esz)
    {
      //Clear existing elements
      destruct( elements, sz );

      //Reset element info
      sz = 0;
      cap = 1;
      eltSize = (Uint32) esz;

      //Free old memory and allocate new
      std::free( elements );
      elements = (Uint8*) std::malloc( cap * eltSize );
    }
    
    /*
    ---------------------------------------------
    Assures that the capacity of the array
    is at least [n]. All the elements become
    invalid and size of the array is reset to 0
    unless the resize operation is requested.
    ---------------------------------------------*/

    void reserve (UintSize n)
    {
      //Clear existing elements
      destruct( elements, sz );
      sz = 0;

      //Check if enough capacity
      if (n > cap)
      {
        //Free old memory and allocate more
        std::free( elements );
        elements = (Uint8*) std::malloc( n * eltSize );
        cap = (Uint32) n;
      }
    }

    void resize (UintSize n)
    {
      if (n <= sz)
        return;

      //Check if enough capacity
      if (n > cap)
      {
        //Clear existing elements
        destruct( elements, sz );
        sz = 0;

        //Free old memory and allocate more
        std::free( elements );
        elements = (Uint8*) std::malloc( n * eltSize );
        cap = (Uint32) n;

        //Construct new elements
        construct( elements, sz );
      }

      //Construct additional elements
      construct( at( sz ), n-sz );
      sz = (Uint32) n;
    }

    /*
    ------------------------------------------
    Assures that the capacity of the array
    is at least [n]. In case of reallocation
    the existing elements are copied and size
    is always preserved.
    ------------------------------------------*/

    void reserveAndCopy (UintSize n)
    {
      //Check if enough capacity
      if (n > cap)
      {
        //Allocate and construct new elements,
        //copy existing elements
        void* newElements = std::malloc( n * eltSize );
        construct( newElements, sz );
        copy( newElements, elements, sz );
        
        //Destruct and free old elements
        destruct( elements, sz );
        std::free( elements );
        
        //Switch to new array
        elements = (Uint8*) newElements;
        cap = (Uint32) n;
      }
    }

    void resizeAndCopy (UintSize n)
    {
      if (n <= sz)
        return;

      //Check if enough capacity
      if (n > cap)
      {
        //Allocate and construct new elements,
        //copy existing elements
        void* newElements = std::malloc( n * eltSize );
        construct( newElements, sz );
        copy( newElements, elements, sz );
        
        //Destruct and free old elements
        destruct( elements, sz );
        std::free( elements );
        
        //Switch to new array
        elements = (Uint8*) newElements;
        cap = (Uint32) n;
      }

      //Construct additional elements
      construct( at( sz ), n-sz );
      sz = (Uint32) n;
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
      construct( at(sz), 1 );
      
      //Shift forward the elements above the index
      if( index < sz )
        for( UintSize i=sz-1; i>=index; --i )
          copy( at(i+1), at(i), 1 );
      
      //Copy the new element at [index]
      copy( at(index), copyElt, 1 );
      sz++;
      
      //Delete the clone
      if( cloned ){
        destruct( copyElt, 1 );
        std::free( copyElt );
      }
    }

    void insertAt( UintSize index )
    {
      //Clamp insertion point to size
      if( index > sz ) index = sz;

      //Make sure we got enough space
      if( sz == cap ) reserveAndCopy( cap * 2 );

      //Construct an element at the back
      construct( at(sz), 1 );

      //Shift forward the elements above the index
      if( index < sz )
        for( UintSize i=sz-1; i>=index; --i )
          copy( at(i+1), at(i), 1 );

      sz++;
    }
    
    void removeAt( UintSize index )
    {
      //Prevent invalid removal
      if( index >= sz ) return;
      
      //Shift backwards the elements above the index
      for( UintSize i=index; i<sz-1; i++ )
        copy( elements + i*eltSize, elements + (i+1)*eltSize, 1 );
      
      //Destruct the last element
      destruct( at( sz-1 ), 1 );
      sz--;
    }
    
    void setAt( UintSize index, const void *newElt )
    { copy( at( index ), newElt, 1 ); }
    
    void pushBack( const void *newElt )
    { insertAt( sz, newElt ); }

    void pushBack()
    { insertAt( sz ); }
    
    void popBack()
    { removeAt( sz-1 ); }

    void* at( UintSize index ) const
    { return elements + index * eltSize; }
    
    void* operator[]( UintSize index ) const
    { return elements + index * eltSize; }

    void* last() const
    { return elements + (sz-1) * eltSize; }
    
    UintSize capacity() const
    { return cap; }
    
    UintSize size() const
    { return sz; }
    
    bool empty() const
    { return sz == 0; }
    
    void* buffer() const
    { return elements; }
    
    UintSize elementSize() const
    { return eltSize; }

    void insertListAt( UintSize index, const GenericArrayList *list )
    {
      for (UintSize i=0; i<list->size(); ++i, ++index)
        insertAt( index, list->at( i ) );
    }

    void pushListBack( const GenericArrayList *list )
    {
      for (UintSize i=0; i<list->size(); ++i)
        pushBack( list->at( i ) );
    }
  };

  /*
  ======================================================
  Template array list
  ======================================================*/

  template <class T> class ArrayList : public GenericArrayList
  {
  public:
    
    ArrayList ()
      : GenericArrayList (sizeof(T)) {}

    ArrayList (UintSize newCap)
      : GenericArrayList (newCap, sizeof(T)) {}
    
    T& first() const
    { return ((T*)elements)[ 0 ]; }
    
    T& last() const
    { return ((T*)elements)[ sz-1 ]; }
    
    T& elementAt( UintSize index ) const
    { return ((T*)elements)[ index ]; }
    
    T& at( UintSize index ) const
    { return ((T*)elements)[ index ]; }
    
    T& operator[]( UintSize index ) const
    { return ((T*)elements)[ index ]; }
    
    T* buffer() const
    { return (T*)elements; }

    void insertAt( UintSize index, const T &newElt )
    { GenericArrayList::insertAt( index, &newElt ); }
    
    void pushBack( const T &newElt )
    { GenericArrayList::pushBack( &newElt ); }
    
    void setAt( UintSize index, const T &newElt )
    { GenericArrayList::setAt( index, &newElt ); }

    bool contains( const T &el ) const
    { return (indexOf( el ) != -1); }
    
    int indexOf( const T &el ) const
    {
      for (UintSize i=0; i<sz; i++)
        if (((T*)elements)[i] == el)
          return (int) i;
      
      return -1;
    }
    
    void remove( const T &el )
    {
      int i = indexOf( el );
      if (i > -1) removeAt( (UintSize) i );
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
  };

  
}//namespace GE
#endif //__GEARRAYLIST_RES_H
