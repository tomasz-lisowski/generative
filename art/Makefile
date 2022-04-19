include ../lib/make-pal/pal.mak
DIR_BUILD:=build
CC:=gcc
CC_FLAGS:=-g -W -Werror -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wno-unused-parameter \
          -I../include -I../build-lib/plutovg/include -L../build-lib/plutovg -L../build \
		  -lamiss -lm -lplutovg

#######################################
ARTS:=000-test 001-lsystem
000-test_SRC:=main.c
001-lsystem_SRC:=main.c
#######################################

all:
	$(MAKE) -j all-conc
all-conc: $(DIR_BUILD) $(addprefix $(DIR_BUILD)/,$(foreach ART,$(ARTS),$(addsuffix .$(EXT_BIN),$(ART))))

define ART_TEMPLATE
$(1)_OBJ:=$$(addprefix $$(DIR_BUILD)/$(1)/, $$($(1)_SRC:.c=.o))
$(1)_DEP:=$$(addprefix $$(DIR_BUILD)/$(1)/, $$($(1)_SRC:.c=.d))
$(1): $(DIR_BUILD)/$(addsuffix .$(EXT_BIN),$(1))
$(DIR_BUILD)/$(addsuffix .$(EXT_BIN),$(1)): $(DIR_BUILD)/$(1) $$($(1)_OBJ)
	$(CC) $$($(1)_OBJ) -o $$(@) $(CC_FLAGS)
endef

# Create recipes for all arts.
$(foreach ART,$(ARTS),$(eval $(call ART_TEMPLATE,$(ART))))

$(DIR_BUILD)/%.o: %.c
	$(CC) $(<) -o $(@) $(CC_FLAGS) -c -MMD

# Recompile source files after a header they include changes.
-include $(foreach ART,$(ARTS),$$($(ART)_DEP))

$(DIR_BUILD) $(foreach ART,$(ARTS),$(DIR_BUILD)/$(ART)):
	$(call pal_mkdir,$(@))
clean:
	$(call pal_rmdir,$(DIR_BUILD))

.PHONY: all all-conc clean
