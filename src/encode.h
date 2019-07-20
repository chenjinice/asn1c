#ifndef __ENCODE_H__
#define __ENCODE_H__

#include <stdint.h>


#define  BUFF_SIZE 1792


void write_to_file(char *path, uint8_t *buffer, int length);


#endif

