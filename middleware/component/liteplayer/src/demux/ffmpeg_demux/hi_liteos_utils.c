#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <math.h>



long int lrintf(float x)
{
    return (int)(rint(x));
}

long int lrint(double x)
{
    return rint(x);
}
