# Exported from https://tiplanet.org/pb/ on Fri Nov 11 04:11:54 2022 (CET)

# ----------------------------
# Program Options
# ----------------------------

NAME         ?= MNDELBRT
ICON         ?= icon.png
DESCRIPTION  ?= "Mandelbrot and julia set"
COMPRESSED   ?= YES
ARCHIVED     ?= YES
HAS_PRINTF   := YES

# ----------------------------
# Compile Options
# ----------------------------

CFLAGS   ?= -Oz -W -Wall -Wextra -Wwrite-strings
CXXFLAGS ?= -Oz -W -Wall -Wextra -Wwrite-strings

# ----------------------------

include $(shell cedev-config --makefile)
