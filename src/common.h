#ifndef __COMMON_H__
#define __COMMON_H__

#include "cJSON.h"
#include "MessageFrame.h"


#define  BUFF_SIZE 1792


void encode(char *path, MessageFrame_t *msg);
void write_to_file(char *path, uint8_t *buffer, int length);
void decode_from_file(char *path);
cJSON *read_json(char *path);


#endif



