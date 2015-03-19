CXX = g++
WARN = -Wall -Wpointer-arith -Wwrite-strings -Wstrict-prototypes
CXXFLAGS = -g -fexceptions
#LIBS = -LK:/libpact/lib -lpact

TARGET = core

DEV = ../../..
BUILD_DIR = $(DEV)/fbox/build
INCLUDE_DIR = $(DEV)/contrib/gnu/include
LIBS_DIR = $(DEV)/contrib/gnu/lib
BIN_DIR = $(DEV)/contrib/gnu/bin

SOURCES = $(wildcard *.cpp) # IDEALLY EXCLUDE test_  FILES
OBJECTS = $(sources:.cc=.o)

test_sources = $(wildcard test_*.cpp)
test_objects = $(sources:.cc=.test.o)


.PHONY:			all clean update install

#############################################
# Targets
#############################################

all:			core.a

core.a:			$(BUILD_DIR)/$(TARGET)/$(OBJECTS)
				mkdir -p $(BUILD_DIR)/$(TARGET)
				ar r $(BUILD_DIR)/$(TARGET)/$(TARGET).a $(BUILD_DIR)/$(TARGET)/*.o

%.o:			$(SOURCES)
				-$(CXX) -c $(CXXFLAGS) $< -o $(BUILD_DIR)/$(TARGET)/$@

test-$(TARGET):	$(test_objects)
				$(CXX) $(CXXFLAGS) $< -o $(BIN_DIR)/$@ $(LIBS)



#############################################
# Maintenance headers
#############################################

install:
				mkdir -p $(INCLUDE_DIR)/$(TARGET)
				cp *.hpp $(INCLUDE_DIR)/$(TARGET)
				cp *.h $(INCLUDE_DIR)/$(TARGET)
				cp $(BUILD_DIR)/$(CORE)/*.a $(LIB_DIR)
				cp $(BUILD_DIR)/$(CORE)/test-$(TARGET) $(BIN_DIR)
		

list:
				ls -l *.cc
		
update:
				cp makefile core.mkf
		
clean:
				rm *.o *.t

