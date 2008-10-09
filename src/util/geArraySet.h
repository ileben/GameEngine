#ifndef __GEARRAYSET_H
#define __GEARRAYSET_H

namespace GE
{
  template<class T> class ArraySet
  {
  protected:
    int _size;
    int _bufsize;
    T *elements;
    
  public:
    ArraySet()
    {
      _size = 0;
      _bufsize = 1;
      elements = new T[_bufsize];
    }
    
    ArraySet(int newCapacity)
    {
      _size = 0;
      _bufsize = newCapacity;
      elements = new T[_bufsize];
    }
    
    ~ArraySet()
    {
      delete[] elements;
    }
    
    void alloc()
    {
      _bufsize *= 2;
      T *temp = new T[_bufsize];
      
      for (int i=0; i<_size; i++)
        temp[i] = elements[i];
      
      delete[] elements;
      elements = temp;
    }
    
    void clear()
    {
      size = 0;
    }
    
    int size()
    {
      return _size;
    }
    
    T& at(int index)
    {
      return elements[index];
    }
    
    T& operator[] (int index)
    {
      return elements[index];
    }
    
    void add(const T& e)
    {
      if (_size == _bufsize)
        alloc();
      
      elements[_size] = e;
      _size++;
    }
    
    bool remove(const T& e)
    {
      for (int i=0; i<_size; i++) {
        if (elements[i] == e) {
          elements[i] = elements[_size-1];
          _size--;
          return true;
        }
      }
      
      return false;
    }
    
    bool removeRev(const T& e)
    {
      for (int i=_size-1; i>=0; i--) {
        if (elements[i] == e) {
          elements[i] = elements[_size-1];
          _size--;
          return true;
        }
      }
      
      return false;
    }
    
    void removeAt(int index)
    {
      elements[index] = elements[_size-1];
      _size--;
    }
  };

}//namespace GE
#endif//__GEARRAYSET_H
