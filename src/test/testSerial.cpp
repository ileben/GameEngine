#include "util/geUtil.h"
#include <iostream>
using namespace GE;

class C
{
  DECLARE_SERIAL_CLASS( C );
  DECLARE_DATAVAR( data );
  DECLARE_END;

public:
  int data;

  C() { std::cout << "C::ctor()" << std::endl; }
  C(SM *sm) { std::cout << "C::ctor(SM*)" << std::endl; }
};

class B
{
  DECLARE_SERIAL_CLASS( B );
  DECLARE_DATAVAR( data );
  DECLARE_END;

public:
  int data;

  B() { std::cout << "B::ctor()" << std::endl; }
  B(SM *sm) { std::cout << "B::ctor(SM*)" << std::endl; }
};

class A
{
  DECLARE_SERIAL_CLASS( A );
  DECLARE_DATAVAR( data );
  DECLARE_OBJVAR( b );
  DECLARE_OBJPTR( c );
  DECLARE_END;

public:
  int data;
  B b;
  C *c;

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
  }

  std::cout << "\nPASS" << std::endl;
  return true;
}

int main (int argc, char **argv)
{
  test1();
  test2();

  return EXIT_SUCCESS;
}
