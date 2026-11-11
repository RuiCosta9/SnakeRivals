PROG=lab5

SRCS = lab5.c

CFLAGS += -pedantic
CPPFLAGS += -I../lab3 -I../lab2

LIB_NAME = libvideo.a
LIB_SRCS = video.c vbe.c
LIB_OBJS = ${LIB_SRCS:.c=.o}

DPADD += ${LIB_NAME} ../lab3/libkbd.a ${LIBLCF}
LDADD += -L. -lvideo -L../lab3 -lkbd -llcf

CLEANFILES += ${LIB_NAME} ${LIB_OBJS}

all: lib

lib: ${LIB_OBJS}
	ar rcs ${LIB_NAME} ${LIB_OBJS}

.include <minix.lcom.mk>