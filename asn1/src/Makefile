

TARGET = libltev_asn1.a
SRC = $(wildcard *.c)
OBJS = $(SRC:%.c=%.o)

INC += -I ./


all:$(OBJS)
	ar -crv $(TARGET) $^

mv:
	mv $(TARGET) ../

%.o:%.c
	gcc -c $< -o $@ $(INC)

clean:
	-rm  $(OBJS)

cleanall:clean
	-rm *.c *.h

regenerate:
#	asn1c -fskeletons-copy -fnative-types -gen-PER -pdu=auto -no-gen-example LTEV.asn
	asn1c -fskeletons-copy -fnative-types -gen-PER -pdu=auto LTEV.asn
	-rm Makefile.am.*
	-rm converter-example.*
	






