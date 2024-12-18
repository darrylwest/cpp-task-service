#
# NOTE! this has been replaced with cmake. it's retained here as an example without cmake
#
.PHONY: unit

# CXX = clang++
CXXFLAGS = -std=c++20 -I.. -Wall -Wextra -pthread

# Conditionally define PREFIX only for macOS (Darwin)
ifneq ($(OS), Windows_NT)
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S), Darwin)
        PREFIX ?= $(shell brew --prefix)
	endif
endif

OPENSSL_DIR = $(PREFIX)/opt/openssl@3
OPENSSL_SUPPORT = -DCPPHTTPLIB_OPENSSL_SUPPORT -I$(OPENSSL_DIR)/include -L$(OPENSSL_DIR)/lib -lssl -lcrypto

ifneq ($(OS), Windows_NT)
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Darwin)
		OPENSSL_SUPPORT += -framework CoreFoundation -framework Security
	endif
endif

ZLIB_SUPPORT = -DCPPHTTPLIB_ZLIB_SUPPORT -lz

BROTLI_DIR = $(PREFIX)/opt/brotli
BROTLI_SUPPORT = -DCPPHTTPLIB_BROTLI_SUPPORT -I$(BROTLI_DIR)/include -L$(BROTLI_DIR)/lib -lbrotlicommon -lbrotlienc -lbrotlidec

all: cryptor

cryptor: cryptor.cpp
	$(CXX) -o ../bin/cryptor $(CXXFLAGS) cryptor.cpp $(OPENSSL_SUPPORT) $(ZLIB_SUPPORT) $(BROTLI_SUPPORT) -I../include

unit:
	$(CXX) -o ../bin/unit $(CXXFLAGS) unit.cpp $(OPENSSL_SUPPORT) $(ZLIB_SUPPORT) $(BROTLI_SUPPORT) -I../include


