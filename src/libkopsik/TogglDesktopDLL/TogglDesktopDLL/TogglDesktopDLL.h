#ifdef TOGGLDESKTOPSDLL_EXPORTS
#define TOGGLDESKTOPDLL_API __declspec(dllexport)
#else
#define TOGGLDESKTOPDLL_API __declspec(dllimport)
#endif
