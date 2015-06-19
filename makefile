# This is a general makefile template.

                               BIN_DIR = ./
                                TARGET = $(BIN_DIR)/libJMEngine.a
                              C_CFLAGS = 
                                MACROS = -D_REENTRANT -D__USE_POSIX -D__linux__ -DLINUX
                                    CC = gcc
                            CPP_CFLAGS = -static
                                  MAKE = make
                                LINKER = g++
                          INCLUDE_DIRS = -I/usr/include -I./include -I/boost_1_49_0 -I./include/json/ -I./include/net -I./include/google \
					 -I./include/log -I./include/memory -I./include/rpc -I./include/server -I./include/util

                                  LIBS = -lboost_filesystem -lboost_system -lboost_thread -lboost_date_time -lprotobuf
                            OPTIM_FLAG = 
                                   CPP = g++ -std=c++11
                                LFLAGS = 
                              LIB_DIRS = -L/usr/local/lib/
                                 VPATH = ./source ./source/json ./source/json/src ./source/log ./source/memory ./source/net ./source/rpc ./source/server ./source/util
                            OBJECT_DIR = ./.obj/
                              CPPFILES = \
					 JME_GLog.cpp \
					 nedmalloc.cpp \
					 json_reader.cpp \
					 json_value.cpp \
					 json_writer.cpp \
					 JME_JsonFile.cpp \
					 JME_Acceptor.cpp \
					 JME_Core.cpp \
					 JME_TcpSession.cpp \
					 JME_RpcClient.cpp \
					 JME_RpcHandler.cpp \
					 JME_RpcServer.cpp \
					 JME_ConnectorServer.cpp \
					 JME_ServerInterface.cpp \
					 JME_Encrypt.cpp \
					 JME_String.cpp


                                CFILES =  

# To use 'make debug=0' build release edition.
ifdef debug
	ifeq ("$(origin debug)", "command line")
		IS_DEBUG = $(debug)
	endif
else
	IS_DEBUG = 1
endif
ifndef IS_DEBUG
	IS_DEBUG = 1
endif
ifeq ($(IS_DEBUG), 1)
	OPTIM_FLAG += -g3
else
	MACROS += -DNDEBUG
endif

# To use 'make quiet=1' all the build command will be hidden.
# To use 'make quiet=0' all the build command will be displayed.
ifdef quiet
	ifeq ("$(origin quiet)", "command line")
		QUIET = $(quiet)
	endif
endif
ifeq ($(QUIET), 1)
	Q = @
else
	Q =
endif

OBJECTS := $(addprefix $(OBJECT_DIR), $(notdir $(CPPFILES:%.cpp=%.o)))
OBJECTS += $(addprefix $(OBJECT_DIR), $(notdir $(CFILES:%.c=%.o)))

CALL_CFLAGS := $(C_CFLAGS) $(INCLUDE_DIRS) $(MACROS) $(OPTIM_FLAG)
CPPALL_CFLAGS := $(CPP_CFLAGS) $(INCLUDE_DIRS) $(MACROS) $(OPTIM_FLAG)
LFLAGS += $(OPTIM_FLAG) $(LIB_DIRS) $(LIBS)

all: checkdir $(TARGET)

$(TARGET): $(OBJECTS)
	$(Q)ar -cr $@ $(OBJECTS)
	@ln -sf $(notdir $(TARGET)) $(BIN_DIR)/libJMEngile.0.9.so

$(OBJECT_DIR)%.o:%.cpp
	$(Q)$(CPP) $(strip $(CPPALL_CFLAGS)) -c $< -o $@

$(OBJECT_DIR)%.o:%.c
	$(Q)$(CC) $(strip $(CALL_CFLAGS)) -c $< -o $@

checkdir:
	@if ! [ -d "$(BIN_DIR)" ]; then \
		mkdir $(BIN_DIR) ; \
		fi
	@if ! [ -d "$(OBJECT_DIR)" ]; then \
		mkdir $(OBJECT_DIR); \
		fi
clean:
	$(Q)rm -f $(OBJECTS)
cleanall: clean
	$(Q)rm -f $(TARGET)

.PHONY: all clean cleanall checkdir
