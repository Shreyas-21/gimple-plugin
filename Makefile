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

first: testing/first.c build/plugin.so
	$(CC) -O2 -fplugin=./build/plugin.so testing/first.c -o build/first

second: testing/second.c build/plugin.so
	$(CC) -O2 -fplugin=./build/plugin.so testing/second.c -o build/second

third: testing/third.c build/plugin.so
	$(CC) -O2 -fplugin=./build/plugin.so testing/third.c -o build/third

.PHONY: all clean first second third
