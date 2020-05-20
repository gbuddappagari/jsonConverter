#ifndef _JSONCONVERTER_H
#define _JSONCONVERTER_H

#define FREE(__x__) if(__x__ != NULL) { free((void*)(__x__)); __x__ = NULL;} else {printf("Trying to free null pointer\n");}
int processEncoding(char *filename, char *encoding);
#endif
