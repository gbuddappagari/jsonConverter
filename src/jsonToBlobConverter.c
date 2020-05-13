/*
Convert below sample json data to blob

{
"portforwarding":[
{
"Protocol":"TCP/UDP",
"Description":"Rule-6",
"Enable":"true",
"InternalClient":"192.168.1.6",
"ExternalPortEndRange":"7006",
"ExternalPort":"7006"
},
{
"Protocol":"TCP/UDP",
"Description":"Rule-7",
"Enable":"true",
"InternalClient":"192.168.1.7",
"ExternalPortEndRange":"7007",
"ExternalPort":"7007"
}
],
"subdoc_name":"portforwarding",
"version":223344,
"transaction_id":2234
}

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <msgpack.h>
#include <cJSON.h>
#include <base64.h>

#define PORTFORWARDING "portforwarding"
#define	SUBDOC_NAME "subdoc_name"
#define VERSION "version"
#define TRANS_ID "transaction_id"
#define PROTOCOL "Protocol"
#define EXTERNAL_PORT "ExternalPort"
#define INTERNAL_CLIENT "InternalClient"
#define PORT_RANGE "ExternalPortEndRange"
#define DESCRIPTION "Description"
#define ENABLE "Enable"


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

static void __msgpack_pack_string( msgpack_packer *pk, const void *string, size_t n )
{
    msgpack_pack_str( pk, n );
    msgpack_pack_str_body( pk, string, n );
}

static int convertJsonToMsgPack(char *data, char **encodedData)
{
	cJSON *jsonData=NULL, *portFwdArray = NULL;
	int encodedDataLen = 0;
	jsonData=cJSON_Parse(data);
	if(jsonData != NULL)
	{
		msgpack_sbuffer sbuf;
		msgpack_packer pk;
		msgpack_sbuffer_init( &sbuf );
		msgpack_packer_init( &pk, &sbuf, msgpack_sbuffer_write );
		msgpack_pack_map( &pk, 4);
	
		char *subdoc_name = strdup(cJSON_GetObjectItem(jsonData, SUBDOC_NAME)->valuestring);
		if(subdoc_name != NULL)
		{
			//printf("subdoc_name: %s\n",subdoc_name);
			__msgpack_pack_string(&pk, SUBDOC_NAME, strlen(SUBDOC_NAME));
			__msgpack_pack_string(&pk, subdoc_name, strlen(subdoc_name));
		}
		int version = cJSON_GetObjectItem(jsonData, VERSION)->valueint;
		//printf("version: %d\n",version);
		__msgpack_pack_string(&pk, VERSION, strlen(VERSION));
		msgpack_pack_int(&pk, version);

		int transId = cJSON_GetObjectItem(jsonData, TRANS_ID)->valueint;
		//printf("transId: %d\n",transId);
		__msgpack_pack_string(&pk, TRANS_ID, strlen(TRANS_ID));
		msgpack_pack_int(&pk, transId);
	
		portFwdArray = cJSON_GetObjectItem(jsonData, PORTFORWARDING);
		if(portFwdArray != NULL)
		{
			__msgpack_pack_string(&pk, PORTFORWARDING, strlen(PORTFORWARDING));
			int paramCount = cJSON_GetArraySize(portFwdArray);
			//printf("paramCount : %d\n",paramCount);
			msgpack_pack_array( &pk, paramCount );
			int i = 0;
			cJSON *portFwdObj = NULL;
			for(i=0; i<paramCount; i++)
			{
				msgpack_pack_map( &pk, 6);
				portFwdObj = cJSON_GetArrayItem(portFwdArray, i);
				char *protocol = strdup(cJSON_GetObjectItem(portFwdObj, PROTOCOL)->valuestring);
				if(protocol != NULL)
				{
					//printf("protocol: %s\n", protocol);
					__msgpack_pack_string(&pk, PROTOCOL, strlen(PROTOCOL));
					__msgpack_pack_string(&pk, protocol, strlen(protocol));
				}
			
				char *description = strdup(cJSON_GetObjectItem(portFwdObj, DESCRIPTION)->valuestring);
				if(description != NULL)
				{
					//printf("description: %s\n", description);
					__msgpack_pack_string(&pk, DESCRIPTION, strlen(DESCRIPTION));
					__msgpack_pack_string(&pk, description, strlen(description));
				}
			
				char *enable = strdup(cJSON_GetObjectItem(portFwdObj, ENABLE)->valuestring);
				if(enable != NULL)
				{
					//printf("enable: %s\n", enable);
					__msgpack_pack_string(&pk, ENABLE, strlen(ENABLE));
					__msgpack_pack_string(&pk, enable, strlen(enable));
				}

				char *internalClient = strdup(cJSON_GetObjectItem(portFwdObj, INTERNAL_CLIENT)->valuestring);
				if(internalClient != NULL)
				{
					//printf("internalClient: %s\n", internalClient);
					__msgpack_pack_string(&pk, INTERNAL_CLIENT, strlen(INTERNAL_CLIENT));
					__msgpack_pack_string(&pk, internalClient, strlen(internalClient));
				}

				char *portRange = strdup(cJSON_GetObjectItem(portFwdObj, PORT_RANGE)->valuestring);
				if(portRange != NULL)
				{
					//printf("portRange: %s\n", portRange);
					__msgpack_pack_string(&pk, PORT_RANGE, strlen(PORT_RANGE));
					__msgpack_pack_string(&pk, portRange, strlen(portRange));
				}

				char *externalPort = strdup(cJSON_GetObjectItem(portFwdObj, EXTERNAL_PORT)->valuestring);
				if(externalPort != NULL)
				{
					//printf("externalPort: %s\n", externalPort);
					__msgpack_pack_string(&pk, EXTERNAL_PORT, strlen(EXTERNAL_PORT));
					__msgpack_pack_string(&pk, externalPort, strlen(externalPort));
				}
			}
		}
		if( sbuf.data )
		{
		    *encodedData = ( char * ) malloc( sizeof( char ) * sbuf.size );
		    if( NULL != *encodedData )
			{
		        memcpy( *encodedData, sbuf.data, sbuf.size );
				//printf("sbuf.data is %s sbuf.size %ld\n", sbuf.data, sbuf.size);
			}
			encodedDataLen = sbuf.size;
		}
		msgpack_sbuffer_destroy(&sbuf);
	}
	else
	{
		printf("Failed to parse JSON\n");
	}
	return encodedDataLen;
}

static void decodeMsgpackData(char *encodedData, int encodedDataLen)
{
	/* deserialize the buffer into msgpack_object instance. */
	/* deserialized object is valid during the msgpack_zone instance alive. */
	msgpack_zone mempool;
	msgpack_zone_init(&mempool, 2048);

	msgpack_object deserialized;
	msgpack_unpack(encodedData, encodedDataLen, NULL, &mempool, &deserialized);
	printf("Decoded msgpack data is \n");
	/* print the deserialized object. */
	msgpack_object_print(stdout, deserialized);
	puts("");
	msgpack_zone_destroy(&mempool);	
}

char *convertMsgpackToBlob(char *data, int size)
{
	char* b64buffer =  NULL;
	int b64bufferSize = b64_get_encoded_buffer_size( size );
	b64buffer = malloc(b64bufferSize + 1);
    if(b64buffer != NULL)
    {
        memset( b64buffer, 0, sizeof( b64bufferSize )+1 );

        b64_encode((uint8_t *)data, size, (uint8_t *)b64buffer);
        b64buffer[b64bufferSize] = '\0' ;
		//printf("blob data is \n%s\n",b64buffer);
	}
	return b64buffer;
}
int main(int argc, char *argv[])
{
	char *filename = NULL;
	char *fileData = NULL;
    size_t len = 0;
	char *encodedData = NULL, *blobData = NULL;
	int encodedDataLen = 0;

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
		printf("Json file data is \n%s\n",fileData);
	}
	else
	{
		fprintf(stderr,"File not Found\n");
	}
	printf("********* Converting json to msgpack *******\n");
	encodedDataLen = convertJsonToMsgPack(fileData, &encodedData);
	if(encodedDataLen > 0)
	{
		printf("Converting Json data to msgpack is success\n");
		printf("Converted msgpack data is \n%s\n",encodedData);
		decodeMsgpackData(encodedData, encodedDataLen);
	}
	printf("********* Converting msgpack to blob *******\n");
	blobData = convertMsgpackToBlob(encodedData, encodedDataLen);
	if(blobData)
	{
		printf("Json is converted to blob\n");
		printf("blob data is \n%s\n",blobData);
	}
	return 0;
}
