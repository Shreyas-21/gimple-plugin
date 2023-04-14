CXX = g++
CC = gcc
CXXFLAGS = -std=c++11 -Wall -fno-rtti -O2
CXXFLAGS += -Wno-literal-suffix

PLUGINDIR=$(shell $(CXX) -print-file-name=plugin)
CXXFLAGS += -I$(PLUGINDIR)/include

all: build/plugin.so test

build/plugin.so: build/plugin.o
	$(CXX) $(LDFLAGS) -g -shared -o $@ $^

build/plugin.o : plugin.cc
	$(CXX) $(CXXFLAGS) -g -fPIC -c $< -o $@

clean:
	rm -f build/plugin.so build/plugin.o build/test

test: test.c build/plugin.so
	$(CC) -O2 -fplugin=./build/plugin.so test.c -o build/test

.PHONY: all clean test
