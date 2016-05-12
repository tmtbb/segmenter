BINARYS = test_datrie

all : $(BINARYS)

INCLUDES = -I . -I ../../qmodule/qcommon/include -I ../../qmodule/transcode/include

#CMD ?= -O3
CMD ?= -g
CFLAGS = $(CMD) -Wall -fno-strict-aliasing


#-----------------------------------------------------------
test_datrie: test_datrie.cpp utility.o transcode.o
	g++ $(CFLAGS) -o $@ $^ $(INCLUDES)

.cpp.o:
	$(CC) $(CFLAGS) $(INCLUDES) -c $? -o $@

clean:
	rm -f *.o $(BINARYS)
