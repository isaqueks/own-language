#include <stdio.h>
#define Throw(error) { printf("\n\n -> [Error] at " __FILE__ " line %d:\n  " error "\n\n", __LINE__); while(1); }