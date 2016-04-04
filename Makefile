EXE = vstab

CC = gcc
# CC = clang

OBJECTS = \
	main.o vector.o resample.o utils.o motion.o average.o \
	avi/avi.o avi/avirgb.o avi/codecs.o avi/endian.o \
	avi/mjpeg.o avi/options.o

CFLAGS = -g -O3 -Wall -march=native
# CFLAGS = -g -O1 -fsanitize=address -fno-omit-frame-pointer

LDFLAGS = -ljpeg -lm

all: $(EXE)

$(EXE) : $(OBJECTS)
	$(CC) $(CFLAGS) -o $(EXE) $(OBJECTS) $(LDFLAGS)

clean :
	rm -f *.o *~ avi/*.o avi/*~ $(EXE)

dep :
	$(CC) $(CFLAGS) -MM $(OBJECTS:.o=.c) >$(EXE).dep

-include $(EXE).dep
