class Foo {
  MY_MACRO
};

class Bar {
  void fct();

  MY_MACRO
};

class Baz {
  MY_MACRO(Baz)
};

class Qux {
  void fct();

  MY_MACRO(Qux)
};

#ifndef MY_MACRO_H
#define MY_MACRO_H

MY_MACRO(Foo)

#endif
