#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "util.h"
int
main(int argc, char *argv[])
{
	if (argc ==2 && !strcmp("-v", argv[1]))
		die("writestr-0.1\n");
	else if (argc != 3)
		die("usage:\twritestr -v\n\twritestr filename str\n");
	FILE *fp = fopen(argv[1], "w");
	fputs(argv[2], fp);
	fclose(fp);
	return(EXIT_SUCCESS);
}
