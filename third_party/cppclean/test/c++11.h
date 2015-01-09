class X {
  X& operator=(const X&) = delete;
  X(const X&) = delete;
};

class Y {
  Y& operator=(const Y&) = default;
  Y(const Y&) = default;
};
