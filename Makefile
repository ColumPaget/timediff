OBJ=parse.o stats.o settings.o help.o libUseful-5/libUseful.a
FLAGS=-g -O2 -DPACKAGE_NAME=\"timediff\" -DPACKAGE_TARNAME=\"timediff\" -DPACKAGE_VERSION=\"1.0\" -DPACKAGE_STRING=\"timediff\ 1.0\" -DPACKAGE_BUGREPORT=\"colums.projects@gmail.com\" -DPACKAGE_URL=\"\" -DSTDC_HEADERS=1 -D_FILE_OFFSET_BITS=64 -DHAVE_LIBZ=1 -DHAVE_LIBCRYPTO=1 -DHAVE_LIBSSL=1 -DHAVE_LIBCAP=1
LIBS=-lcap -lssl -lcrypto -lz 
STATIC_LIBS=libUseful-5/libUseful.a
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
