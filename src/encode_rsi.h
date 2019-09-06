#ifndef __ENCODE_RSI_H__
#define __ENCODE_RSI_H__


#include <stdint.h>
#include "cJSON.h"
#include "MessageFrame.h"

void print_rsi(MessageFrame_t *msg);
void encode_rsi(cJSON *json,char *uper_file);


#endif

