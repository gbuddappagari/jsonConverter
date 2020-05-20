#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "jsonConverter.h"

int main(int argc, char *argv[])
{
	char *filename = NULL, *enc = NULL;
	int ret = 0;
	if(argc < 2)
	{
		fprintf(stderr,"Usage: %s -f <filename> --[encoding]\n", argv[0]);
		return 0;
	}
	
	static const struct option long_options[] = {
        {"json-file",      required_argument, 0, 'f'},
        {"B",              no_argument,       0, 'B'},
        {"M",              no_argument,       0, 'M'},
        {0, 0, 0, 0}
    };
	int c;
	optind = 1;
	while (1)
	{
		/* getopt_long stores the option index here. */
		int option_index = 0;
		c = getopt_long (argc, argv, "f:B:M",
		long_options, &option_index);

		/* Detect the end of the options. */
		if (c == -1)
			break;

		switch (c)
		{
			case 'f':
				filename = strdup(optarg);
				printf("json-file is %s\n",filename);
			break;
			case 'B':
				printf("BLOB encoding\n");
				enc = strdup("B");
			break;
			case 'M':
				printf("Msgpack encoding\n");
				enc = strdup("M");
			break;
			case '?':
			/* getopt_long already printed an error message. */
			break;

			default:
				printf("Enter Valid arguments..\n");
			return -1;
		}
	}

	ret = processEncoding(filename, enc);
	if(!ret)
	{
		printf("Encoding failed\n");
	}
	FREE(filename);
	FREE(enc);
	return 0;
}

