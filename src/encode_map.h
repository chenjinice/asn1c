#ifndef __ENCODE_MAP_H__
#define __ENCODE_MAP_H__

#include <stdint.h>
#include "cJSON.h"
#include "MessageFrame.h"

void print_map(MessageFrame_t *msg);
void encode_map(cJSON *json, char *uper_file);

#endif

