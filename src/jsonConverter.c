#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <msgpack.h>
#include <cJSON.h>
#include <base64.h>
#include "jsonConverter.h"
/*----------------------------------------------------------------------------*/
/*                             Function Prototypes                            */
/*----------------------------------------------------------------------------*/
static int readFromFile(const char *filename, char **data, size_t *len);
static int writeToFile(char *file_path, char *data, size_t size);
static void packJsonString( cJSON *item, msgpack_packer *pk );
static void packJsonNumber( cJSON *item, msgpack_packer *pk );
static void packJsonArray( cJSON *item, msgpack_packer *pk, int isBlob );
static void packJsonObject( cJSON *item, msgpack_packer *pk, int isBlob);
static void packJsonBool(cJSON *item, msgpack_packer *pk, bool value);
static void __msgpack_pack_string( msgpack_packer *pk, const void *string, size_t n );
static int convertJsonToBlob(char *data, char **encodedData, int isBlob);
static int convertJsonToMsgPack(char *data, char **encodedData, int isBlob);
static void decodeMsgpackData(char *encodedData, int encodedDataLen);
static int convertMsgpackToBlob(char *data, int size, char **encodedData);
static char *decodeBlobData(char *data);

static int count =0;
static int blob_count = 0;
/*----------------------------------------------------------------------------*/
/*                             External Functions                             */
/*----------------------------------------------------------------------------*/

int processEncoding(char *filename, char *encoding, int isBlob)
{
	char outFile[128] = {'\0'};
	char *fileData = NULL;
    size_t len = 0;
	char *encodedData = NULL, *temp = NULL;
	int encodedLen = 0;
	
	if(readFromFile(filename, &fileData, &len))
	{
		if(strcmp(encoding,"B") == 0)
		{
			encodedLen = convertJsonToBlob(fileData, &encodedData, isBlob);
		}
		else if(strcmp(encoding, "M") == 0)
		{
			encodedLen = convertJsonToMsgPack(fileData, &encodedData, isBlob);
		}
		if(encodedLen > 0 && encodedData != NULL)
		{
			temp = strdup(filename);
			sprintf(outFile,"%s.bin",strtok(temp, "."));
			printf("Encoding is success. Hence writing encoded data to %s\n",outFile);
			if(writeToFile(outFile, encodedData, encodedLen) == 0)
			{
				fprintf(stderr,"%s File not Found\n", outFile);
				FREE(encodedData);
				FREE(temp);
				return 0;
			}
			FREE(encodedData);
			FREE(temp);
		}
		FREE(fileData);
	}
	else
	{
		fprintf(stderr,"%s File not Found\n",filename);
		return 0;
	}
	return 1;
}
/*----------------------------------------------------------------------------*/
/*                             Internal Functions                             */
/*----------------------------------------------------------------------------*/

static int readFromFile(const char *filename, char **data, size_t *len)
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

static int writeToFile(char *file_path, char *data, size_t size)
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

static int getItemsCount(cJSON *object)
{
	int count = 0;
	while(object != NULL)
	{
		object = object->next;
		count++;
	}
	return count;
}
static void __msgpack_pack_string( msgpack_packer *pk, const void *string, size_t n )
{
    msgpack_pack_str( pk, n );
    msgpack_pack_str_body( pk, string, n );
}

static void packJsonString( cJSON *item, msgpack_packer *pk )
{
	__msgpack_pack_string(pk, item->string, strlen(item->string));
	__msgpack_pack_string(pk, item->valuestring, strlen(item->valuestring));
}

static void packJsonNumber( cJSON *item, msgpack_packer *pk )
{
	__msgpack_pack_string(pk, item->string, strlen(item->string));
	msgpack_pack_int(pk, item->valueint);
}

static void packJsonBool(cJSON *item, msgpack_packer *pk, bool value)
{
	__msgpack_pack_string(pk, item->string, strlen(item->string));
	if(value)
	{
		msgpack_pack_true(pk);
	}
	else
	{
		msgpack_pack_false(pk);
	}
}
static void packJsonArray(cJSON *item, msgpack_packer *pk, int isBlob)
{
	int arraySize = cJSON_GetArraySize(item);
	//printf("%s:%s\n",__FUNCTION__, item->string);
	if(item->string != NULL && (isBlob == 0 || (strcmp(item->string,"parameters") == 0)))
	{
		//printf("packing %s\n",item->string);
		msgpack_pack_map( pk, 1);
		__msgpack_pack_string(pk, item->string, strlen(item->string));
		count++;
	}
	else if(count >0)
	{
		__msgpack_pack_string(pk, item->string, strlen(item->string));
	}
	msgpack_pack_array( pk, arraySize );
	int i=0;
	for(i=0; i<arraySize; i++)
	{
		cJSON *arrItem = cJSON_GetArrayItem(item, i);
		switch((arrItem->type) & 0XFF)
		{
			case cJSON_True:
				packJsonBool(arrItem, pk, true);
				break;
			case cJSON_False:
				packJsonBool(arrItem, pk, false);
				break;
			case cJSON_String:
				packJsonString(arrItem, pk);
				break;
			case cJSON_Number:
				packJsonNumber(arrItem, pk);
				break;
			case cJSON_Array:
				packJsonArray(arrItem, pk, isBlob);
				break;
			case cJSON_Object:
				packJsonObject(arrItem, pk, isBlob);
				break;
		}
	}
}

int getEncodedBlob(char *data, char **encodedData)
{
	cJSON *jsonData=NULL;
	int encodedDataLen = 0;
	printf("------- %s -------\n",__FUNCTION__);
	jsonData=cJSON_Parse(data);
	if(jsonData != NULL)
	{
		msgpack_sbuffer sbuf1;
		msgpack_packer pk1;
		msgpack_sbuffer_init( &sbuf1 );
		msgpack_packer_init( &pk1, &sbuf1, msgpack_sbuffer_write );
		msgpack_pack_map( &pk1, getItemsCount(jsonData));
		blob_count = 1;
		packJsonArray(jsonData->child, &pk1, 1);
		if( sbuf1.data )
		{
		    *encodedData = ( char * ) malloc( sizeof( char ) * sbuf1.size );
		    if( NULL != *encodedData )
			{
		        memcpy( *encodedData, sbuf1.data, sbuf1.size );
			}
			encodedDataLen = sbuf1.size;
		}
		msgpack_sbuffer_destroy(&sbuf1);
		cJSON_Delete(jsonData);
	}
	else
	{
		printf("Failed to parse JSON\n");
	}
	printf("------- %s -------\n",__FUNCTION__);
	return encodedDataLen;
}
static void packBlobData(cJSON *item, msgpack_packer *pk )
{
	char *blobData = NULL, *encodedBlob = NULL;
	int len = 0;
	printf("------ %s ------\n",__FUNCTION__);
	blobData = (char *)malloc(sizeof(char) * (strlen(item->valuestring)+(strlen(item->string))+5));
	sprintf(blobData, "{\"%s\":%s}",item->string,item->valuestring);
	printf("%s\n",blobData);
	len = getEncodedBlob(blobData, &encodedBlob);
	printf("%s\n",encodedBlob);
	//__msgpack_pack_string(pk, item->string, strlen(item->string));
	__msgpack_pack_string(pk, encodedBlob, len);
	FREE(encodedBlob);
	FREE(blobData);
	printf("------ %s ------\n",__FUNCTION__);
	blob_count = 0;

}

static void packJsonObject( cJSON *item, msgpack_packer *pk, int isBlob )
{
	//printf("%s\n",__FUNCTION__);
	if(item->string != NULL)
	{
		__msgpack_pack_string(pk, item->string, strlen(item->string));
	}
	cJSON *child = item->child;
	if((blob_count == 1)||(item->child->string != NULL && (strcmp(item->child->string, "name") == 0)))
	{
		msgpack_pack_map( pk, getItemsCount(child));
	}
	while(child != NULL)
	{
		switch((child->type) & 0XFF)
		{
			case cJSON_True:
				packJsonBool(child, pk, true);
				break;
			case cJSON_False:
				packJsonBool(child, pk, false);
				break;
			case cJSON_String:
				if(item->string != NULL && (strcmp(item->string, "value") == 0) && isBlob == 1)
				{
					packBlobData(child, pk);
				}
				else
				{
					packJsonString(child, pk);
				}
				break;
			case cJSON_Number:
				packJsonNumber(child, pk);
				break;
			case cJSON_Array:
				packJsonArray(child, pk, isBlob);
				break;
			case cJSON_Object:
				packJsonObject(child, pk, isBlob);
				break;
		}
		child = child->next;
	}
}

static int convertJsonToMsgPack(char *data, char **encodedData, int isBlob)
{
	cJSON *jsonData=NULL;
	int encodedDataLen = 0;
	jsonData=cJSON_Parse(data);
	if(jsonData != NULL)
	{
		msgpack_sbuffer sbuf;
		msgpack_packer pk;
		msgpack_sbuffer_init( &sbuf );
		msgpack_packer_init( &pk, &sbuf, msgpack_sbuffer_write );
		packJsonObject(jsonData, &pk, isBlob);
		if( sbuf.data )
		{
		    *encodedData = ( char * ) malloc( sizeof( char ) * sbuf.size );
		    if( NULL != *encodedData )
			{
		        memcpy( *encodedData, sbuf.data, sbuf.size );
			}
			encodedDataLen = sbuf.size;
		}
		msgpack_sbuffer_destroy(&sbuf);
		cJSON_Delete(jsonData);
	}
	else
	{
		printf("Failed to parse JSON\n");
	}
	return encodedDataLen;
}

static int convertJsonToBlob(char *data, char **encodedData, int isBlob)
{
	char *msgpackData = NULL, *blobData = NULL, *decodedBlob = NULL;
	int msgpackDataLen = 0, blobDataLen = 0;
	printf("********* Converting json to msgpack *******\n");
	msgpackDataLen = convertJsonToMsgPack(data, &msgpackData, isBlob);
	if(msgpackDataLen > 0)
	{
		printf("Converting Json data to msgpack is success\n");
		printf("Converted msgpack data is \n%s\n",msgpackData);
		decodeMsgpackData(msgpackData, msgpackDataLen);
	}
	else
	{
		printf("Failed to encode json data to msgpack\n");
		return 0;
	}
	printf("********* Converting msgpack to blob *******\n");
	blobDataLen = convertMsgpackToBlob(msgpackData, msgpackDataLen, &blobData);
	if(blobDataLen>0)
	{
		printf("Json is converted to blob\n");
		printf("blob data is \n%s\n",blobData);
		*encodedData = blobData;
		decodedBlob = decodeBlobData(blobData);
		printf("Decoded blob data is \n%s\n",decodedBlob);
	}
	else
	{
		printf("Failed to encode msgpack data to blob\n");
		FREE(msgpackData);
		return 0;
	}
	if(strcmp(msgpackData, decodedBlob) == 0)
	{
		printf("Encoded msgpack data and decoded blob data are equal\n");
	}
	FREE(msgpackData);
	return blobDataLen;
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

static int convertMsgpackToBlob(char *data, int size, char **encodedData)
{
	char* b64buffer =  NULL;
	int b64bufferSize = b64_get_encoded_buffer_size( size );
	b64buffer = malloc(b64bufferSize + 1);
    if(b64buffer != NULL)
    {
        memset( b64buffer, 0, sizeof( b64bufferSize )+1 );

        b64_encode((uint8_t *)data, size, (uint8_t *)b64buffer);
        b64buffer[b64bufferSize] = '\0' ;
		*encodedData = b64buffer;
	}
	return b64bufferSize;
}

static char *decodeBlobData(char *data)
{
	int size = 0;
	char *decodedData = NULL;
    size = b64_get_decoded_buffer_size(strlen(data));
    decodedData = (char *) malloc(sizeof(char) * size);
    if(decodedData)
    {
		memset( decodedData, 0, sizeof(char) *  size );
		size = b64_decode( (const uint8_t *)data, strlen(data), (uint8_t *)decodedData );
		decodeMsgpackData(decodedData, size);
	}
	return decodedData;
}

