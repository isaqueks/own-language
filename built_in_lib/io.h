#ifndef LIB_H
#define LIB_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "../libs/list/list.h"
#include "../src/function.h"
#include "../src/variable.h"
#include "../src/context.h"
#include "../src/primtypes.h"

void lib_print(void* ctxptr);
void lib_install(context_t* main_context);

#endif