

TARGET = libltev_asn_2020.a
DIR = ./src
SRC = $(wildcard $(DIR)/*.c)
OBJS = $(SRC:%.c=%.o)

INC += -I $(DIR)


all:$(OBJS)
	ar -crv $(TARGET) $^

mv:
	mv $(DIR)/$(TARGET) ./

%.o:%.c
	gcc -c $< -o $@ $(INC)

clean:
	-rm  $(OBJS)

cleanall:clean
	-rm $(DIR)/*.c $(DIR)/*.h

regenerate:
# asn1c 版本 asn1c-0.9.29 , 生成的文件注释里有写了asn1c的版本和命令等信息
#	asn1c -fskeletons-copy -fnative-types -gen-PER -pdu=auto -no-gen-example LTEV.asn -D $(DIR)
	asn1c -fskeletons-copy -fnative-types -gen-PER -pdu=auto LTEV.asn -D $(DIR)
	-rm $(DIR)/Makefile.am.*
	-rm $(DIR)/converter-example.*
	






