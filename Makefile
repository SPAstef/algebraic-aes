#### BEGIN PROJECT NAME ####
LIBNAME := aes
#### END PROJECT NAME ####


#### BEGIN USER OPTIONS ####
# Enables debug mode: 0 (no debug), 1 (debug), 2 (debug with optimization)
DEBUG := 0
# Override default C compiler: 0/1
OVERRIDE_DEFAULT_CC := 1
# Choose optimization level: 0/1/2/3/fast (values >2 may actually degrade performance!)
OPT_LEVEL := 2
# Project C standard
C_STD = 11
#### END USER OPTIONS ####

#### BEGIN TARGETS ####
#--- libraries (add a dummy target if otherwise empty) ---#
# C library targets 
TARGETS_LIB_C :=
TARGETS_LIB_C += aes
TARGETS_LIB_C += aes_ni
TARGETS_LIB_C += fq/fq_mat
TARGETS_LIB_C += fq/fq_poly
TARGETS_LIB_C += fq/fq_vec
TARGETS_LIB_C += fq/fq
TARGETS_LIB_C += rand
TARGETS_LIB_C += zp/zp_mat
TARGETS_LIB_C += zp/zp_poly
TARGETS_LIB_C += zp/zp_vec
TARGETS_LIB_C += zp/zp

#--- tests ---#
# C test targets
TARGETS_TEST_C :=
TARGETS_TEST_C += aes_encrypt

#### END TARGETS ####

#### BEGIN COMPILER OVERRIDE ####
ifeq ($(CC), )
    OVERRIDE_DEFAULT_CC := 1
endif

ifeq ($(OVERRIDE_DEFAULT_CC), 1)
    ifeq ($(OS), Windows_NT)
        CC := cl
        CFLAGS :=
    else
        CC := cc
        CFLAGS :=
    endif
endif
#### END COMPILER OVERRIDE ####

#### BEGIN DLL dependencies ####
DLL := 
#### END DLL dependencies ####

######## 
# What comes next should (ideally) not require changes, unless new project-specific user options, 
# new compilers, etc. are needed
########

#### BEGIN DIRECTORIES SETUP ####
# Directory for executables
BINPATH := bin
# Directory for build object files
BUILDPATH := build
# path for C files
CPATH := c
# Directory for dependencies
DEPPATH := dep
# Directory for header files
INCPATH := include
# Directory for output library
LIBPATH := lib
# Directory for source files
SRCPATH := src
# Directory for test files
TESTPATH := test
#### END DIRECTORIES SETUP ####

#### BEGIN TARGETS GROUPING ####
TARGETS_LIB_C := $(addprefix $(LIBPATH)/$(CPATH)/,$(TARGETS_LIB_C))
TARGETS_TEST_C := $(addprefix $(TESTPATH)/,$(TARGETS_TEST_C))
TARGETS_NONLIB_C := $(TARGETS_TEST_C)  

# Put all targets together
TARGETS_C := $(TARGETS_LIB_C) $(TARGETS_NONLIB_C)

TARGETS_LIB := $(TARGETS_LIB_C)
TARGETS_NONLIB := $(TARGETS_NONLIB_C)
TARGETS_TEST := $(TARGETS_TEST_C)
TARGETS := $(TARGETS_C)
#### END TARGETS GROUPING ####


#### BEGIN COMPILER SETUP ####
# Override flags for debugging
ifneq ($(DEBUG), 0)
    USE_ALWAYS_INLINE := 0
    CUFLAGS := -g
    ifeq ($(CC), cl)
        CFLAGS := -DEBUG -Zi
    else
        CFLAGS := -g
        ifeq ($(CC), icx)
            ifeq ($(DEBUG), 1)
                CFLAGS += -Rno-debug-disables-optimization
            endif
        endif
    endif
endif
ifneq ($(DEBUG), 1)
    CFLAGS += -O$(OPT_LEVEL)
endif

# Additional inclusion paths
INCFLAGS := -I$(INCPATH) -I$(DEPPATH)

# Source compilation flags
COMP_SRC_FLAGS := -c#-save-temps=obj

# Additional C compiler flags 
ifeq ($(CC), cl)
    OBJ_OUT_CFLAG := -Fo:
    EXE_OUT_CFLAG := -Fe:
    DEP_OUT_CFLAG := -MF:
    CFLAGS += -nologo -arch:AVX2 -std:clatest -W4 -MP $(INCFLAGS)
else
    OBJ_OUT_CFLAG := -o # notice the extra space!
    EXE_OUT_CFLAG := -o # notice the extra space!
    DEP_OUT_CFLAG := -MF # notice the extra space!
    EXTRA_CFLAGS := -std=c$(C_STD) -march=native -Wall -Wextra -MMD -MP $(INCFLAGS)
    ifeq ($(CC), icx)
        ifeq ($(OS), Windows_NT)
            EXTRA_CFLAGS := -Qstd=c$(C_STD) -march=native -Qiopenmp -nologo -Wall -Wextra -QMMD -QMP $(INCFLAGS)
            DEP_OUT_CFLAG := -QMF # notice the extra space!
        else
            EXTRA_CFLAGS += -qopenmp
        endif
    else
        EXTRA_CFLAGS += -fopenmp
    endif
    CFLAGS += $(EXTRA_CFLAGS)
endif

# Archive command and flags
ifeq ($(OS), Windows_NT)
    AR := lib
    ARFLAGS := -nologo
    LIBOUTFLAGS := /OUT:
    LIBEXT := lib
else
    AR := ar
    ARFLAGS := r 
    LIBEXT := a
endif
#### END COMPILER SETUP ####

#### BEGIN MACRO DEFINITIONS SETUP ####
# Disable debug-related code
ifeq ($(DEBUG), 0)
    CFLAGS += -DNDEBUG
    CXXFLAGS += -DNDEBUG
endif
#### END MACRO DEFINITIONS SETUP ####

#### BEGIN FILES SETUP ####
# Dependency file extension
DEPEXT := d

# Executable file extenstion
EXEEXT := exe

# Object file extenstion
ifeq ($(CC), cl)
    OBJEXT := obj
else
    OBJEXT := o
endif

# DLL file extension
ifeq ($(OS), Windows_NT)
    DLLEXT := .lib # Note the . is part of the extension
else
    DLLEXT :=
endif

# Dependency files (makes correctly recompile sources on header change)
DEP := $(TARGETS:%=$(BUILDPATH)/%.$(DEPEXT))

# Library file
LIBFILE := $(LIBPATH)/lib$(LIBNAME).$(LIBEXT)

#### END FILES SETUP ####

#### BEGIN LINKER SETUP ####
LDFLAGS := 
#### END LINKER SETUP ####

#### BEGIN COMMAND ALIASING ####
# Make useful commands OS independent
ifeq ($(OS), Windows_NT)
    MKDIR = if not exist $(subst /,\,$1) mkdir $(subst /,\, $1)
    MV = mv $(1) $(2)
    RM = if exist $(1) rmdir /S /Q $(1)
    TOUCH = type nul >> $(1)
else
    MKDIR = mkdir -p $(1)
    MV = mv $(1) $(2)/
    RM = rm -rf $(1)
    TOUCH = touch $(1)
endif
#### END COMMAND ALIASING ####

#### BEGIN MAIN RECIPES ####
# Build all targets
all: library nonlibrary
	

# Remove all build files
clean:
	$(call RM,$(BUILDPATH))
	$(call RM,$(BINPATH))
	$(call RM,$(LIBPATH))

# Build library
library: $(TARGETS_LIB)
	@$(call MKDIR, $(LIBPATH))
	$(AR) $(ARFLAGS) $(LIBOUTFLAGS)$(LIBFILE) $(foreach target, $(TARGETS_LIB), $(BUILDPATH)/$(target).$(OBJEXT))

nonlibrary: executables testsuite
	

# Build executables
executables: $(TARGETS_EXE)
	

# Build tests
testsuite: $(TARGETS_TEST)
	@$(call MKDIR, $(BINPATH))

#### END MAIN RECIPES ####

#### BEGIN SPECIFIC TARGET DEPENDENCIES ####
#### END SPECIFIC TARGET DEPENDENCIES ####

#### BEGIN GENERIC TARGET RECIPES ####
# Build C nonlibrary targets
$(TARGETS_NONLIB_C): %: $(BUILDPATH)/%.$(OBJEXT) library
	@$(call MKDIR, $(dir $(BINPATH)/$@.$(EXEEXT)))
	$(CC) $(CFLAGS) $< $(EXE_OUT_CFLAG)$(BINPATH)/$@.$(EXEEXT) $(LIBFILE) $(LDFLAGS)

# Build C library targets
$(TARGETS_LIB_C): %: $(BUILDPATH)/%.$(OBJEXT)
	
#### END GENERIC TARGET RECIPES ####

#### BEGIN GENERIC SOURCE RECIPES ####
# Compile C executable/library sources
$(BUILDPATH)/$(LIBPATH)/$(CPATH)/%.$(OBJEXT): $(SRCPATH)/%.c
	@$(call MKDIR, $(dir $@))
	$(CC) $(CFLAGS) $(COMP_SRC_FLAGS) $< $(OBJ_OUT_CFLAG)$@

# Compile C sources
$(BUILDPATH)/%.$(OBJEXT): %.c
	@$(call MKDIR, $(dir $@))
	$(CC) $(CFLAGS) $(COMP_SRC_FLAGS) $< $(OBJ_OUT_CFLAG)$@
#### END GENERIC SOURCE RECIPES ####

#### BEGIN DEPENDENCY RECIPIES INCLUSION ####
-include $(DEP)
#### END DEPENDENCY RECIPIES INCLUSION ####
