class Iterator
{
  friend class LinkedList;
  friend class CyclicIterator;
  
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

class CyclicIterator
{
  LinkedList *list;
  Node *node;
  Node *start;
  bool moved;

public:

  CyclicIterator()
  {
    list = NULL;
    node = NULL;
    start = NULL;
    moved = false;
  }

  CyclicIterator (LinkedList &l)
  {
    begin( l );
  }

  CyclicIterator (LinkedList &l, const Iterator &it)
  {
    begin( l, it );
  }

  CyclicIterator (const CyclicIterator &it)
  {
    begin( it );
  }

  void begin (LinkedList &l)
  {
    node = l._begin.node;
    list = &l;
    start = node;
    moved = false;
  }

  void begin (LinkedList &l, const Iterator &it)
  {
    node = it.node;
    list = &l;
    start = node;
    moved = false;
  }

  void begin (const CyclicIterator &it)
  {
    node = it.node;
    list = it.list;
    start = node;
    moved = false;
  }

  bool end ()
  {
    return (moved && node == start);
  }

  CyclicIterator& operator= (const CyclicIterator &it)
  {
    node = it.node;
    list = it.list;
    start = it.start;
    moved = it.moved;
    return *this;
  }

  CyclicIterator& operator--()
  {
    if (node == list->_begin.node)
      node = list->_end.node;

    node = node->prev;

    moved = true;
    return *this;
  }
  
  CyclicIterator& operator++()
  {
    node = node->next;

    if (node == list->_end.node)
      node = list->_begin.node;

    moved = true;
    return *this;
  }

  bool operator==(const CyclicIterator &i) const
  {
    return node == i.node;
  }
  
  bool operator!=(const CyclicIterator &i) const
  {
    return node != i.node;
  }

  T& operator*() const
  {
    return node->element;
  }
  
  T* operator->() const 
  {
    return &node->element;
  }

  operator Iterator ()
  {
    Iterator i;
    i.node = node;
    return i;
  }
};
