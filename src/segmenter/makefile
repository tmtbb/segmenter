TARGET_D = dict_manager_debug
TARGET_O = dict_manager



OBJS_D = relrank_dict_debug.o \
		 ./largetrie/utility_debug.o \
		 ./largetrie/transcode_debug.o \
		 dict_test_debug.o \
		 atom_recognizer.o \
		 dfa.o \
		 
OBJS_O = relrank_dict.o \
		 ./largetrie/utility.o \
		 ./largetrie/transcode.o \
		 dict_test.o \
		 atom_recognizer.o \
		 dfa.o \
		   

CC = gcc
CPP = g++
CFLAGS_COMMON = -Wall -D_REENTRANT
#-L

CFLAGS_O = $(CFLAGS_COMMON) -O3 -g
CFLAGS_D = $(CFLAGS_COMMON) -g

INCLUDES = -I./*.h -I$(HOME)/qmodule/qcommon/include -I$(HOME)/qmodule/transcode/include -I../re2/include/ -I/home/gongyingkun/pcre2-10.20/bin/include/
LIBS_O = -L../qmodule/transcode/lib -ltranscode -lboost
LIBS_D = ../re2/lib/libre2.a -lpthread  /home/gongyingkun/pcre2-10.20/bin/lib/libpcre2-8.a /home/gongyingkun/pcre2-10.20/bin/lib/libpcre2-32.a

%.o : %.c
	$(CC) $(CFLAGS_O) $(INCLUDES) -c -o $@ $<

%_debug.o : %.c
	$(CC) $(CFLAGS_D) $(INCLUDES) -c -o $@ $<

%.o : %.cpp
	$(CPP) $(CFLAGS_O) $(INCLUDES) -c -o $@ $<
	
%_debug.o : %.cpp
	$(CPP) $(CFLAGS_D) $(INCLUDES) -c -o $@ $<

all: $(TARGET_D) $(TARGET_O)
	
$(TARGET_D): $(OBJS_D)
	$(CPP) -g -o $@ $^ $(INCLUDES) $(LIBS_D)
	
$(TARGET_O): $(OBJS_O)
	$(CPP) -g -o $@ $^ $(INCLUDES) $(LIBS_D)

	cp $(TARGET_D) ../bin/
	cp $(TARGET_O) ../bin/
clean:
	rm -f $(TARGET_D) $(TARGET_O)
	rm -f *.o

# end of file

