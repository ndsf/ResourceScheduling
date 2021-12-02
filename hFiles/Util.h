#pragma once
#include "ResourceScheduler.h"
#include<fstream>
#define DEBUG 1

void generator(ResourceScheduler&,int);

void WriteData(string fileName, string text);

#ifdef DEBUG
 #define D printf("[%d]", __LINE__);
#else
 #define D for(;0;)
#endif