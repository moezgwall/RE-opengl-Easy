#ifndef __HELPER_H 
#define __HELPER_H 

#include <stdio.h>

#ifdef HELPER_IMPLEMENTATION 

#define UNUSED(X) (void)(X) 
#define TODO(X) printf("[TODO] : %s @line:%d,@file:%s",#X,__LINE__,__FILE__);

#endif // HELPER_IMPLEMENTATION

#endif // __HELPER_H 
