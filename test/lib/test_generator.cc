#include "generator.hpp"


int yield_list(int x) {
  static int a = 0;
  static int first = 0;

  while(a < x) {
    BEGIN_YIELD
    printf("inside function %d\n", a);
    YIELD(a)
    a += 1;
    END_YIELD
  }

  throw StopIteration();
}


int func(shared_ptr< List<int> > iter) {
  for(; iter->has_next(); ){
    return iter->next() + 10;
  }
  throw StopIteration();
}


int main() {
  auto list = make_shared< List<int> >(3, 10, 20, 30);

  /*
  for(; list->has_next(); )
    cout << list->next() << endl;
  */

  /*
  auto x = make_shared< Generator< shared_ptr< List<int> >,  int > >(func, list);
  for(; 1; ) {
    cout << x->next() << endl;
  }
  */

  /*
  auto x = make_shared< GeneratorFromFunction<function< int (int) >, int, int> > (yield_list, 10);
  for(; 1; ) {
    cout << x->next() << endl;
  }
  */


  cout << yield_list(10) << endl;
  cout << yield_list(10) << endl;
  cout << yield_list(10) << endl;
  cout << yield_list(10) << endl;
  cout << yield_list(10) << endl;
  cout << yield_list(10) << endl;
}
