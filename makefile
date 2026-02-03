# ----------------------------
# Makefile Options
# ----------------------------

NAME = WDS
ICON = icon.png
DESCRIPTION = "water drinking simulator"
COMPRESSED = NO
ARCHIVED = YES

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

include $(shell cedev-config --makefile)
