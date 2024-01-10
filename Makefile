OBJ=parse.o stats.o settings.o help.o 
FLAGS=-g -O2 -DPACKAGE_NAME=\"timediff\" -DPACKAGE_TARNAME=\"timediff\" -DPACKAGE_VERSION=\"1.1\" -DPACKAGE_STRING=\"timediff\ 1.1\" -DPACKAGE_BUGREPORT=\"colums.projects@gmail.com\" -DPACKAGE_URL=\"\" -DHAVE_STDIO_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_STRINGS_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_UNISTD_H=1 -DSTDC_HEADERS=1 -DHAVE_LIBZ=1 -DHAVE_LIBUSEFUL_5_LIBUSEFUL_H=1 -DHAVE_LIBUSEFUL_5=1
LIBS=-lUseful-5 -lz 
STATIC_LIBS=
PREFIX=/usr/local

all: $(OBJ)
	gcc $(FLAGS) -otimediff $(OBJ) main.c $(LIBS) $(STATIC_LIBS)

stats.o: stats.h stats.c
	gcc $(FLAGS) -c stats.c

parse.o: parse.h parse.c
	gcc $(FLAGS) -c parse.c

settings.o: settings.h settings.c
	gcc $(FLAGS) -c settings.c

help.o: help.h help.c
	gcc $(FLAGS) -c help.c

libUseful-5/libUseful.a:
	make -C libUseful-5


clean:
	rm -rf timediff *.o */*.o */*.so */*.a *.orig


install:
	cp -f timediff $(PREFIX)/bin
