# Please tweak it to suit your environment and project.  You
# may want to move it to your project's root directory.
#
# SYNOPSIS:
#
#   make [all]  - makes everything.
#   make TARGET - makes the given target.
#   make clean  - removes all files generated by make.


# Where to find user code.

LIB_DIR = ../library
LIB_DIR_O = ../../../lib
PUB_DIR = ../pub
SO_DIR = ./../kid/bin

THIRD_PATH = $(LIB_DIR)/mylib/public

CUSTOM_THIRD_PATH = $(LIB_DIR)/mylib/third_party

LINK_PATHS = -L./ -L/usr/local/libmemcached-1.0.7/lib/  -L/usr/lib64/mysql

# Flags passed to the preprocessor.
CPPFLAGS += -isystem -D_DEBUG -DUSR_LOG_MSG -D_REENTRANT -D_GNU_SOURCE -DUSR_LOG_ERROR -DUSR_LOG_MSG -DNET_WORK -DBRY_PTL

# Flags passed to the C++ compiler.
CXXFLAGS += -g -Wextra -Wall -pthread

# All tests produced by this Makefile.  Remember to add new tests you
# created to the list.
MAIN = $(SO_DIR)/loader.out
                
INCLUDE_PATHS = -I/usr/local/libmemcached-1.0.7/include \
				-I$(LIB_DIR)/BasicCore\
				-I$(LIB_DIR)/mylib/public\
				-I./../../../src\
				-I$(PUB_DIR)\
				-I$(THIRD_PATH)\
				-I/usr/local/include\
				-I/usr/include/mysql\
				-I$(USER_DIR)\
				-I$(CUSTOM_THIRD_PATH)/hiredis\
				-I$(CUSTOM_THIRD_PATH)\
				-I$(LIB_DIR)/mylib/base\
				-I/usr/local/include/python2.7/\


# House-keeping build targets.

ALL_HEADERS = $(USER_HEADERS)
all : $(MAIN)

clean :
	rm -f $(MAIN)  *.o

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

loader.o : ./loader.cc
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./loader.cc $(INCLUDE_PATHS) 

native_library.o : ./$(LIB_DIR)/mylib/public/basic/native_library.cc ./$(LIB_DIR)/mylib/public/basic/native_library.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./$(LIB_DIR)/mylib/public/basic/native_library.cc $(INCLUDE_PATHS)
	
mig_log.o : ./$(LIB_DIR)/mylib/public/log/mig_log.cc ./$(LIB_DIR)/mylib/public/log/mig_log.h
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c ./$(LIB_DIR)/mylib/public/log/mig_log.cc $(INCLUDE_PATHS)

	

$(TEST) : 

$(MAIN) : loader.o native_library.o mig_log.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LINK_PATHS) -lutil -lpython2.7 -lpthread -lexpat -lrt -lpthread -ldl -lcurl \
	$^ -o $@
