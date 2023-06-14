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
#include <signal.h>


void read_constants(std::string filename);
bool hitWall(double, double);
double randomDouble();
int randomInt(int a, int b);
double randomDouble(int a, int b);
double findAngle(double x1, double y1, double x2, double y2);

typedef struct
{
	double x;
	double y;
	int numOfPortions;
	pthread_mutex_t portions_mutex;
} FOOD;

typedef struct
{
	double x;
	double y;
	double direction;
	double pheromone;
	double foodX;
	double foodY;
	pthread_mutex_t direction_mutex;
} ANT;

/*

	in ant1:
	lock(mutex)
	portions --;
	unlock()

	wait(1 second)

	ant2:
	lock
*/
