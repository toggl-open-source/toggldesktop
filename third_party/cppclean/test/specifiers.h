class Base {
  virtual ~Base();
  virtual foo();
  virtual bar();
};

class Derived {
  void foo() override {
    getKey()->skip();
    getValue()->skip();
  }

  void bar() final {
    getKey()->skip();
    getValue()->skip();
  }
};
