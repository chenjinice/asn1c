#ifndef __ENCODE_MAP_H__
#define __ENCODE_MAP_H__

#include <stdint.h>
#include "MessageFrame.h"

void print_map(MessageFrame_t *msg);
void encode_map(char *json_file,char *uper_file);

#endif

