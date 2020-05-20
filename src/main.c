#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jsonConverter.h"

int readFromFile(const char *filename, char **data, size_t *len)
{
	FILE *fp;
	int ch_count = 0;
	fp = fopen(filename, "r+");
	if (fp == NULL)
	{
		return 0;
	}
	fseek(fp, 0, SEEK_END);
	ch_count = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	*data = (char *) malloc(sizeof(char) * (ch_count + 1));
	fread(*data, 1, ch_count,fp);
	*len = (size_t)ch_count;
	fclose(fp);
	return 1;
}


int main(int argc, char *argv[])
{
	char *filename = NULL;
	char *fileData = NULL;
    size_t len = 0;

	if(argc >= 2 && argv[1] != NULL)
	{
		printf("Json file is %s\n",argv[1]);
		filename = strdup(argv[1]);
	}
	else
	{
		printf("Json file path is required\n");
		printf("\nUsage: ./jsonToBlobConverter jsonFilePath\n");
	}

	printf("****** Reading data from file *******\n");
	if(readFromFile(filename, &fileData, &len))
	{
		printf("Json file data is %ld bytes\n",len);
	}
	else
	{
		fprintf(stderr,"File not Found\n");
	}
	convertJsonToBlob(fileData);
	free(filename);
}

