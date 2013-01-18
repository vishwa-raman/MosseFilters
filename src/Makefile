# Makefile for ADM

CC = g++
AR = ar
LD = ld
RANLIB = ranlib
RM = /bin/rm
MKDIR = /bin/mkdir
CP = /bin/cp
RM = /bin/rm

#makedepend flags
DFLAGS = 

# SVM path
SVM_PATH = ../svm_light

#Compiler flags
#if mode variable is empty, setting debug build mode
ifeq ($(mode),opt)
   CFLAGS = -Wall -O3 -fPIC -shared -fopenmp
   BUILD_DIR = ../build/incar_gaze/src.opt
else
   mode = debug
   CFLAGS = -g -Wall -fPIC -shared -DSINGLETHREADED
   BUILD_DIR = ../build/incar_gaze/src
endif

CFILES = Globals.cpp Location.cpp Filter.cpp OnlineFilter.cpp Annotations.cpp Feature.cpp Trainer.cpp Classifier.cpp GazeTracker.cpp

OFILES = Globals.o Location.o Filter.o OnlineFilter.o Annotations.o Feature.o Trainer.o Classifier.o GazeTracker.o

INSTALL_DIR = ../install/lib
HEADER_DIR = ../install/include
BIN_DIR = ../install/bin

TRACK_LIB = libtrack.a

TRACK_INCLUDE = Annotations.h Classifier.h FeatureBase.h Feature.h FeatureLNAngle.h FeatureLNDist.h \
	FeatureLRDist.h FeatureLX.h FeatureRNDist.h FeatureRX.h \
	FilterBase.h FeatureNX.h FeatureRNAngle.h FeatureLRNArea.h \
	Filter.h OnlineFilter.h GazeTracker.h Globals.h LocationBase.h \
	Location.h Trainer.h

OUT = $(INSTALL_DIR)/libtrack.a

INCLUDES = -I ./ `pkg-config opencv --cflags` `pkg-config fftw3 --cflags` -I ${SVM_PATH}
LIBS = `pkg-config opencv --cflags --libs` `pkg-config fftw3 --cflags --libs`

OBJS := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(filter %.cpp,$(CFILES)))

#OBJS = $(patsubst %,$(BUILD_DIR)/%,$(OFILES))

.phony:all header

all: information $(OUT)

information:
ifneq ($(mode),opt)
ifneq ($(mode),debug)
	@echo "Invalid build mode." 
	@echo "Please use 'make mode=opt' or 'make mode=debug'"
	@exit 1
endif
endif
	@echo "Building on "$(mode)" mode"
	@echo ".........................."

$(BUILD_DIR)/%.o: %.cpp $(TRACK_INCLUDE)
	$(MKDIR) -p $(BUILD_DIR)	
	$(CC) -c $(INCLUDES) -o $@ $< $(CFLAGS) 

$(OUT): $(OBJS) 
	$(MKDIR) -p $(INSTALL_DIR)	
	$(MKDIR) -p $(HEADER_DIR)	
	$(AR) rcs $(OUT) $^ ${SVM_PATH}/svm_common.o
	$(RANLIB) $(OUT)
	$(CP) -p $(TRACK_INCLUDE) $(HEADER_DIR)
	@echo track finished

header:
	$(MKDIR) -p $(HEADER_DIR)	
	$(CP) -p $(TRACK_INCLUDE) $(HEADER_DIR)
	@echo header finished

depend:
	makedepend -- $(DFLAGS) -- $(CFILES)

.PHONY: clean

clean:
	$(RM) -f $(BUILD_DIR)/*.o $(OUT)

