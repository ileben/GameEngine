#include "util/geUtil.h"
#include <iostream>
using namespace GE;

class A;
class B;
class C;


class B : public Object
{
  DECLARE_SERIAL_SUBCLASS( B, Object );
  DECLARE_DATAVAR( data );
  DECLARE_END;

public:
  int data;

  B() { std::cout << "B::ctor()" << std::endl; }
  B(SM *sm) { std::cout << "B::ctor(SM*)" << std::endl; }
};

class C : public B
{
  DECLARE_SERIAL_SUBCLASS( C, B );
  DECLARE_DATAVAR( data2 );
  DECLARE_OBJREF( ptr );
  DECLARE_END;

public:
  int data2;
  A* ptr;

  C() { std::cout << "C::ctor()" << std::endl; }
  C(SM *sm) { std::cout << "C::ctor(SM*)" << std::endl; }
};

class A : public Object
{
  DECLARE_SERIAL_SUBCLASS( A, Object );
  DECLARE_DATAVAR( data );
  DECLARE_OBJVAR( b );
  DECLARE_OBJPTR( c );
  DECLARE_END;

public:
  int data;
  B b;
  B *c;

  A() { std::cout << "A::ctor()" << std::endl; }
  A(SM *sm) : b(sm) { std::cout << "A::ctor(SM*)" << std::endl; }
};

DEFINE_SERIAL_CLASS( A, ClassID( 0, 0, 0, 1 ) );
DEFINE_SERIAL_CLASS( B, ClassID( 0, 0, 0, 2 ) );
DEFINE_SERIAL_CLASS( C, ClassID( 0, 0, 0, 3 ) );

bool test1()
{
  std::cout << "\n========= Test1 ==========\n" << std::endl;

  A *a = new A;
  a->data = 1;
  a->b.data = 2;
  a->c = new C;
  a->c->data = 3;
  ((C*)a->c)->data2 = 4;
  ((C*)a->c)->ptr = a;

  std::cout << "Saving..." << std::endl;
  
  void *data;
  UintSize size;
  SerializeManager sm;
  sm.save( a, &data, &size );

  std::cout << "Loading..." << std::endl;

  ClassPtr cls;
  A *a2 = (A*) sm.load( data, &cls );

  std::cout << "Testing..." << std::endl;

  if (a2->data != a->data) {
    std::cout << "FAILED! a2 data A != a data A" << std::endl;
    return false;
  }
  if (a2->b.data != a->b.data) {
    std::cout << "FAILED! a2 data B != a data B" << std::endl;
    return false;
  }
  if (a2->c->data != a->c->data) {
    std::cout << "FAILED! a2 data C != a data C" << std::endl;
    return false;
  }
  if (((C*)a2->c)->data2 != ((C*)a->c)->data2) {
    std::cout << "FAILED! a2 data2 C != a data2 C" << std::endl;
    return false;
  }
  if (((C*)a2->c)->ptr != a2) {
    std::cout << "FAILED! a2->c->ptr != a2" << std::endl;
    return false;
  }

  std::cout << "\nPASS" << std::endl;
  return true;
}

bool test2()
{
  std::cout << "\n========= Test2 ==========\n" << std::endl;
  ObjArrayList<A> *ar = new ObjArrayList<A>;
  for (UintSize i=0; i<3; ++i)
  {
    A a;
    a.data = (int) i;
    a.b.data = (int) i*10;
    a.c = new C;
    a.c->data = (int) i*100;
    ((C*)a.c)->data2 = (int) i*1000;
    ((C*)a.c)->ptr = NULL;
    ar->pushBack( a );
  }

  std::cout << "Saving..." << std::endl;

  void *data;
  UintSize size;
  SerializeManager sm;
  sm.save( ar, &data, &size );

  std::cout << "Loading..." << std::endl;

  ClassPtr cls;
  ObjArrayList<A> *ar2 = (ObjArrayList<A>*) sm.load( data, &cls );

  std::cout << "Testing..." << std::endl;

  if (ar2->size() != ar->size()) {
    std::cout << "FAILED! ar2->size() != ar->size()" << std::endl;
    return false;
  }
  
  for (UintSize i=0; i<ar2->size(); ++i)
  {
    if (ar2->at(i).data != ar->at(i).data) {
      std::cout << "FAILED! ar2 data A != ar data A" << std::endl;
      return false;
    }
    if (ar2->at(i).b.data != ar->at(i).b.data) {
      std::cout << "FAILED! ar2 data B != ar data B" << std::endl;
      return false;
    }
    if (ar2->at(i).c->data != ar->at(i).c->data) {
      std::cout << "FAILED! ar2 data C != ar data C" << std::endl;
      return false;
    }
    if (((C*)ar2->at(i).c)->data2 != ((C*)ar->at(i).c)->data2) {
      std::cout << "FAILED! ar2 data2 C != ar data2 C" << std::endl;
      return false;
    }
  }

  std::cout << "\nPASS" << std::endl;
  return true;
}

class D : public Object
{
  DECLARE_SERIAL_SUBCLASS( D, Object );
  DECLARE_OBJVAR( str );
  DECLARE_END;

public:
  CharString str;

  D() {}
  D(SM *sm) : str(sm) {}
};

DEFINE_SERIAL_CLASS( D, ClassID( 0,0,0,4 ));

void test3()
{
  D *d = new D;
  d->str = "test";

  void *data; UintSize size;
  SerializeManager sm;
  sm.save( d, &data, &size );
  D *d2 = (D*) sm.load( data );

  const SM::ObjectList &objects = sm.getObjects();
  for (UintSize o=0; o<objects.size(); ++o)
  {
    Object *obj = objects[o];
    ClassPtr cls = ClassOf( obj );
    int zomg = 0;
  }
}

int main (int argc, char **argv)
{
  test1();
  test2();
  test3();

  return EXIT_SUCCESS;
}
