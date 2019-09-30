#ifndef __ENCODE_RSI_H__
#define __ENCODE_RSI_H__


#include <stdint.h>
#include "cJSON.h"
#include "MessageFrame.h"

void rsiPrint(MessageFrame_t *msg);
void rsiEncode(cJSON *json,char *uper_file);


#endif

