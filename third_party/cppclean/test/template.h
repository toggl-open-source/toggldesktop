template <typename T>
class Foo
{
};


Foo<int []> bar()
{
  Foo<int []> x;
  return x;
}
