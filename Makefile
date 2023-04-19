# Flags set by elixir_make
ERL_EI_INCLUDE_DIR ?= $(shell erl -eval 'io:format("~s", [lists:concat([code:root_dir(), "/erts-", erlang:system_info(version), "/include"])])' -s init stop -noshell)
ERL_EI_LIBDIR ?= $(shell erl -eval 'io:format("~s", [lists:concat([code:root_dir(), "/erts-", erlang:system_info(version), "/lib"])])' -s init stop -noshell)
ERL_CFLAGS ?= -I$(ERL_EI_INCLUDE_DIR)
ERL_LDFLAGS ?= -L$(ERL_EI_LIBDIR)
MIX_TARGET ?= no-target
MIX_APP_PATH ?= $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
PREFIX = $(MIX_APP_PATH)/priv/$(MIX_TARGET)
CFLAGS ?= -O3 -std=c99 -Wall -Wextra $(ERL_CFLAGS)

$(info    ERL_LDFLAGS is $(ERL_LDFLAGS))

ifeq ($(CROSSCOMPILE),)
    ifneq ($(shell uname -s),Linux)
        # not linux, so use MacOS shared library ld flags
        CFLAGS += -undefined dynamic_lookup -dynamiclib
    else
        CFLAGS += -fPIC -shared
    endif
else
CFLAGS += -fPIC -shared
endif


NIF = $(PREFIX)/exyyjson.so

# Define the NIF source files
NIF_OBJ = c_src/yyjson/yyjson.o c_src/encode_term.o c_src/ex_yyjson.o

# Define the default target
all: $(NIF)

# Define the target to build the NIF library
$(PREFIX)/exyyjson.so: $(PREFIX) $(NIF_OBJ)
	$(CC) $(CFLAGS) $(LD_FLAGS) $(ERL_LDFLAGS) -o $(NIF) $(NIF_OBJ)

$(PREFIX):
	mkdir -p $(PREFIX)

# Define the target to clean the build artifacts
clean:
	$(RM) $(NIF_OBJ) $(NIF)
