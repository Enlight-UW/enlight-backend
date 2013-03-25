## -*- Makefile -*-
##
## User: alex
## Time: Feb 19, 2013 8:21:55 PM
## Makefile created by Oracle Solaris Studio.
##
## This file is generated automatically.
##


#### Compiler and tool definitions shared by all build targets #####
CCC = g++
CXX = g++
BASICOPTS = -g
CCFLAGS = $(BASICOPTS)
CXXFLAGS = $(BASICOPTS)
CCADMIN = 


# Define the target directories.
TARGETDIR_enlight-backend=GNU-amd64-Windows


all: $(TARGETDIR_enlight-backend)/enlight-backend

## Target: enlight-backend
OBJS_enlight-backend =  \
	$(TARGETDIR_enlight-backend)/GlobalStateTracker.o \
	$(TARGETDIR_enlight-backend)/main.o \
	$(TARGETDIR_enlight-backend)/UDPStack.o \
	$(TARGETDIR_enlight-backend)/UnixUDPStack.o
USERLIBS_enlight-backend = $(SYSLIBS_enlight-backend) 
DEPLIBS_enlight-backend =  
LDLIBS_enlight-backend = $(USERLIBS_enlight-backend)


# Link or archive
$(TARGETDIR_enlight-backend)/enlight-backend: $(TARGETDIR_enlight-backend) $(OBJS_enlight-backend) $(DEPLIBS_enlight-backend)
	$(LINK.cc) $(CCFLAGS_enlight-backend) $(CPPFLAGS_enlight-backend) -o $@ $(OBJS_enlight-backend) $(LDLIBS_enlight-backend)


# Compile source files into .o files
$(TARGETDIR_enlight-backend)/GlobalStateTracker.o: $(TARGETDIR_enlight-backend) GlobalStateTracker.cpp
	$(COMPILE.cc) $(CCFLAGS_enlight-backend) $(CPPFLAGS_enlight-backend) -o $@ GlobalStateTracker.cpp

$(TARGETDIR_enlight-backend)/main.o: $(TARGETDIR_enlight-backend) main.cpp
	$(COMPILE.cc) $(CCFLAGS_enlight-backend) $(CPPFLAGS_enlight-backend) -o $@ main.cpp

$(TARGETDIR_enlight-backend)/UDPStack.o: $(TARGETDIR_enlight-backend) UDPStack.cpp
	$(COMPILE.cc) $(CCFLAGS_enlight-backend) $(CPPFLAGS_enlight-backend) -o $@ UDPStack.cpp

$(TARGETDIR_enlight-backend)/UnixUDPStack.o: $(TARGETDIR_enlight-backend) UnixUDPStack.cpp
	$(COMPILE.cc) $(CCFLAGS_enlight-backend) $(CPPFLAGS_enlight-backend) -o $@ UnixUDPStack.cpp



#### Clean target deletes all generated files ####
clean:
	rm -f \
		$(TARGETDIR_enlight-backend)/enlight-backend \
		$(TARGETDIR_enlight-backend)/GlobalStateTracker.o \
		$(TARGETDIR_enlight-backend)/main.o \
		$(TARGETDIR_enlight-backend)/UDPStack.o \
		$(TARGETDIR_enlight-backend)/UnixUDPStack.o
	$(CCADMIN)
	rm -f -r $(TARGETDIR_enlight-backend)


# Create the target directory (if needed)
$(TARGETDIR_enlight-backend):
	mkdir -p $(TARGETDIR_enlight-backend)


# Enable dependency checking
.KEEP_STATE:
.KEEP_STATE_FILE:.make.state.GNU-amd64-Windows

