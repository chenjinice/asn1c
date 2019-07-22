#ifndef __ENCODE_RSI_H__
#define __ENCODE_RSI_H__


#include <stdint.h>
#include "MessageFrame.h"

void print_rsi(MessageFrame_t *msg);
void encode_rsi(char *json_file,char *uper_file);


#endif

