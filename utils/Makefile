CC = g++
AR = ar
LD = ld
RANLIB = ranlib
RM = /bin/rm
MKDIR = /bin/mkdir
CP = /bin/cp
RM = /bin/rm

SVM_PATH = ../svm_light

ifeq ($(mode),opt)
   CCFLAGS = -Wall -O3 -fPIC -shared -fopenmp
   BUILD_DIR = ../build/incar_gaze/test.opt
   LIBS = `pkg-config opencv --cflags --libs` `pkg-config fftw3 --cflags --libs` -lfftw3_omp -fopenmp -lpthread
else
   mode = debug
   CCFLAGS = -g -Wall -fPIC -shared
   BUILD_DIR = ../build/incar_gaze/test
   LIBS = `pkg-config opencv --cflags --libs` `pkg-config fftw3 --cflags --libs`
endif

CFILES = stream.cpp test.cpp capture.cpp annotate.cpp accuracy.cpp sectors.cpp

TRACK_INCLUDE = ../install/include
TRACK_INSTALL = ../install/lib
INSTALL_DIR = ../install/bin

TEST_OUT = $(INSTALL_DIR)/test
STREAM_OUT = $(INSTALL_DIR)/stream
CAPTURE_OUT = $(INSTALL_DIR)/capture
ANNOT_OUT = $(INSTALL_DIR)/annotate
ACCURACY_OUT = $(INSTALL_DIR)/accuracy
SECTORS_OUT = $(INSTALL_DIR)/sectors

INCLUDES = -I ./ -I $(TRACK_INCLUDE) `pkg-config opencv --cflags` `pkg-config fftw3 --cflags` -I ${SVM_PATH}

OBJS := $(patsubst %.cpp, $(BUILD_DIR)/%.o, $(filter %.cpp,$(CFILES)))

.phony: all

all: $(TEST_OUT) $(STREAM_OUT) $(CAPTURE_OUT) $(ANNOT_OUT) $(ACCURACY_OUT) $(SECTORS_OUT)

$(BUILD_DIR)/%.o: %.cpp 
	$(MKDIR) -p $(BUILD_DIR)	
	$(CC) -c $(INCLUDES) -o $@ $< $(CCFLAGS) 

$(TEST_OUT): $(OBJS) $(TRACK_INSTALL)/libtrack.a
	$(MKDIR) -p $(INSTALL_DIR)	
	$(CC) -o $(TEST_OUT) $(BUILD_DIR)/test.o -L$(TRACK_INSTALL) -ltrack $(LIBS)
	@echo test finished

$(STREAM_OUT): $(OBJS) $(TRACK_INSTALL)/libtrack.a
	$(MKDIR) -p $(INSTALL_DIR)	
	$(CC) -o $(STREAM_OUT) $(BUILD_DIR)/stream.o -L$(TRACK_INSTALL) -ltrack $(LIBS)
	@echo stream finished

$(CAPTURE_OUT): $(OBJS) $(TRACK_INSTALL)/libtrack.a
	$(MKDIR) -p $(INSTALL_DIR)	
	$(CC) -o $(CAPTURE_OUT) $(BUILD_DIR)/capture.o -L$(TRACK_INSTALL) -ltrack $(LIBS)
	@echo capture finished

$(ANNOT_OUT): $(OBJS) $(TRACK_INSTALL)/libtrack.a
	$(MKDIR) -p $(INSTALL_DIR)	
	$(CC) -o $(ANNOT_OUT) $(BUILD_DIR)/annotate.o -L$(TRACK_INSTALL) -ltrack $(LIBS)
	@echo annotate finished

$(SECTORS_OUT): $(OBJS) $(TRACK_INSTALL)/libtrack.a
	$(MKDIR) -p $(INSTALL_DIR)	
	$(CC) -o $(SECTORS_OUT) $(BUILD_DIR)/sectors.o -L$(TRACK_INSTALL) -ltrack $(LIBS)
	@echo sectors finished

$(ACCURACY_OUT): $(OBJS) $(TRACK_INSTALL)/libtrack.a
	$(MKDIR) -p $(INSTALL_DIR)	
	$(CC) -o $(ACCURACY_OUT) $(BUILD_DIR)/accuracy.o -L$(TRACK_INSTALL) -ltrack $(LIBS)
	@echo accuracy finished

.PHONY: clean

clean:
	$(RM) -f $(BUILD_DIR)/*.o $(TEST_OUT) $(STREAM_OUT) $(CAPTURE_OUT) $(SECTORS_OUT)

