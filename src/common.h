#ifndef __COMMON_H__
#define __COMMON_H__

#include "cJSON.h"
#include "MessageFrame.h"


#define  BUFF_SIZE 65535

int get_file_size(char *path);
int read_file(char *path,uint8_t **buffer);
void write_file(char *path, uint8_t *buffer, int length);
cJSON *read_json(char *path);
void encode(char *path, MessageFrame_t *msg);
MessageFrame_t *decode(char *path);


#endif



