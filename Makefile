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

# This pathetic excuse for a Makefile is just a hack until
# the project advances enough for me to want to put the
# effort into setting up the Autotools build-chain.

.PHONY: all
all: $(TARGETS)
	$(MAKE) -C keyvo && $(MAKE) -C keyvo-cli

.PHONY: clean
clean:
	$(MAKE) -C keyvo clean && $(MAKE) -C keyvo-cli clean
