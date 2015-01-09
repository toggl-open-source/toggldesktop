
void SomeFunction() {}

static void static_functions_are_fine()
{
}

template <typename T>
static void static_templated_functions_are_fine()
{
    T t;
    t.foo();
}
