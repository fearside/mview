#
# mview's Makefile
#

#CC	= cc
CC	= gcc
#COMP 	= compress
COMP 	= gzip
DEBUG	= -DDEBUG
LARGE	= -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE
DATE	= `date +%Y%m%d`
OPTIM	= -O
#CFLAGS	= -pg ${OPTIM} ${DEBUG}
#CFLAGS	= -g -Wall ${OPTIM}
CFLAGS	= -g -Wall -std=c99 ${OPTIM} ${DEBUG} ${LARGE}
LDFLAGS	= # -static
INCS	= mview.h
OBJS	= sys_err.o \
	  getlog.o \
	  mview.o
SRCS	= sys_err.c \
	  getlog.c \
	  mview.c

TARGET	= mview


all:${TARGET}

TAGS:
	etags *.c *.h

${TARGET}:${OBJS}
	${CC} ${CFLAGS} ${LDFLAGS} -o $@ $^

touch:
	touch *.c

.c.o:

.h.c:

clean: clean-getlog
	rm -f core *.exe.stackdump *.o *.exe ${TARGET} gmon.out

clean-getlog:
	rm -f getlog getlog.txt

#
# test suite
#
test: getlog test-all

getlog: getlog.c sys_err.c
	${CC} ${CFLAGS} -DDEBUG_GETLOG -o $@ $^


test-all: test-getlog test-storeword

test-getlog:
	@/bin/echo " --- start getlog test ==> \c"
	@./getlog ./Test/getlog.in1 > ./Test/.result.getlog.out1
	@./getlog ./Test/getlog.in2 > ./Test/.result.getlog.out2
	@diff -c ./Test/.result.getlog.out1 ./Test/.result.getlog.out2 > /dev/null
	@/bin/echo "successfully done --- "


# end of makefile
