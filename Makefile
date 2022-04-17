include lib/make-pal/pal.mak
DIR_SRC:=src
DIR_BUILD:=build
DIR_BUILD_LIB:=build-lib
DIR_LIB:=lib
CC:=gcc
CXX:=g++
AR:=ar

MAIN_NAME:=libamiss
MAIN_SRC:=$(wildcard $(DIR_SRC)/*.c)
MAIN_OBJ:=$(MAIN_SRC:$(DIR_SRC)/%.c=$(DIR_BUILD)/%.o)
MAIN_DEP:=$(MAIN_OBJ:%.o=%.d)
MAIN_CC_FLAGS:=-g -W -Werror -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wno-unused-parameter \
               -fPIC -Iinclude -Ibuild-lib/plutovg/include
MAIN_AR_FLAGS:=-rsc

all:
	$(MAKE) -j all-conc
all-lib: plutovg
all-conc: main

# Create static library.
main: $(DIR_BUILD) $(DIR_BUILD)/$(MAIN_NAME).$(EXT_LIB_STATIC)
$(DIR_BUILD)/$(MAIN_NAME).$(EXT_LIB_STATIC): $(MAIN_OBJ)
	$(AR) $(MAIN_AR_FLAGS) $(@) $(^)

# Build plutovg.
plutovg: $(DIR_BUILD_LIB)
	$(call pal_mkdir,$(DIR_LIB)/plutovg/build)
	cd $(DIR_LIB)/plutovg/build && cmake -G "$(CMAKE_GENERATOR)" -DCMAKE_C_COMPILER=$(CC) -DCMAKE_CXX_COMPILER=$(CXX) -DCMAKE_MAKE_PROGRAM=$(MAKE) ..
	cd $(DIR_LIB)/plutovg/build && $(MAKE)
	$(call pal_mkdir,$(DIR_BUILD_LIB)/plutovg)
	$(call pal_mkdir,$(DIR_BUILD_LIB)/plutovg/include)
	$(call pal_cp,$(DIR_LIB)/plutovg/build/libplutovg.a,$(DIR_BUILD_LIB)/plutovg)
	$(call pal_cpdir,$(DIR_LIB)/plutovg/include,$(DIR_BUILD_LIB)/plutovg/include)
	$(call pal_rmdir,$(DIR_BUILD_LIB)/plutovg/include/CMakeLists.txt)

# Compile source files to object files.
$(DIR_BUILD)/%.o: $(DIR_SRC)/%.c
	$(CC) $(<) -o $(@) $(MAIN_CC_FLAGS) -c -MMD

# Recompile source files after a header they include changes.
-include $(MAIN_DEP)

$(DIR_BUILD) $(DIR_BUILD_LIB):
	$(call pal_mkdir,$(@))
clean:
	$(call pal_rmdir,$(DIR_BUILD))

.PHONY: all all-conc main plutovg clean
