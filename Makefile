# Example Makefile for ROSE users
# This makefile is provided as an example of how to use ROSE when ROSE is
# installed (using "make install").  This makefile is tested as part of the
# "make distcheck" rule (run as part of tests before any SVN checkin).
# The test of this makefile can also be run by using the "make installcheck"
# rule (run as part of "make distcheck").


# Location of include directory after "make install"
ROSE_INCLUDE_DIR = /mnt/netapp/home2/nchaimov/src/rose-0.9.5a-15674/compileTreeNoJava/include

# Location of Boost include directory
BOOST_CPPFLAGS = -I/mnt/netapp/home2/nchaimov/boost/include

# Location of Dwarf include and lib (if ROSE is configured to use Dwarf)
ROSE_DWARF_INCLUDES = /mnt/netapp/home2/nchaimov/src/dwarf-20110612/libdwarf
ROSE_DWARF_LIBS_WITH_PATH = /mnt/netapp/home2/nchaimov/src/dwarf-20110612/libdwarf

# Location of library directory after "make install"
ROSE_LIB_DIR =  /mnt/netapp/home2/nchaimov/src/rose-0.9.5a-15674/compileTreeNoJava/lib

ROSE_HOME =  /mnt/netapp/home2/nchaimov/src/rose-0.9.5a-15674/compileTreeNoJava

CC                    = gcc
CXX                   = g++
CPPFLAGS              = $(BOOST_CPPFLAGS) -I$(ROSE_DWARF_INCLUDES)
#CXXCPPFLAGS           = @CXXCPPFLAGS@
CXXFLAGS              = -gdwarf-2 -g3 -Wall -DDEBUG
LDFLAGS               = -L$(ROSE_DWARF_LIBS_WITH_PATH) -L/mnt/netapp/home2/nchaimov/boost/lib -L/mnt/netapp/home2/nchaimov/lib -static -pthread -Wl,--start-group -lpthread -lboost_system -lboost_wave -lhpdf -lrose -lm -lboost_date_time -lboost_thread -lboost_filesystem -lgcrypt -lgpg-error -lboost_program_options -lboost_regex -lelf -ldwarf -Wl,--end-group 

#ROSE_LIBS = $(ROSE_LIB_DIR)/librose.la

# Location of source code
ROSE_SOURCE_DIR = ./src
 

executableFiles = printRoseAST undwarf readtest


# Default make rule to use
all: $(executableFiles)

clean:
	rm -f $(executableFiles) *.o


dlstubs.o: $(ROSE_SOURCE_DIR)/dlstubs.c
	$(CC) -o $@ -c $(ROSE_SOURCE_DIR)/dlstubs.c -ldl

printRoseAST: $(ROSE_SOURCE_DIR)/printRoseAST.cpp dlstubs.o
	$(CXX) -I$(ROSE_INCLUDE_DIR) $(CPPFLAGS) $(CXXFLAGS) -o $@ $(ROSE_SOURCE_DIR)/printRoseAST.cpp dlstubs.o $(LIBS_WITH_RPATH) -L$(ROSE_LIB_DIR) $(LDFLAGS) -z muldefs  

readtest: $(ROSE_SOURCE_DIR)/readtest.cpp dlstubs.o
	$(CXX) -I$(ROSE_INCLUDE_DIR) $(CPPFLAGS) $(CXXFLAGS) -o $@ $(ROSE_SOURCE_DIR)/readtest.cpp dlstubs.o $(LIBS_WITH_RPATH) -L$(ROSE_LIB_DIR) $(LDFLAGS) -z muldefs  

undwarf.o: $(ROSE_SOURCE_DIR)/undwarf.cpp $(ROSE_SOURCE_DIR)/typeTable.h
	$(CXX) -I$(ROSE_INCLUDE_DIR) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $(ROSE_SOURCE_DIR)/undwarf.cpp

typeTable.o: $(ROSE_SOURCE_DIR)/typeTable.cpp $(ROSE_SOURCE_DIR)/typeTable.h
	$(CXX) -I$(ROSE_INCLUDE_DIR) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $(ROSE_SOURCE_DIR)/typeTable.cpp  

DwarfROSEConverter.o: $(ROSE_SOURCE_DIR)/DwarfROSEConverter.cpp $(ROSE_SOURCE_DIR)/DwarfROSEConverter.h $(ROSE_SOURCE_DIR)/attributes.h
	$(CXX) -I$(ROSE_INCLUDE_DIR) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $(ROSE_SOURCE_DIR)/DwarfROSEConverter.cpp  

attributes.o: $(ROSE_SOURCE_DIR)/attributes.cpp $(ROSE_SOURCE_DIR)/attributes.h
	$(CXX) -I$(ROSE_INCLUDE_DIR) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $(ROSE_SOURCE_DIR)/attributes.cpp  

sageUtils.o: $(ROSE_SOURCE_DIR)/sageUtils.cpp $(ROSE_SOURCE_DIR)/sageUtils.h
	$(CXX) -I$(ROSE_INCLUDE_DIR) $(CPPFLAGS) $(CXXFLAGS) -o $@ -c $(ROSE_SOURCE_DIR)/sageUtils.cpp  

undwarf: undwarf.o typeTable.o DwarfROSEConverter.o attributes.o dlstubs.o sageUtils.o
	$(CXX) -I$(ROSE_INCLUDE_DIR) $(CPPFLAGS) $(CXXFLAGS) -o $@ $+ $(LIBS_WITH_RPATH) -L$(ROSE_LIB_DIR) $(LDFLAGS) 
