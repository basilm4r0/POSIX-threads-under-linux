#ifndef _GNU_SOURCE
	#define _GNU_SOURCE
#endif

#ifndef __LOCAL_H__
	#define __LOCAL_H__
#endif

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <pthread.h>

#include <string>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <vector>
#include <fstream>
#include <sstream>
#include <sys/wait.h>
#include <math.h>

void read_constants(std::string filename);
