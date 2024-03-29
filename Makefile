# Please tweak it to suit your environment and project.  You
# may want to move it to your project's root directory.
#
# SYNOPSIS:
#
#   make [all]  - makes everything.
#   make TARGET - makes the given target.
#   make clean  - removes all files generated by make.


# Where to find user code.

TARGET_DIR = ./../bin

#THIRD_PATH = ./../../mylib


LINK_PATHS = -L./

# Flags passed to the preprocessor.
CPPFLAGS += -isystem 

# Flags passed to the C++ compiler.
CXXFLAGS += -g  -Wextra -pthread -Wall -D_REENTRANT -D_GNU_SOURCE -DDEBUG -D_DEBUG

# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.
TAGET_FILE = $(TARGET_DIR)/dict_runner
                
INCLUDE_PATHS = -I/usr/local/include\
		-I./*.h\
		-I../re2/include\


# House-keeping build targets.

ALL_HEADERS = $(USER_HEADERS)
all : $(TAGET_FILE)

clean :
	rm -f $(TAGET_FILE)  *.o

# Builds gtest.a and gtest_main.a.

# Usually you shouldn't tweak such internal variables, indicated by a
# trailing _.
#GTEST_SRCS_ = $(GTEST_DIR)/src/*.cc $(GTEST_DIR)/src/*.h $(GTEST_HEADERS)

# For simplicity and to avoid depending on Google Test's
# implementation details, the dependencies specified below are
# conservative and not optimized.  This is fine as Google Test
# compiles fast and for ordinary users its source rarely changes.
#gtest-all.o : $(GTEST_SRCS_)
#	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest-all.cc

#gtest_main.o : $(GTEST_SRCS_)
#	$(CXX) $(CPPFLAGS) -I$(GTEST_DIR) $(CXXFLAGS) -c \
            $(GTEST_DIR)/src/gtest_main.cc

#gtest.a : gtest-all.o
#	$(AR) $(ARFLAGS) $@ $^

#gtest_main.a : gtest-all.o gtest_main.o
#	$(AR) $(ARFLAGS) $@ $^

# Builds a sample test.  A test should link with either gtest.a or
# gtest_main.a, depending on whether it defines its own main()
# function.

relrank_dict.o : ./relrank_dict.cpp ./relrank_dict.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./relrank_dict.cpp $(INCLUDE_PATHS)

transcode.o : ./largetrie/transcode.cpp ./largetrie/transcode.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./largetrie/transcode.cpp $(INCLUDE_PATHS)

utility.o : ./largetrie/utility.cpp ./largetrie/utility.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./largetrie/utility.cpp $(INCLUDE_PATHS)

dict_runner.o : ./dict_runner.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./dict_runner.cpp $(INCLUDE_PATHS)

dfa.o: ./dfa.cpp ./dfa.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./dfa.cpp $(INCLUDE_PATHS)

atom_recognizer.o : ./atom_recognizer.cpp ./atom_recognizer.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./atom_recognizer.cpp $(INCLUDE_PATHS)
	
	
$(TAGET_FILE) : relrank_dict.o transcode.o utility.o dict_runner.o dfa.o atom_recognizer.o\
	/usr/local/lib/libpcre2-8.a /usr/local/lib/libpcre2-32.a
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LINK_PATHS)  -lpthread -lrt -lpthread \
	$^ -o $@
