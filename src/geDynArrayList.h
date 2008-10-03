#ifndef __GEARRAYLIST_RES_H
#define __GEARRAYLIST_RES_H

namespace GE
{
  class GE_API_ENTRY GenArrayList
  {
    DECLARE_SERIAL_CLASS (GenArrayList);
    DECLARE_CALLBACK (CLSEVT_SERIALIZE, serialize);
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
    
    GenArrayList (SerializeManager *sm)
    {
      //Make sure it is re-serializable
      if (sm->isDeserializing())
        eltCls = IClass::FromID (eltClsID);
    }
    
    virtual void serialize (void *param)
    {
      SerializeManager *sm = (SM*)param;
      sm->memberData (&sz, sizeof (sz));
      sm->memberData (&eltSize, sizeof (eltSize));
      sm->memberData (&eltClsID, sizeof (eltClsID));
      
      //Make sure it is usable after loading
      if (sm->isLoading()) {
        eltCls = IClass::FromID (eltClsID);
        cap = sz;
      }
      
      //Assume if class given the elements are pointers
      //(otherwise the array is not serializable anyway)
      if (sz > 0) {
        if (eltCls != NULL)
          sm->resourcePtrPtr (eltCls, (void***)&elements, sz);
        else sm->dynamicPtr (&elements, sz * eltSize);
      }
      
      //Make sure it is usable after loading
      if (sm->isLoading() && sz == 0) {
        elements = (Uint8*) std::malloc (eltSize);
        cap = 1;
      }
    }
    
  protected:
    
    /*
    -----------------------------------------------------
    Constructs n elements at given address
    -----------------------------------------------------*/
    
    virtual void construct (void *dst, UintP n) {
      //Implemented in ResArrayList
    }
    
    /*
    -----------------------------------------------------
    Destructs n elements at given address
    -----------------------------------------------------*/
    
    virtual void destruct (void *dst, UintP n) {
      //Implemented in ResArrayList
    }
    
    /*
    -----------------------------------------------------
    Copies n elements from source to destination buffer
    -----------------------------------------------------*/
    
    virtual void copy (void *dst, const void *src, UintP n) {
      std::memcpy (dst, src, n * eltSize);
    }
    
  public:
    
    /*
    ---------------------------------------------
    Default constructor. Assumes Uint8 elements.
    We should let this really.
    ---------------------------------------------*/

    GenArrayList ()
    {
      this->eltSize = sizeof (Uint8);
      this->eltClsID = ClassID(0);
      this->eltCls = NULL;
      
      sz = 0;
      cap = 1;
      elements = (Uint8*) std::malloc (cap * eltSize);
    }
    
    /*
    --------------------------------------
    Simple constructor. Initializes the
    array with storage for 1 element
    --------------------------------------*/
    
		GenArrayList (UintP eltSize, ClassPtr eltCls)
		{
      this->eltSize = (Uint32) eltSize;
      this->eltClsID = (eltCls ? eltCls->getID () : ClassID(0));
      this->eltCls = eltCls;
      
			sz = 0;
			cap = 1;
      elements = (Uint8*) std::malloc (cap * eltSize);
		}

    /*
    -----------------------------------
    Constructor to initialize the
    array with given capacity.
    -----------------------------------*/
		
		GenArrayList (UintP eltSize, ClassPtr eltCls, UintP newCap)
		{
      this->eltSize = (Uint32) eltSize;
      this->eltClsID = (eltCls ? eltCls->getID () : ClassID(0));
      this->eltCls = eltCls;
      
			sz = 0;
			cap = (Uint32) newCap;
      elements = (Uint8*) std::malloc (cap * eltSize);
		}

    /*
    -------------------------------------
    Dtor
    -------------------------------------*/
		
		virtual ~GenArrayList()
    {
      destruct (elements, sz);
      std::free (elements);
		}

    /*
    ---------------------------------------------
    Assures that the capacity of the array
    is at least [n]. All the elements become
    invalid and size of the array is reset to 0.
    ---------------------------------------------*/

    void reserve (UintP n)
    {
      destruct (elements, sz);
      sz = 0;
      
      if (n > cap) {
        
        std::free (elements);
        elements = (Uint8*) std::malloc (n * eltSize);
        cap = (Uint32) n;
      }
    }

    /*
    ------------------------------------------
    Assures that the capacity of the array
    is at least [n]. In case of reallocation
    the existing elements are copied and size
    is always preserved.
    ------------------------------------------*/

    void reserveAndCopy (UintP n)
    {
      if (n <= cap) return;
      
      void* newElements = std::malloc (n * eltSize);
      construct (newElements, sz);
      copy (newElements, elements, sz);
      
      destruct (elements, sz);
      std::free (elements);
      
      elements = (Uint8*) newElements;
      cap = (Uint32) n;
    }

    /*
    ---------------------------------------
    Invalidates all the items and resets
    array size to 0.
    ---------------------------------------*/
		
		void clear()
		{
      destruct (elements, sz);
			sz = 0;
		}

    /*
    ------------------------------------------------------
    Enlarges the array capacity at element insertion by
    the factor of 2. This yields an ammortized complexity
    of O(1) for element insertion.
    ------------------------------------------------------*/
		
		void insertAt (UintP index, const void *newElt)
		{
      void *copyElt = (void*) newElt;      
      bool cloned = false;

      //Clamp insertion point to size
			if (index > sz) index = sz;
      
      //Will any moving take place?
			if (sz == cap || index < sz)
      {
        //It might be a reference to our own element
        //so better clone it before reallocation
        copyElt = std::malloc (eltSize);
        construct (copyElt, 1);
        copy (copyElt, newElt, 1);
        cloned = true;
      }
      
      //Make sure we got enough space
      if (sz == cap) reserveAndCopy (cap * 2);
      
      //Construct an element at the back
      construct (elements + sz*eltSize, 1);
      
      //Shift forward the elements above the index
      if (index < sz)
			  for (UintP i=sz-1; i>=index; i--)
          copy (elements + (i+1)*eltSize, elements + i*eltSize, 1);
			
      //Copy the new element at [index]
      copy (elements + index*eltSize, copyElt, 1);
			sz++;
      
      //Delete the clone
      if (cloned) {
        destruct (copyElt, 1);
        std::free (copyElt);
      }
		}
    
		void removeAt (UintP index)
		{
      //Prevent invalid removal
      if (index >= sz) return;

      //Shift backwards the elements above the index
			for (UintP i=index; i<sz-1; i++)
        copy (elements + i*eltSize, elements + (i+1)*eltSize, 1);
      
      //Destruct the last element
      destruct (elements + sz*eltSize, 1);
			sz--;
		}

		void setAt (UintP index, const void *newElt)
		{
      copy (elements + index, newElt, 1);
		}
    
		void pushBack (const void *newElt)
		{
      insertAt (sz, newElt);
		}
    
    void popBack ()
    {
      removeAt (sz-1);
    }

    int capacity() const
    {
      return cap;
    }
    
		int size() const
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
	};

  /*
  -----------------------------------------------------
  A serializable array list with non-class elements
  -----------------------------------------------------*/
  
  template <class T> class DynArrayList : public GenArrayList
	{
  public:
    
    DynArrayList ()
      : GenArrayList (sizeof(T), NULL)
    {}
    
    DynArrayList (int newCap)
      : GenArrayList (sizeof(T), NULL, newCap)
    {}

    
    DynArrayList (UintP eltSize, ClassPtr eltCls)
      : GenArrayList (eltSize, eltCls)
    {}
    
    DynArrayList (UintP eltSize, ClassPtr eltCls, UintP newCap)
      : GenArrayList (eltSize, eltCls, newCap)
    {}
    
		void pushBack (const T &newElt)
		{
      GenArrayList::pushBack (&newElt);
		}

		void setAt (int index, const T &newElt)
		{
      GenArrayList::setAt (index, &newElt);
		}
    
    T& first() const
    {
      return ((T*)elements) [0];
    }
    
    T& last() const
    {
      return ((T*)elements) [sz-1];
    }
    
		T& elementAt (int index) const
		{
			return ((T*)elements) [index];
		}

    T& at (int index) const
    {
      return ((T*)elements) [index];
    }
		
		T& operator[](int index) const
		{
			return ((T*)elements) [index];	
    }
    
		T* buffer() const
		{
			return (T*) elements;
		}
		
		bool contains (const T &el) const
		{
			return (indexOf(el) != -1);
		}
		
		int indexOf (const T &el) const
		{
			for (int i=0; i<sz; i++)
				if (((T*)elements) [i] == el)
					return i;
      
			return -1;
		}
    
		void remove (const T &el)
		{
			int i = indexOf(el);
			if (i > -1)	removeAt(i);
		}
  };

  /*
  ------------------------------------------------
  A typical array list (non-serializable!)
  ------------------------------------------------*/
  
	template <class T>
  class ResArrayList : public DynArrayList <T>
	{
  public:

    ResArrayList ()
      : DynArrayList (sizeof(T), NULL)
    {}
    
    ResArrayList (int newCap)
      : DynArrayList (sizeof(T), NULL newCap)
    {}
    
    virtual void construct (Uint8 *dst, UintP n)
    {
      new (dst) T [n];
    }
    
    virtual void destruct (Uint8 *dst, UintP n)
    {
      T *tdst = (T*)dst;
      for (int d=0; d<n; ++d)
        tdst[n].~T();
    }
    
    virtual void copy (Uint8 *dst, const Uint8 *src, UintP n)
    {
      T *tdst = (T*)dst;
      T *tsrc = (T*)src;
      for (int i=0; i<n; ++i)
        tdst[i] = tsrc[i];
    }
  };

  
  /*
  --------------------------------------------------------
  A serializable list of pointers to serializable classes
  --------------------------------------------------------*/
  
	template <class T>
  class ResPtrArrayList : public DynArrayList <T*>
	{
  public:
    
    ResPtrArrayList ()
      : DynArrayList (sizeof(T*), Class(T))
    {}
    
    ResPtrArrayList (UintP newCap)
      : DynArrayList (sizeof(T*), Class(T), newCap)
    {}
  };
  

}//namespace GE
#endif //__GEARRAYLIST_RES_H
