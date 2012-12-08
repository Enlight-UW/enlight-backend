#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
GREP=grep
NM=nm
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc.exe
CCC=g++.exe
CXX=g++.exe
FC=gfortran
AS=as.exe

# Macros
CND_PLATFORM=Cygwin-Windows
CND_CONF=Debug
CND_DISTDIR=dist
CND_BUILDDIR=build

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=${CND_BUILDDIR}/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/main.o \
	${OBJECTDIR}/_ext/618568943/GlobalStateTracker.o \
	${OBJECTDIR}/_ext/618568943/UDPStack.o \
	${OBJECTDIR}/_ext/618568943/UnixUDPStack.o


# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	"${MAKE}"  -f nbproject/Makefile-${CND_CONF}.mk ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/enlight-backend.exe

${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/enlight-backend.exe: ${OBJECTFILES}
	${MKDIR} -p ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/enlight-backend ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/main.o: nbproject/Makefile-${CND_CONF}.mk main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/main.o main.cpp

${OBJECTDIR}/_ext/618568943/GlobalStateTracker.o: nbproject/Makefile-${CND_CONF}.mk /cygdrive/C/Users/Alex/Documents/GitHub/enlight-backend/GlobalStateTracker.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/618568943
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/618568943/GlobalStateTracker.o /cygdrive/C/Users/Alex/Documents/GitHub/enlight-backend/GlobalStateTracker.cpp

${OBJECTDIR}/_ext/618568943/UDPStack.o: nbproject/Makefile-${CND_CONF}.mk /cygdrive/C/Users/Alex/Documents/GitHub/enlight-backend/UDPStack.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/618568943
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/618568943/UDPStack.o /cygdrive/C/Users/Alex/Documents/GitHub/enlight-backend/UDPStack.cpp

${OBJECTDIR}/_ext/618568943/UnixUDPStack.o: nbproject/Makefile-${CND_CONF}.mk /cygdrive/C/Users/Alex/Documents/GitHub/enlight-backend/UnixUDPStack.cpp 
	${MKDIR} -p ${OBJECTDIR}/_ext/618568943
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/_ext/618568943/UnixUDPStack.o /cygdrive/C/Users/Alex/Documents/GitHub/enlight-backend/UnixUDPStack.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r ${CND_BUILDDIR}/${CND_CONF}
	${RM} ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/enlight-backend.exe

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
