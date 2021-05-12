#ifndef __MAP_ABOUT_H__
#define __MAP_ABOUT_H__

#include <stdint.h>
#include "cJSON.h"
#include "MessageFrame.h"


int  mapJsonCheck(cJSON *json);
void mapEncode(cJSON *json, char *uper_file);



#endif

