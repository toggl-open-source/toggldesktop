
#include "d1.h"
#include "other_defined1.h"

static void SomeStaticFunction() { }

namespace {
void SomeAnonymousFunction() { }
}

void SomeFunction1() {
  SomeStaticFunction();
}

void SomeOtherFunction1() {
  SomeAnonymousFunction();
}

void SomePublicFunction() {
}

void SomeClass::Method() {
}
