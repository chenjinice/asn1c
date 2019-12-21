#ifndef __COMMON_H__
#define __COMMON_H__


#include "cJSON.h"
#include "MessageFrame.h"


#define MY_INFO 0
#define MY_OK   1
#define MY_ERR  2
#define mylog(format,...) myprint(MY_INFO,format,##__VA_ARGS__)
#define myok(format,...) myprint(MY_OK,format,##__VA_ARGS__)
#define myerr(format,...) myprint(MY_ERR,format,##__VA_ARGS__)

#define BUFF_SIZE       100000


uint8_t byteReverse(uint8_t data);
void myprint(int type, const char *format,...);
char *getPreSuf(int level, char *key);

// file
int getFileSize(char *path);
int readFile(char *path,uint8_t **buffer);
void writeToFile(char *path, uint8_t *buffer, int length);
cJSON *readJson(char *path);



// encode  &  decode
void decodePerFile(char *per_file,int flag);
void encode(char *path, MessageFrame_t *msg);
void encodeJsonFile(char *json_file,char *per_file);

#endif



