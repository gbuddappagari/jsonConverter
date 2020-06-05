#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include "../jsonConverter.h"

#define MULTIPART_DOC "/tmp/multipart.bin"
#define MAX_BUFSIZE 512

typedef struct multipart_subdoc
{
	char *name;
	char *version;
	char *data;
	size_t length;
}multipart_subdoc_t;

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

int parseSubDocArgument(char **args, int count, multipart_subdoc_t **docs)
{
	int i = 0, j = 0;
	char *fileName = NULL;
	char *fileData = NULL;
	char outFile[128];
    size_t len = 0;
	for (i = 2; i<count; i++)
	{
		(*docs)[j].version = strtok(args[i],",");
		(*docs)[j].name = strtok(NULL,",");
		fileName = strtok(NULL,",");
		if(processEncoding(fileName, "M", 0))
		{
			sprintf(outFile,"%s.bin",strtok(fileName,"."));
			if(readFromFile(outFile, &fileData, &len))
			{
				(*docs)[j].data = strdup(fileData);
				(*docs)[j].length = len;
			}
			else
			{
				printf("Failed to open %s\n",fileName);
				(*docs)[j].data = NULL;
				(*docs)[j].length = 0;
			}
		}
		else
		{
			return 0;
		}
		j++;
	}
	return 1;
}

int append_str (char * dest, char * src)
{
	int len;
	len = strlen (src);
	if (len > 0)
	    strncpy (dest, src, len);
	return len;
}

void generateBoundary(char *s ) 
{
	const int len = 50;
    static const char charset[] =
        "0123456789"
		"+"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
	srand((unsigned int)clock());
    for (int i = 0; i < len; ++i) {
        s[i] = charset[rand() % (sizeof(charset) - 1)];
    }

    s[len] = 0;
}

int add_header(char *name, char *value, char *buffer)
{
	int bufLength = strlen(name)+strlen(value)+2;
	sprintf(buffer, "%s%s\r\n",name,value);
	return bufLength;
}

int getSubDocBuffer(multipart_subdoc_t subdoc, char **buffer)
{
	int bufLength = 0, length = 0;
	static char  * hdrbuf = NULL;
	char  * pHdr = NULL;
	if (hdrbuf==NULL)
	{
		hdrbuf = (char *) malloc (sizeof(char)*(MAX_BUFSIZE+subdoc.length));
	}
	memset (hdrbuf, 0, sizeof(char)*(MAX_BUFSIZE+subdoc.length));
	pHdr = hdrbuf;
	length = add_header("Content-type: ","application/msgpack", pHdr);
	pHdr += length;
	length = add_header("Etag: ",subdoc.version, pHdr);
	pHdr += length;
	length = add_header("Namespace: ",subdoc.name, pHdr);
	pHdr += length;
	length = append_str(pHdr, "\n");
	pHdr += length;
	length = append_str(pHdr, subdoc.data);
	pHdr += length;
	length = append_str(pHdr, "\r\n\n");
	pHdr += length;
	*buffer = hdrbuf;
	bufLength = strlen(hdrbuf);
	return bufLength;
}
int writeToFile(char *file_path, char *data, size_t size)
{
	FILE *fp;
	fp = fopen(file_path , "w+");
	if (fp == NULL)
	{
		printf("Failed to open file %s\n", file_path );
		return 0;
	}
	if(data !=NULL)
	{
		fwrite(data, size, 1, fp);
		fclose(fp);
		return 1;
	}
	else
	{
		printf("writeToFile failed, Data is NULL\n");
		fclose(fp);
		return 0;
	}
}

int getSubDocsDataSize(multipart_subdoc_t *subdocs, int count)
{
	int total = 0;
	for(int i=0; i<count; i++)
	{
		total += subdocs[i].length;
	}
	return total;
}

int generateMultipartBuffer(char *rootVersion, int subDocCount, multipart_subdoc_t *subdocs, char **buffer)
{
	char boundary[50] = {'\0'};
	char *temp = NULL;
	int subDocsDataSize = 0, subdocLen = 0, bufLen = 0, len = 0;
	generateBoundary(boundary);
	printf("boundary: %s\n",boundary);
	subDocsDataSize = getSubDocsDataSize(subdocs, subDocCount);
	printf("Subdocs data size: %d\n",subDocsDataSize);
	*buffer = (char *)malloc(sizeof(char)*(MAX_BUFSIZE+subDocsDataSize));
	memset(*buffer, 0, sizeof(char)*(MAX_BUFSIZE+subDocsDataSize));
	temp = *buffer;
	len = append_str(temp, "HTTP 200 OK\r\n");
	temp += len;
	len = add_header("Content-type: multipart/mixed; boundary=",boundary,temp);
	temp += len;
	len = add_header("Etag: ",rootVersion,temp);
	temp += len;
	len = append_str(temp, "\n");
	temp += len;
	for (int j = 0; j<subDocCount; j++)
	{
		len = add_header("--",boundary,temp);
		temp += len;
		char *subDocBuffer = NULL;
		subdocLen = getSubDocBuffer(subdocs[j], &subDocBuffer);
		printf("subdocLen: %d\n", subdocLen);
		strncpy(temp, subDocBuffer, subdocLen);
		temp += subdocLen;
	}
	len = add_header("--",boundary,temp);
	temp += len;
	bufLen = (int)strlen(*buffer);
	return bufLen;
}

int main(int argc, char *argv[])
{
	int subDocCount = 0;
	char *rootVersion = NULL;
	multipart_subdoc_t *subdocs = NULL;
	char  * buffer = NULL;
	int bufLen = 0;
	if(argc < 3)
	{
		printf("Usage: ./multipartDoc <root-version> <subDocVersion1,subDocName1,subDocFilePath1> ... <subDocVersionN,subDocNameN,subDocFilePathN>\n");
		exit(1);
	}
	rootVersion = strdup(argv[1]);
	printf("rootVersion: %s\n",rootVersion);
	subDocCount = argc - 2;
	printf("subDocCount: %d\n",subDocCount);
	subdocs = (multipart_subdoc_t  *)malloc(sizeof(multipart_subdoc_t )*subDocCount);
	if(!parseSubDocArgument(argv, argc, &subdocs))
	{
		FREE(rootVersion);
		FREE(subdocs);
		return 0;
	}
	bufLen = generateMultipartBuffer(rootVersion, subDocCount, subdocs, &buffer);
	if(bufLen > 0)
	{
		printf("Multipart buffer length is %d\n",bufLen);
		if(writeToFile(MULTIPART_DOC, buffer, bufLen) == 0)
		{
			fprintf(stderr,"%s File not Found\n",MULTIPART_DOC);
		}
	}
	else
	{
		fprintf(stderr, "Failed to generate multipart buffer\n");
	}
	if(subdocs != NULL)
	{
		for(int i=0; i< subDocCount; i++)
		{
			FREE(subdocs[i].data);
		}
		FREE(subdocs);
	}
	FREE(buffer);
	FREE(rootVersion);
	return 0;
}
