# Keyvo - Key-Value Caching Server
# Copyright (C) Jose Fernando Lopez Fernandez, 2020.
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

vpath %.c src

CC       := gcc
CFLAGS   := -std=c17 -Wall -Wextra -Wpedantic -O3 -march=native
CPPFLAGS := -Iinclude  -D_POSIX_C_SOURCE=200112L -D_GNU_SOURCE
LDFLAGS  := 
LIBS     :=

RM       := rm -f

SRCS     := $(notdir $(wildcard src/*.c))
OBJS     := $(patsubst %.c,%.o,$(SRCS))

TARGET   := keyvo

.PHONY: all
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $^ $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -c -o $@ $^

.PHONY: clean
clean:
	$(RM) $(OBJS) $(TARGET)
