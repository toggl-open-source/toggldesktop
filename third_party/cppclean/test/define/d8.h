
#include <auto_ptr>

namespace ns {
class OneReg;
class Foo {
  auto_ptr<OneReg> saved_;
};
}  // namespace ns
