# ----------------------------
# Makefile Options
# ----------------------------

NAME = ADVNTURE
ICON = icon.png
DESCRIPTION = "The classic Atari 2600 game!"
COMPRESSED = YES
ARCHIVED = YES

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

FONTDIR = $(SRCDIR)/font
FONT = $(FONTDIR)/AdventureFont.fnt
FONT_INC = $(FONTDIR)/AdventureFont.inc

DEPS = $(FONT_INC)

# ----------------------------

include $(shell cedev-config --makefile)

# ----------------------------

$(FONT_INC): $(FONT)
	$(Q)$(call MKDIR,$(@D))
	$(Q)convfont -o carray -f $< -a 1 -b 1 -w bold -c 2 -x 9 -l 0x0B -Z $@
