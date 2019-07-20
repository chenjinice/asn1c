#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include "encode.h"



void write_to_file(char *path, uint8_t *buffer, int length)
{
    int fd = open(path,O_WRONLY|O_CREAT,0666);
    if(fd == -1){
        printf("open %s failed\n",path);
        return;
    }
    int ret = write(fd,buffer,length);
    if(ret != length){
        printf("write to file error : ret = %d,length = %d\n",ret,length);
    }else{
        printf("write to %s success !\n",path);
    }
    close(fd);
}
