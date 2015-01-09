
#include "something1.h"

class Bar;
class Baz;
class Bong;
class Bling;

class F : public Bar<Baz, Bling>, Bong, Something {
};
