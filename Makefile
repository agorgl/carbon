#=- Makefile -=#
#---------------------------------------------------------------
# Usage
#---------------------------------------------------------------
# The following generic Makefile is based on a specific project
# structure that makes the following conventions:
#
# The vanilla directory structure is in the form:
# - Root
#   - include [1]
#   - deps    [2]
#   - src     [3]
# Where:
#  [1] exists only for the library projects, and contains the header files
#  [2] exists only for the projects with external dependencies,
#      and each subfolder represents a dependency with a project
#      in the form we are currently documenting
#  [3] contains the source code for the project
#
# A working (built) project can also contain the following:
# - Root
#   - bin [5]
#   - lib [6]
#   - tmp [7]
# Where:
#  [5] contains executable results
#  [6] contains library results (static or shared)
#  [7] contains intermediate object files of compiling processes

# Disable the built-in implicit rules
MAKEFLAGS += --no-builtin-rules

# Detect OS
ifeq ($(OS), Windows_NT)
	HOST_OS := Windows
else
	HOST_OS := $(shell uname -s)
endif

# CreateProcess NULL bug
ifeq ($(HOST_OS), Windows)
	SHELL = cmd.exe
endif

# Set current makefile location
MKLOC ?= $(CURDIR)/$(firstword $(MAKEFILE_LIST))
export MKLOC

#---------------------------------------------------------------
# Build variable parameters
#---------------------------------------------------------------
# Variant = (debug|release)
VARIANT ?= debug
TOOLCHAIN ?= GCC
SILENT ?=
VERBOSE ?=
# Default target OS is host if not provided
TARGET_OS ?= $(HOST_OS)
# Install location
LOCAL_REPO ?= $(HOME)/.local

#---------------------------------------------------------------
# Helpers
#---------------------------------------------------------------
# Recursive wildcard func
rwildcard = $(foreach d, $(wildcard $1*), $(call rwildcard, $d/, $2) $(filter $(subst *, %, $2), $d))
# Keeps only paths corresponding to files
filter-files = $(filter-out $(patsubst %/., %, $(wildcard $(addsuffix /., $(1)))), $(1))

# Suppress full command output
ifeq ($(HOST_OS), Windows)
	suppress_out = > nul 2>&1
else
	suppress_out = > /dev/null 2>&1
endif

# Ignore command error
ifeq ($(HOST_OS), Windows)
	ignore_err = || (exit 0)
else
	ignore_err = || true
endif

# Native paths
ifeq ($(HOST_OS), Windows)
	native_path = $(subst /,\,$(1))
else
	native_path = $(1)
endif

# Makedir command
ifeq ($(HOST_OS), Windows)
	MKDIR_CMD = mkdir
else
	MKDIR_CMD = mkdir -p
endif
mkdir = -$(if $(wildcard $(1)/.*), , $(MKDIR_CMD) $(call native_path, $(1)) $(suppress_out)$(ignore_err))

# Rmdir command
ifeq ($(HOST_OS), Windows)
	RMDIR_CMD = rmdir /s /q
else
	RMDIR_CMD = rm -rf
endif
rmdir = $(if $(wildcard $(1)/.*), $(RMDIR_CMD) $(call native_path, $(1)),)

# Copy command
ifeq ($(HOST_OS), Windows)
	COPY_CMD = copy /Y
else
	COPY_CMD = cp
endif
copy = $(COPY_CMD) $(call native_path, $(1)) $(call native_path, $(2)) $(suppress_out)

# Remove command
ifeq ($(HOST_OS), Windows)
	REMOVE_CMD = del /q
else
	REMOVE_CMD = rm -f
endif
remove = $(REMOVE_CMD) $(call native_path, $(1)) $(suppress_out)

# Recursive folder copy command
ifeq ($(HOST_OS), Windows)
	RCOPY_CMD = robocopy /S /E $(call native_path, $(1)) $(call native_path, $(2)) $(suppress_out)$(ignore_err)
else
	RCOPY_CMD = cp -r $(call native_path, $(1))/* $(call native_path, $(2))
endif
rcopy = $(RCOPY_CMD)

# Path separator
pathsep = $(strip $(if $(filter $(HOST_OS), Windows), ;, :))

# Lowercase
lc = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,\
	$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,\
	$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,\
	$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))

# Quiet execution of command
quiet = $(if $(SILENT), $(suppress_out),)

# Command shown when executed
showcmd = $(if $(VERBOSE),,@)

# Newline macro
define \n


endef

# Space variable
space := $(subst ,, )

# Comma variable
comma := ,

# Canonical path (1 = base, 2 = path)
canonical_path = $(filter-out $(1), $(patsubst $(strip $(1))/%,%,$(abspath $(2))))
canonical_path_cur = $(call canonical_path, $(CURDIR), $(1))

# Gather given file from all subdirectories (1 = base, 2 = file)
findfile = $(foreach d, $(wildcard $1/*), $(filter %$(strip $2), $d))
# Gather given file from all subdirectories recursively (1 = base, 2 = file)
rfindfile = $(foreach d, $(wildcard $1/*), $(filter %$(strip $2), $d) $(call rfindfile, $d, $2))
# Gather all project configs
gatherprojs = $(strip $(patsubst ./%, %, $(patsubst %/config.mk, %, $(sort $(call rfindfile, $1, config.mk)))))

# Search for library in given paths
searchlibrary = $(foreach sd, $2, $(call findfile, $(sd), $1))
# Parse libraryname::version pairs
lib-from-extlib-pair = $(firstword $(subst ::, , $(1)))
ver-from-extlib-pair = $(strip $(or $(lastword $(subst ::, , $(1))), dev))
# Construct base path to repo dependency
extdep-pair = $(strip $(call lc,$(1)))::$(strip $(2))
extdep-path = $(LOCAL_REPO)/builds/$(subst ::,/,$(call extdep-pair, $(1), $(2)))
extdep-conf = $(call extdep-path, \
					$(call lib-from-extlib-pair, $(1)), \
					$(call ver-from-extlib-pair, $(1)))/pkg.mk

#---------------------------------------------------------------
# Global constants
#---------------------------------------------------------------
# Os executable extension
ifeq ($(TARGET_OS), Windows)
	EXECEXT = .exe
else
	EXECEXT = .out
endif
# Os dynamic library extension
ifeq ($(TARGET_OS), Windows)
	DLEXT = .dll
else
	DLEXT = .so
endif
# Object file extension
OBJEXT = .o
# Headers dependency file extension
HDEPEXT = .d
# Intermediate build directory
BUILDDIR := tmp

#---------------------------------------------------------------
# Header dependency generation helpers
#---------------------------------------------------------------
# Command chunks that help generate dependency files in each toolchain
sed-escape = $(subst /,\/,$(subst \,\\,$(1)))
msvc-dep-gen = $(1) /showIncludes >$(basename $@)$(HDEPEXT) & \
	$(if $(strip $(2)),,sed -e "/^Note: including file:/d" $(basename $@)$(HDEPEXT) &&) \
	sed -i \
		-e "/: error /q1" \
		-e "/^Note: including file:/!d" \
		-e "s/^Note: including file:\s*\(.*\)$$/\1/" \
		-e "s/\\/\//g" \
		-e "s/ /\\ /g" \
		-e "s/^\(.*\)$$/\t\1 \\/" \
		-e "2 s/^.*$$/$(call sed-escape,$@)\: $(call sed-escape,$<) &/g" \
		$(basename $@)$(HDEPEXT) || (echo. >$(basename $@)$(HDEPEXT) & exit 1)
gcc-dep-gen = -MMD -MT $@ -MF $(basename $@)$(HDEPEXT)

# Command wrapper that adds dependency generation functionality to given compile command
ifndef NO_INC_BUILD
dep-gen-wrapper = $(if $(filter $(TOOLCHAIN), MSVC), \
	$(call msvc-dep-gen, $(1), $(2)), \
	$(1) $(gcc-dep-gen))
else
dep-gen-wrapper = $(1)
endif

#---------------------------------------------------------------
# Colors
#---------------------------------------------------------------
ifneq ($(HOST_OS), Windows)
	ESC := $(shell printf "\033")
endif
NO_COLOR       := $(ESC)[0m
LGREEN_COLOR   := $(ESC)[92m
LYELLOW_COLOR  := $(ESC)[93m
LMAGENTA_COLOR := $(ESC)[95m
LRED_COLOR     := $(ESC)[91m
DGREEN_COLOR   := $(ESC)[32m
DYELLOW_COLOR  := $(ESC)[33m
DCYAN_COLOR    := $(ESC)[36m

#---------------------------------------------------------------
# Toolchain dependent values
#---------------------------------------------------------------
ifeq ($(TOOLCHAIN), MSVC)
	# Compiler
	CC          := cl
	CXX         := cl
	CFLAGS      := /nologo /EHsc /W4 /c
	CXXFLAGS    :=
	COUTFLAG    := /Fo:
	# Preprocessor
	INCFLAG     := /I
	DEFINEFLAG  := /D
	# Archiver
	AR          := lib
	ARFLAGS     := /nologo
	SLIBEXT     := .lib
	SLIBPREF    :=
	AROUTFLAG   := /OUT:
	# Linker
	LD          := link
	LDFLAGS     := /nologo /manifest
	LIBFLAG     :=
	LIBSDIRFLAG := /LIBPATH:
	LOUTFLAG    := /OUT:
	LSOFLAGS    := /DLL
	# Variant specific flags
	ifeq ($(VARIANT), debug)
		CFLAGS  += /MTd /Zi /Od /FS /Fd:$(BUILDDIR)/$(TARGETNAME).pdb
		LDFLAGS += /debug
	else
		CFLAGS  += /MT /O2
		LDFLAGS += /incremental:NO
	endif
else
	# Compiler
	CC          := gcc
	CXX         := g++
	CFLAGS      := -Wall -Wextra -c
	CXXFLAGS    := -std=c++14
	COUTFLAG    := -o
	CSOFLAGS    := -fPIC
	# Preprocessor
	INCFLAG     := -I
	DEFINEFLAG  := -D
	# Archiver
	AR          := ar
	ARFLAGS     := rcs
	SLIBEXT     := .a
	SLIBPREF    := lib
	AROUTFLAG   :=
	# Linker
	LD          := g++
	LDFLAGS     :=
	ifeq ($(TARGET_OS), Windows)
		LDFLAGS += -static-libgcc -static-libstdc++
	endif
	LIBFLAG     := -l
	LIBSDIRFLAG := -L
	LOUTFLAG    := -o
	LSOFLAGS    := -shared -fPIC
	# Variant specific flags
	ifeq ($(VARIANT), debug)
		CFLAGS  += -g -O0
	else
		CFLAGS  += -O2
	endif
endif

ifdef CROSS_COMPILE
	CC  := $(CROSS_COMPILE)-$(CC)
	CXX := $(CROSS_COMPILE)-$(CXX)
	AR  := $(CROSS_COMPILE)-$(AR)
	LD  := $(CROSS_COMPILE)-$(LD)
	CFLAGS   += --sysroot=$(SYSROOT)
	CXXFLAGS += --sysroot=$(SYSROOT)
	CPPFLAGS += --sysroot=$(SYSROOT)
	LDFLAGS  += --sysroot=$(SYSROOT)
endif

#---------------------------------------------------------------
# Rule generators
#---------------------------------------------------------------
# Compile rules (1 = extension, 2 = command generator, 3 = subproject path)
define compile-rule
$(BUILDDIR)/$(VARIANT)/$(strip $(3))%.$(strip $(1))$(OBJEXT): $(strip $(3))%.$(strip $(1))
	@$$(info $(LGREEN_COLOR)[>] Compiling$(NO_COLOR) $(LYELLOW_COLOR)$$<$(NO_COLOR))
	@$$(call mkdir, $$(@D))
	$(showcmd)$$(call dep-gen-wrapper, $(2), $(SILENT)) $(quiet)
endef

#---------------------------------------------------------------
# Meta
#---------------------------------------------------------------
# Used by project generators to make $(D) and $(DP) variables available
define subproj-template-prologue
# Subproject name
$(eval D = $(strip $(1)))
# Subproject path
$(eval DP = $(if $(filter $(D),.),,$(D)/))
endef

#---------------------------------------------------------------
# Per project configuration
#---------------------------------------------------------------
# Should at least define:
# - PRJTYPE variable (Executable|StaticLib|DynLib)
# - LIBS variable (optional, Executable type only)
# Can optionally define:
# - TARGETNAME variable (project name, defaults to name of the root folder)
# - SRCDIR variable (source directory)
# - BUILDDIR variable (intermediate build directory)
# - SRC variable (list of the source files, defaults to every code file in SRCDIR)
# - SRCEXT variable (list of extensions used to match source files)
# - DEFINES variable (list defines in form of PROPERTY || PROPERTY=VALUE)
# - ADDINCS variable (list with additional include dirs)
# - MOREDEPS variable (list with additional dep dirs)

# Parses given subproject configuration to _$(subproj) postfixed variables.
# Fills in with defaults on some values if not given.
define parse-subproject-config
${subproj-template-prologue}
# Clear previous variables
$(foreach v, PRJTYPE VERSION DEFINES LIBS MOREDEPS EXTDEPS SRCDIR SRC ADDINCS ADDLIBDIR MCFLAGS MLDFLAGS, undefine $(v)${\n})

# Include configuration
-include $(DP)config.mk

# Gather variables from config
PRJTYPE_$(D)   := $$(PRJTYPE)
VERSION_$(D)   := $$(VERSION)
DEFINES_$(D)   := $$(DEFINES)
LIBS_$(D)      := $$(LIBS)
MOREDEPS_$(D)  := $$(MOREDEPS)
EXTDEPS_$(D)   := $$(EXTDEPS)
MCFLAGS_$(D)   := $$(MCFLAGS)
MLDFLAGS_$(D)  := $$(MLDFLAGS)
# Variables refering to local project paths,
# must be prefixed with subproject path
SRCDIR_$(D)    := $$(addprefix $(DP), $$(SRCDIR))
SRC_$(D)       := $$(addprefix $(DP), $$(SRC))
ADDINCS_$(D)   := $$(addprefix $(DP), $$(ADDINCS))
ADDLIBDIR_$(D) := $$(addprefix $(DP), $$(ADDLIBDIR))

# Set defaults on unset variables
VERSION_$(D)    := $$(strip $$(or $$(VERSION_$(D)), dev))
TARGETNAME_$(D) := $$(or $$(TARGETNAME_$(D)), $$(notdir $$(if $$(filter $(D),.), $$(CURDIR), $(D))))
SRCDIR_$(D)     := $$(or $$(SRCDIR_$(D)), $(DP)src)
SRCEXT_$(D)     := *.c *.cpp *.cc *.cxx
SRC_$(D)        := $$(or $$(SRC_$(D)), $$(call rwildcard, $$(SRCDIR_$(D)), $$(SRCEXT_$(D))))
endef

#---------------------------------------------------------------
# Populated values
#---------------------------------------------------------------
#=- Populate core project target values
define populate-target-values
${subproj-template-prologue}
# Target directory
ifeq ($$(PRJTYPE_$(D)), Executable)
	TARGETDIR_$(D) := $(DP)bin
else
	TARGETDIR_$(D) := $(DP)lib
endif
# Output
ifeq ($$(PRJTYPE_$(D)), StaticLib)
	TARGET_$(D) := $(SLIBPREF)$$(strip $$(call lc,$$(TARGETNAME_$(D))))$(SLIBEXT)
else ifeq ($$(PRJTYPE_$(D)), DynLib)
	TARGET_$(D) := $(SLIBPREF)$$(strip $$(call lc,$$(TARGETNAME_$(D))))$(DLEXT)
else
	TARGET_$(D) := $$(TARGETNAME_$(D))$(EXECEXT)
endif
# Master output full path
MASTEROUT_$(D) := $$(TARGETDIR_$(D))/$(VARIANT)/$$(TARGET_$(D))
# Objects
OBJ_$(D) := $$(addprefix $(BUILDDIR)/$(VARIANT)/, $$(SRC_$(D):=$(OBJEXT)))
# Header dependencies
HDEPS_$(D) := $$(OBJ_$(D):$(OBJEXT)=$(HDEPEXT))
# Install location
INSTALL_PREFIX_$(D) := $$(call extdep-path, $$(TARGETNAME_$(D)), $$(VERSION_$(D)))
# Install pair
INSTALL_PAIR_$(D)   := $$(call extdep-pair, $$(TARGETNAME_$(D)), $$(VERSION_$(D)))
endef

#--------------------------------------------------
#- Read external pkg file for given dependency
define read-ext
-include $$(call extdep-conf, $(1))
PKGS += $$(PKGDEPS)
endef

#=- Populate project dependency values
define populate-dep-values
${subproj-template-prologue}
# Implicit dependencies directory
DEPSDIR_$(D) := $(DP)deps
# Implicit dependencies
DEPS_$(D) := $$(call gatherprojs, $$(DEPSDIR_$(D)))
# Explicit dependencies
DEPS_$(D) += $$(foreach md, $$(MOREDEPS_$(D)), $$(or $$(call canonical_path_cur, $(DP)/$$(md)), .))
# Append dependency pairs from local dependencies
undefine PKGS
PKGS :=
$$(foreach dep, $$(EXTDEPS_$(D)), $$(eval $$(call read-ext, $$(dep))))
EXTDEPS_$(D) += $$(PKGS)
endef

#--------------------------------------------------
#=- Populate project path values
define populate-path-values
${subproj-template-prologue}
# External (repo installed) dependency directories
EXTDEPPATHS_$(D) := $$(foreach ed, $$(EXTDEPS_$(D)), $$(call extdep-path, \
						$$(call lib-from-extlib-pair, $$(ed)), \
						$$(call ver-from-extlib-pair, $$(ed))))
# Include search paths
INCPATHS_$(D)    := $$(strip $(DP)include \
						$$(foreach dep, $$(DEPS_$(D)) \
										$$(filter-out $$(DEPS_$(D)), $$(wildcard $$(DEPSDIR_$(D))/*)), \
											$$(dep)/include) \
						$$(ADDINCS_$(D)) \
						$$(foreach extdep, $$(EXTDEPPATHS_$(D)), $$(extdep)/include))
# Library search paths
LIBPATHS_$(D)    := $$(strip $$(foreach libdir,\
									$$(foreach dep, $$(DEPS_$(D)), $$(dep)/lib) \
									$$(ADDLIBDIR_$(D)),\
								$$(libdir)/$(strip $(VARIANT))) \
								$$(foreach extdep, $$(EXTDEPPATHS_$(D)), $$(extdep)/lib))
endef

#--------------------------------------------------
#=- Populate project flag values
define populate-flag-values
${subproj-template-prologue}
# Preprocessor flags
CPPFLAGS_$(D) := $$(addprefix $(DEFINEFLAG), $$(DEFINES_$(D)))
# Include path flags
INCDIR_$(D)   := $$(addprefix $(INCFLAG), $$(INCPATHS_$(D)))
# Library path flags
LIBSDIR_$(D)  := $$(addprefix $(LIBSDIRFLAG), $$(LIBPATHS_$(D)))
# Library flags
LIBFLAGS_$(D) := $$(strip $$(foreach lib, $$(LIBS_$(D)), $(LIBFLAG)$$(lib)$(if $(filter $(TOOLCHAIN), MSVC),.lib,)))
# Extra link flags when building shared libraries
ifeq ($$(PRJTYPE_$(D)), DynLib)
	# Add shared library toggle
	MORELFLAGS_$(D) := $(LSOFLAGS)
endif
# Setup rpath flag parameter for linux systems
ifeq ($$(PRJTYPE_$(D)), Executable)
ifneq ($(TARGET_OS), Windows)
	# Path from executable location to project root
	RELPPREFIX_$(D)  := $$(subst $$(space),,$$(foreach dir, $$(subst /,$$(space),$$(dir $$(MASTEROUT_$(D)))),../))
	# Library paths relative to the executable
	LIBRELPATHS_$(D) := $$(addprefix $$(RELPPREFIX_$(D)), $$(LIBPATHS_$(D)))
	# Add rpath param to search for dependent shared libraries relative to the executable location
	MORELFLAGS_$(D)  := '-Wl$$(comma)-rpath$$(comma)$$(subst $$(space),:,$$(addprefix $$$$ORIGIN/, $$(LIBRELPATHS_$(D))))'
endif
endif
endef

#--------------------------------------------------
#=- Populate project rule dependency values
define populate-rule-dep-values
${subproj-template-prologue}
# Build rule dependencies
BUILDDEPS_$(D) := $$(foreach dep, $$(DEPS_$(D)), $$(MASTEROUT_$$(dep)))
# Install dependencies for static library
ifeq ($$(PRJTYPE_$(D)), StaticLib)
INSTDEPS_$(D) := $$(addprefix install_, $$(DEPS_$(D)))
endif
endef

#--------------------------------------------------
#=- Populate project install values
# 1 = dst file 2 = src file
define copy-file-rule
$(1): $(2)
	$$(info $(LGREEN_COLOR)[>] Copying$(NO_COLOR) $(LYELLOW_COLOR)$(strip $(2)) -> $$(subst \,/,$(strip $(1)))$(NO_COLOR))
	$(showcmd)$$(call mkdir, $$(@D))
	$(showcmd)$$(call copy, $(2), $$(@D))
INSTALL_FILES += $(1)
endef

# 1 = dst folder 2 = src folder
define copy-folder-rule
$(foreach f, $(call filter-files, $(call rwildcard, $(2), *)), \
	$(call copy-file-rule, $(1)/$(call canonical_path, $(abspath $(2)), $(f)), $(f))${\n})
endef

define populate-install-values
${subproj-template-prologue}
# Reset install file list
$(eval undefine INSTALL_FILES)
ifneq ($$(PRJTYPE_$(D)), Executable)
# Copy header folder
$$(eval $$(call copy-folder-rule, $$(INSTALL_PREFIX_$(D))/include, $$(DP)include))
endif
# Copy master output folder
$$(eval $$(call copy-folder-rule, $$(INSTALL_PREFIX_$(D))/$$(lastword $$(subst /, , $$(TARGETDIR_$(D)))), $$(dir $$(MASTEROUT_$(D)))))
# Save install file list
INSTALL_FILES_$(D) := $$(INSTALL_FILES)
# Package description file
PKG_INFO_$(D) := $$(call extdep-conf, $$(INSTALL_PAIR_$(D)))
endef

#--------------------------------------------------
#=- Generate compilation database entries
# 1 = src file 2 = compilation command
define compilation-database-entry-template
    {
        "directory": "$(patsubst %/,%,$(dir $(MKLOC)))",
        "file":      "$(strip $(1))",
        "command":   "$(strip $(2))"
    }
endef

define compile-database-entry
DB_ENTRY_$(strip $1) = $$(call compilation-database-entry-template, $(1), $(2))
DB_ENTRY_LIST += DB_ENTRY_$(strip $1)
endef

#---------------------------------------------------------------
# Rules
#---------------------------------------------------------------
# Generate rules for the given project
define gen-build-rules
${subproj-template-prologue}
# Dummy banner file for current project build
BANNERFILE_$(D) := $(BUILDDIR)/banners/banner_$(subst /,_,$(D))

# Main build rule
build_$(D): $$(MASTEROUT_$(D))

# Executes target
run_$(D): build_$(D)
	@echo Executing $$(MASTEROUT_$(D)) ...
	@$$(eval export PATH := $(PATH)$(pathsep)$$(subst $$(space),$(pathsep),$$(addprefix $$(CURDIR)/, $$(LIBPATHS_$(D)))))
	@cd $(D) && $$(call native_path, $$(call canonical_path, $$(abspath $$(CURDIR)/$(D)), $$(MASTEROUT_$(D))))

# Pkg.mk file contents
define PCFG_$(D)
PKGDEPS := $$(strip $$(EXTDEPS_$(D)) $$(foreach dep, $$(DEPS_$(D)), $$(INSTALL_PAIR_$$(dep))))
endef

# Creates package info file
$$(PKG_INFO_$(D)): $(DP)config.mk
	$$(info $(LGREEN_COLOR)[>] Pkginfo $(NO_COLOR)$(LYELLOW_COLOR)$$(@F) -> $$(@)$(NO_COLOR))
	$(showcmd)echo $$(PCFG_$(D)) > $$(@)

# Installs files to repository
install_$(D): $$(INSTDEPS_$(D)) $$(INSTALL_FILES_$(D)) $$(PKG_INFO_$(D))

# Show banner for current build execution
$$(BANNERFILE_$(D)):
	@$$(call mkdir, $$(@D))
	$$(info $(LRED_COLOR)[o] Building$(NO_COLOR) $(LMAGENTA_COLOR)$$(TARGETNAME_$(D))$(NO_COLOR))
	@echo Bum. > $$@

# Show banner if not shown
$$(OBJ_$(D)): | $$(BANNERFILE_$(D))

# Print build debug info
showvars_$(D): $$(BANNERFILE_$(D))
	@echo MASTEROUT: $$(MASTEROUT_$(D))
	@echo SRCDIR:    $$(SRCDIR_$(D))
	@echo SRC:       $$(SRC_$(D))
	@echo DEPS:      $$(DEPS_$(D))
	@echo BUILDDEPS: $$(BUILDDEPS_$(D))
	@echo CFLAGS:    $$(CFLAGS)
	@echo CPPFLAGS:  $$(CPPFLAGS_$(D))
	@echo INCDIR:    $$(INCDIR_$(D))
	@echo LIBSDIR:   $$(LIBSDIR_$(D))
	@echo LIBFLAGS:  $$(LIBFLAGS_$(D))
	@echo HDEPS:     $$(HDEPS_$(D))
	@echo INSTALL:   $$(INSTALL_PREFIX_$(D)): $$(INSTALL_FILES_$(D))
	@echo EXTDEPS:   $$(EXTDEPS_$(D))
	@echo EXTPATHS:  $$(EXTDEPPATHS_$(D))

# Show include search paths for current subproject
showincpaths_$(D):
	$$(info $$(INCPATHS_$(D)))

# Show defines for current subproject
showdefines_$(D):
	$$(info $$(DEFINES_$(D)))

# Include extra rules
-include $(DP)rules.mk

$$(MASTEROUT_$(D)): $$(BUILDDEPS_$(D)) $$(OBJ_$(D))
ifneq ($$(PRJTYPE_$(D)), StaticLib)
# Link rule
	@$$(info $(DGREEN_COLOR)[+] Linking$(NO_COLOR) $(DYELLOW_COLOR)$$@$(NO_COLOR))
	@$$(call mkdir, $$(@D))
	$(showcmd)$(LD) $(LDFLAGS) $$(MLDFLAGS_$(D)) $$(LIBSDIR_$(D)) $(LOUTFLAG)$$@ $$(OBJ_$(D)) $$(LIBFLAGS_$(D)) $$(MORELFLAGS_$(D))
else
# Archive rule
	@$$(info $(DCYAN_COLOR)[+] Archiving$(NO_COLOR) $(DYELLOW_COLOR)$$@$(NO_COLOR))
	@$$(call mkdir, $$(@D))
	$(showcmd)$(AR) $(ARFLAGS) $(AROUTFLAG)$$@ $$(OBJ_$(D))
endif

# Compile commands
CCOMPILE_$(D)   = $(CC) $(CFLAGS) $$(MCFLAGS_$(D)) $$(CPPFLAGS_$(D)) $$(INCDIR_$(D)) $$< $(COUTFLAG) $$@ $$(MORECFLAGS_$(D))
CXXCOMPILE_$(D) = $(CXX) $(CFLAGS) $(CXXFLAGS) $$(MCFLAGS_$(D)) $$(CPPFLAGS_$(D)) $$(INCDIR_$(D)) $$< $(COUTFLAG) $$@ $$(MORECFLAGS_$(D))

# Generate compile rules
$(foreach ext, c m, $(call compile-rule, $(ext), $$(CCOMPILE_$(D)), $(DP))${\n})
$(foreach ext, cpp cxx cc mm, $(call compile-rule, $(ext), $$(CXXCOMPILE_$(D)), $(DP))${\n})

# Generate compilation database entries
$(foreach cu, $(SRC_$(D)), $(call compile-database-entry, $(cu), \
	$$(subst $$@,$(BUILDDIR)/cdb/$(cu)$(OBJEXT), \
		$$(subst $$<, $(cu), \
			$$(if $$(filter c m, $$(patsubst .%,%,$$(suffix $(cu)))), \
				$$(CCOMPILE_$(D)), \
				$$(CXXCOMPILE_$(D)))))) \
	${\n})

endef

#---------------------------------------------------------------
# Generation
#---------------------------------------------------------------
# Scan all directories for subprojects
SUBPROJS := $(call gatherprojs, .)
# Parse subproject configs
$(foreach subproj, $(SUBPROJS), $(eval $(call parse-subproject-config, $(subproj))))
# Generate subproject values
$(foreach generator, \
			populate-target-values    \
			populate-dep-values       \
			populate-path-values      \
			populate-flag-values      \
			populate-install-values   \
			populate-rule-dep-values, \
		$(foreach subproj, $(SUBPROJS), $(eval $(call $(generator), $(subproj)))))
# Create sublists with dependency and main projects
SILENT_SUBPROJS    := $(foreach subproj, $(SUBPROJS), $(if $(findstring deps, $(subproj)), $(subproj)))
NONSILENT_SUBPROJS := $(filter-out $(SILENT_SUBPROJS), $(SUBPROJS))
# Generate subproject rules
SILENT := 1
$(foreach subproj, $(SILENT_SUBPROJS), $(eval $(call gen-build-rules, $(subproj))))
undefine SILENT
$(foreach subproj, $(NONSILENT_SUBPROJS), $(eval $(call gen-build-rules, $(subproj))))

# Aliases
build: build_.
run: run_.
install: install_.
showvars: showvars_.
showvars_all: $(addprefix showvars_, $(SUBPROJS))

# Track down Dynamic Library projects and find their dependencies
SO_PROJS := $(strip $(foreach subproj, $(SUBPROJS), $(if $(filter $(PRJTYPE_$(subproj)), DynLib), $(subproj),)))
SO_DEPS  := $(strip $(sort $(foreach sop, $(SO_PROJS), $(foreach dep, $(DEPS_$(sop)), $(if $(filter $(PRJTYPE_$(dep)), StaticLib), $(dep),)))))
# Make these projects have position independent code
$(foreach pic, $(SO_PROJS) $(SO_DEPS), $(eval MORECFLAGS_$(pic) += $(CSOFLAGS)))

# Dump compilation database
compile_db:
	$(info [)
	$(foreach e, \
		$(filter-out \
			$(lastword $(DB_ENTRY_LIST)), \
			$(DB_ENTRY_LIST)), \
		$(info $($(e)),))
	$(info $($(lastword $(DB_ENTRY_LIST))))
	$(info ])

# Cleanup rule
clean:
	@echo Cleaning...
	@$(call rmdir, $(BUILDDIR))

# Don't create dependencies when we're cleaning
NOHDEPSGEN = clean
ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NOHDEPSGEN))))
	# GNU Make attempts to (re)build the file it includes
	HDEPS = $(foreach subproj, $(SUBPROJS), $(HDEPS_$(subproj)))
	-include $(HDEPS)
endif

# Set default goal
.DEFAULT_GOAL := build

# Disable builtin rules
.SUFFIXES:

# Non file targets
PHONYRULETYPES := build run install showvars showincpaths showdefines
PHONYPREREQS := $(foreach ruletype, $(PHONYRULETYPES), $(addprefix $(ruletype)_, $(SUBPROJS))) \
		run \
		install \
		showvars \
		showvars_all \
		compile_db \
		clean
.PHONY: $(PHONYPREREQS)
