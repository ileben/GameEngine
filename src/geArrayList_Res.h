#ifndef __GEARRAYLIST_RES_H
#define __GEARRAYLIST_RES_H

namespace GE
{
  
	template <class T, bool resourceElements>
  class ArrayList_Res : public IResource
	{
    DECLARE_SERIAL_CLASS (ArrayList_Res);
    DECLARE_END;
    
  public:
    
    virtual Uint32 getID () { return 1; }
    virtual UintP getSize () { return sizeof (ArrayList_Res); }
    virtual void getPointers (SerializeManager *sm)
    {
      if (_size != 0)
      {
        //if (resourceElements)
          //sm->resourcePtr (&elements, size());
        //else
          sm->dynamicPtr (&elements, size() * sizeof (T));
      }
    }
    
  public:
    class Iterator
    {
      friend class ArrayList_Res;
      T *element;

      Iterator(T* e) {
        element = e;
      }

    public:
      Iterator() {}

      Iterator(const Iterator &i) {
        element = i.element;
      }
      
      Iterator& operator= (const Iterator &i) {
      	element = i.element;
        return *this;
      }

      bool operator== (const Iterator &i) {
        return element == i.element;
      }

      bool operator!= (const Iterator &i) {
        return element != i.element;
      }
      
      Iterator& operator++() {
      	element++;
        return *this;
      }
      
      Iterator operator++(int) {
      	Iterator ans(element);
        element++;
        return ans;
      }
      
      Iterator& operator--() {
      	element--;
        return *this;
      }
      
      Iterator operator--(int) {
      	Iterator ans(element);
      	element--;
        return ans;
      }

      Iterator operator+(int i) {
        Iterator it(element + i);
        return it;
      }

      Iterator operator-(int i) {
        Iterator it(element - i);
        return it;
      }
      
      Iterator& operator+=(int i) {
      	element += i;
        return *this;
      }
      
      Iterator& operator-=(int i) {
      	element -= i;
        return *this;
      }

      T& operator* () {
        return *element;
      }

      T* operator-> () {
        return element;
      }
    };

	protected:
		int _size;
		int max_size;
		T *elements;
    Iterator _begin;
    Iterator _end;

    /*-----------------------------------------
     * Enlarges the array capacity at element
     * insertion by the factor of 2. This
     * yields an ammortized complexity of O(1)
     * for element insertion.
     *-----------------------------------------*/
    
		void allocMore()
		{
			T *temp = new T[max_size * 2];
			
			for (int i=0; i<_size; i++)
				temp[i] = elements[i];
			
			delete[] elements;
			elements = temp;
			max_size *= 2;
      
      _begin.element = elements;
      _end.element = elements + _size;
		}
    
	public:
  	const Iterator& begin() const {return _begin;}
    const Iterator& end() const {return _end;}
    T& first() const {return elements[0];}
    T& last() const {return elements[_size-1];}
    bool empty() const {return _size == 0;}
    
    /*--------------------------------------
     * Default constructor. Initializes the
     * array with storage for 1 element
     *--------------------------------------*/

    ArrayList_Res (SerializeManager *sm)
    {
      _begin.element = elements;
      _end.element = elements;
    }
    
		ArrayList_Res()
		{
			_size = 0;
			max_size = 1;
			elements = new T[max_size];
      _begin.element = elements;
      _end.element = elements;
		}

    /*-----------------------------------
     * Constructor to initialize the
     * array with given capacity.
     *-----------------------------------*/
		
		ArrayList_Res(int newCapacity)
		{
			_size = 0;
			max_size = newCapacity;
			elements = new T[max_size];
      _begin.element = elements;
      _end.element = elements;
		}

    /*-------------------------------------
     * Destructor deletes all the elements
     *-------------------------------------*/
		
		virtual ~ArrayList_Res()
		{
			clear();		
			delete[] elements;
		}
    
    void setSize(int newsize)
    {
    	if (max_size < newsize) {
      
      	delete[] elements;
        max_size *= 2;
        if (max_size < newsize)
        	max_size = newsize;
        elements = new T[max_size];
      }
      
      _size = newsize;
      _begin.element = elements;
      _end.element = elements + _size;
    }

    /*---------------------------------------------
     * Assures that the capacity of the array
     * is at least [n]. All the elements become
     * invalid and size of the array is reset to 0.
     *---------------------------------------------*/

    void reserve(int n)
    {
      if (n > max_size) {

        delete[] elements;
        elements = new T[n];
        max_size = n;
      }

      _size = 0;
      _begin.element = elements;
      _end.element = elements;
    }

    /*------------------------------------------
     * Assures that the capacity of the array
     * is at least [n]. In case of reallocation
     * the existing elements are copied and size
     * is always preserved.
     *------------------------------------------*/

    void reserveAndCopy(int n)
    {
      if (n <= max_size) return;

      T* newElements = new T[n];
      for (int i=0; i<_size; ++i)
        newElements[i] = elements[i];

      delete[] elements;
      elements = newElements;
      max_size = n;

      _begin.element = elements;
      _end.element = elements + _size;
    }

    /*-------------------------------------------
     * Getter functions to return information
     * about the current array state.
     *-------------------------------------------*/

    int capacity() const
    {
      return max_size;
    }

		int size() const
		{
			return _size;
		}
		
		T* buffer() const
		{
			return elements;
		}

    /*---------------------------------------
     * Invalidates all the items and resets
     * array size to 0.
     *---------------------------------------*/
		
		void clear()
		{   
			_size = 0;
      _end.element = elements;
		}
		
		void pushBack(const T &new_el)
		{
      if (_size==max_size)
      {
        //It might be a reference to our own element
        //so better store it before reallocation
        T temp_new_el = new_el;
        allocMore();
        elements[_size++] = temp_new_el;

      }else{

			  elements[_size++] = new_el;
      }

      _end.element = elements + _size;
		}

    void popBack()
    {
      if (_size==0) return;
      _size--;
      _end.element = elements + _size;
    }
		
		void remove(const T &el)
		{
			int i = indexOf(el);
			if (i > -1)	removeAt(i);
		}
		
		void removeAt(int index)
		{	
			if (index < _size-1)
				for (int i=index; i<_size-1; i++)
					elements[i] = elements[i+1];

			_size--;
      _end.element = elements + _size;
		}
		
		void insertAt(int index, const T &new_el)
		{
			if (index > _size)
				index = _size;

			if (_size==max_size) allocMore();

			if (index < _size)
				for (int i=_size-1; i>=index; i--)
					elements[i+1] = elements[i];
			
			elements[index] = new_el;
			_size++;
      _end.element = elements + _size;
		}
		
		T& elementAt(int index) const
		{
			return elements[index];
		}

    T& at(int index) const
    {
      return elements[index];
    }
		
		T& operator[](int index) const
		{
			return elements[index];	
		}
		
		void setAt(int index, const T &el)
		{
			elements[index] = el;
		}
		
		bool contains(const T &el) const
		{
			return (indexOf(el) != -1);
		}
		
		int indexOf(const T &el) const
		{
			for (int i=0; i<_size; i++)
				if (elements[i] == el)
					return i;

			return -1;
		}
    
    int indexOf(const Iterator &it) const
    {
      return it.element - elements;
    }
	};

}//namespace GE
#endif //__GEARRAYLIST_RES_H
