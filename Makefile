# Name: Drahník Lukáš
# Project: ISA: DNS resolver
# Date: 30.10.2019
# Email: <xdrahn00@stud.fit.vutbr.cz>
# File: Makefile

################# PROJECT #########################

P_NAME = dns
P_DOC_NAME = manual
P_LICENSE = LICENSE
P_MAN_PAGE = dns.1
P_SPEC = dns.spec
P_DOC_FOLDER = doc/
P_DOC_RESULT = $(P_DOC_NAME).pdf
P_DOC_MAKEFILE = doc/Makefile
P_README = README
P_TESTS = tests/tests.sh tests/ref/* tests/server/src/*.c tests/server/src/*.h tests/server/Makefile tests/server/Readme.md tests/logs/
P_SOURCES = src/*.c
P_HEADERS = src/*.h

################# FLAGS ###########################

CC = gcc
CFLAGS = -std=gnu99 -Wextra -Werror -pedantic -g -Wall
P_FLAGS = -lm

################# BUILD ###########################

all: $(P_NAME)

$(P_NAME): $(P_SOURCES) $(P_HEADERS)
	$(CC) $(CFLAGS) $(P_SOURCES) -o $(P_NAME) $(P_FLAGS)

################# ARCHIVE #########################

A_NAME = xdrahn00
A_FILES = Makefile $(P_SOURCES) $(P_HEADERS) $(P_README) $(P_MAN_PAGE) $(P_SPEC) $(P_LICENSE) $(P_TESTS) # exceptions are archive files placed in subfolders 

tar:
	# build doc
	make tex

	# pack
	tar -cvzf $(A_NAME).tar $(A_FILES) -C $(P_DOC_FOLDER) $(P_DOC_RESULT)

untar:
	rm -rf $(A_NAME) && mkdir -p $(A_NAME) && tar -C $(A_NAME) -xvzf $(A_NAME).tar

rmtar:
	rm -f $(A_NAME).tar

################## INSTALL #########################

BUILD_ROOT = 
VERSION	=
INSTALL_DIR	= $(BUILD_ROOT)/usr/lib/$(P_NAME)
INSTALL_SOURCES = $(P_NAME)

MAN_DIR	= $(BUILD_ROOT)/usr/share/man/man1
BIN_DIR	= $(BUILD_ROOT)/usr/bin

SHARE_DIR = $(BUILD_ROOT)/usr/share
DOC_DIR = $(SHARE_DIR)/doc/$(P_NAME)
LICENSES_DIR = $(SHARE_DIR)/licenses/$(P_NAME)

install:
	mkdir -p $(INSTALL_DIR)
	mkdir -p $(BIN_DIR)
	mkdir -p $(MAN_DIR)
	mkdir -p $(DOC_DIR)
	mkdir -p $(LICENSES_DIR)
	install -m 0644 $(INSTALL_SOURCES) $(INSTALL_DIR)
	install -m 0644 $(P_LICENSE) $(LICENSES_DIR)
	install -m 0644 $(P_DOC_RESULT) $(DOC_DIR)
	cd $(BUILD_ROOT) && sudo ln -sf /usr/lib/$(P_NAME)/$(P_NAME) $(BUILD_ROOT)/usr/bin/$(P_NAME)
	sudo chmod 0755 $(INSTALL_DIR)/$(P_NAME)
	install -m 0644 $(P_MAN_PAGE) $(MAN_DIR)

################# TESTS ############################

test:
	bash ./tests/tests.sh $(PWD)/$(P_NAME)

################### SERVER #########################

server:
	cd tests/server && make

serverclean:
	cd tests/server && make clean

################### DOC ############################

tex:
	cd doc && make doc NAME=$(P_DOC_NAME)

################# CLEAN ############################

clean:
	# clean binary
	rm -f $(P_NAME)

	# clean doc
	cd ./doc/ && make clean

	# clean server
	cd tests/server && make clean
