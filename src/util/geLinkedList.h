#ifndef __GELINKEDLIST_H
#define __GELINKEDLIST_H

namespace GE
{
  /*
	template<class T>
	class ArrayList : public SmartObj
	{
  public:
    class Iterator
    {
      friend class ArrayList;
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
    

		ArrayList()
		{
			_size = 0;
			max_size = 1;
			elements = new T[max_size];
      _begin.element = elements;
      _end.element = elements;
		}

		
		ArrayList(int newCapacity)
		{
			_size = 0;
			max_size = newCapacity;
			elements = new T[max_size];
      _begin.element = elements;
      _end.element = elements;
		}

		
		virtual ~ArrayList()
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
    
    void addList(ArrayListRef<T> list)
    {
    	for (int i=0; i<list->size(); i++)
      	pushBack(list[i]);
    }
		
		ArrayListRef<T> clone() const
		{
			ArrayListRef<T> newarr = new ArrayList<T>(this->max_size);

			for (int i=0; i<_size; i++) {
				newarr->pushBack(elements[i]);
			}
			
			return newarr;
		}
	}; */

  
  ////////////////////////////////////////////////
  //LinkedList - dynamic size list that connects
  //elements with pointers to next list element
  ////////////////////////////////////////////////
  
  template <class T> class LinkedList
  {
  public:
    class Iterator;
    
  private:
    class Node
    {
      friend class LinkedList;
      friend class Iterator;
      
      T element;
      Node *next;
      Node *prev;
    };
    
  public:
    
    class Iterator
    {
      friend class LinkedList;
      
      Node *node;
      
      Iterator(Node *n)
      {
        node = n;
      }
      
    public:
      
      Iterator()
      {
        node = NULL;
      }
      
      Iterator(const Iterator &i)
      {
        node = i.node;
      }
      
      Iterator& operator=(const Iterator &i)
      {
        node = i.node;
        return *this;
      }
      
      bool operator==(const Iterator &i) const
      {
        return node == i.node;
      }
      
      bool operator!=(const Iterator &i) const
      {
        return node != i.node;
      }
      
      Iterator& operator--()
      {
        node = node->prev;
        return *this;
      }
      
      Iterator& operator++()
      {
        node = node->next;
        return *this;
      }
      
      Iterator operator+ (int a) const
      {
        Iterator i = *this;
        for (int ii=0; ii<a; ++ii) ++i;
        return i;
      }
      
      Iterator operator- (int a) const
      {
        Iterator i = *this;
        for (int ii=0; ii<a; ++ii) --i;
        return i;
      }
      
      T& operator*() const
      {
        return node->element;
      }
      
      T* operator->() const 
      {
        return &node->element;
      }
    };

    Iterator _begin;
    Iterator _end;
    int _size;
    Node _tail;
    
  public:
    
    const Iterator& begin() const {return _begin;}
    const Iterator& end() const {return _end;}
    T& first() const {return _begin.node->element;}
    T& last() const {return _end.node->prev->element;}
    int size() const {return _size;}
    bool empty() const {return _size==0;}
    
    LinkedList()
    {
      _tail.prev = NULL;
      _tail.next = NULL;
      
      _begin.node = &_tail;
      _end.node = &_tail;
      
      _size = 0;
    }
    
    ~LinkedList()
    {
      clear();
    }
    
    void clear()
    {
      while (_begin.node != _end.node) {
        Node *beg = _begin.node;
        _begin.node = _begin.node->next;
        delete beg;
      }
      
      _size = 0;
      _begin.node = _end.node;
      _begin.node->prev = NULL;
    }
    
    Iterator pushFront(const T &e)
    {
      Node *n = new Node;
      n->element = e;
      
      n->prev = NULL;
      n->next = _begin.node;
      
      _begin.node->prev = n;
      _begin.node = n;
      
      _size++;
      return Iterator(n);
    }
    
    Iterator pushBack(const T &e)
    {
      if (_end.node->prev == NULL) {
        
        return pushFront(e);
        
      }else{
        
        Node *n = new Node;
        n->element = e;
        
        n->next = _end.node;
        n->prev = _end.node->prev;
        
        _end.node->prev->next = n;
        _end.node->prev = n;
        
        _size++;
        return Iterator(n);
      }
    }
    
    Iterator popFront()
    {
      Node *beg = _begin.node;
      beg->next->prev = NULL;
      _begin = beg->next;
      delete beg;
      
      _size--;
      return _begin;
    }
    
    Iterator popBack()
    {
      if (_begin.node->next ==_end.node) {
        
        popFront();
        return _end;
        
      }else{
        
        Node *prev = _end.node->prev;
        prev->prev->next = _end.node;
        _end.node->prev = prev->prev;
        delete prev;
        
        _size--;
        return _end;
      }
    }
    
    Iterator insertAt(const Iterator &i, const T &e)
    {
      if (i.node == _begin.node) {
        
        return pushFront(e);
        
      }else {
        
        Node *n = new Node;
        n->element = e;
        
        n->prev = i.node->prev;
        n->next = i.node;
        
        i.node->prev->next = n;
        i.node->prev = n;
        
        _size++;
        return Iterator(n);
      }
    }
    
    Iterator removeAt(const Iterator &i)
    {
      if (i.node == _begin.node) {
        
        return popFront();
        
      }else{
        
        i.node->prev->next = i.node->next;
        i.node->next->prev = i.node->prev;
        Iterator out(i.node->next);
        delete i.node;
        
        _size--;
        return out;
      }
    }
    
    Iterator remove(const T &e)
    {
      Iterator i = iteratorOf(e);
      if (i != _end) return removeAt(i);
      else return _end;
    }
    
    void setAt(const Iterator &i, const T &e)
    {
      i.node->element = e;
    }
    
    void replace(const T &eold, const T &enew)
    {
      Iterator i = iteratorOf(eold);
      if (i != _end) setAt(i, enew);
    }
    
    bool contains(const T &e)
    {
      for (Iterator i=_begin; i!=_end; ++i)
        if (i.node->element == e) return true;
      
      return false;
    }
    
    Iterator iteratorOf(const T &e)
    {
      for (Iterator i=_begin; i!=_end; ++i)
        if (i.node->element == e) return i;
      
      return _end;
    }
    
    int indexOf(const T &e)
    {
      int index = 0;
      for (Iterator i=_begin; i!=_end; ++i, ++index)
        if (i.node->element == e) return index;
      
      return -1;
    }
    
    int indexOf(const Iterator &i)
    {
      if (i.node == NULL)
        return -1;
      
      int index = 0;
      for (Iterator ii=_begin; ii!=i; ++ii)
        ++index;
      
      return index;
    }
  };

}//namespace GE
#endif//__GELINKEDLIST_H
