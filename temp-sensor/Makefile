# name of your application
APPLICATION = temp-sensor
USEMODULE += xtimer
USEMODULE += sx1276
USEMODULE += ps
USEMODULE += od
INCLUDES += -I$(CURDIR)/include

LORA_REGION ?= US915

FEATURES_REQUIRED += periph_i2c

# If no BOARD is found in the environment, use this default:
BOARD ?= istemp-v1

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../../RIOT

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
