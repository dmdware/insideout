
#include "sys/includes.h"
#include "sys/texture.h"
#include "ms3d.h"


void main()
{
	MS3DModel m;
	unsigned int test;
	char c[3];
	m.rewrite("in.ms3d", test, test, test, test, true);
	fgets(c, 2, stdin);
}







