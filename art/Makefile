include ../lib/make-pal/pal.mak
DIR_BUILD:=build
CC:=gcc
CC_FLAGS:=-g -W -Werror -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wno-unused-parameter -fsanitize=address \
          -I../include -L../build -lamiss

#######################################
ARTS:=000_test
000_test_SRC:=000_test/main.c
#######################################

all:
	$(MAKE) -j all_conc
all_conc: $(DIR_BUILD) $(addprefix $(DIR_BUILD)/,$(foreach ART,$(ARTS),$(addsuffix .$(EXT_BIN),$(ART))))

define ART_TEMPLATE
$(1)_OBJ:=$$(addprefix $$(DIR_BUILD)/, $$($(1)_SRC:.c=.o))
$(1)_DEP:=$$(addprefix $$(DIR_BUILD)/, $$($(1)_SRC:.c=.d))
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

.PHONY: all all_conc clean
