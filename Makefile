# Name: Drahník Lukáš
# Project: ISA: DNS resolver
# Date: 30.10.2019
# Email: <xdrahn00@stud.fit.vutbr.cz>
# File: Makefile

################# PROJECT #########################

P_NAME = dns
P_DOC_NAME = manual
P_DOC_SOURCE = doc/$(P_DOC_NAME).tex
P_DOC_RESULT = doc/$(P_DOC_NAME).pdf
P_DOC_MAKEFILE = doc/Makefile
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
A_FILES = Makefile $(P_DOC_RESULT) $(P_DOC_MAKEFILE) $(P_SOURCES) $(P_HEADERS)

tar:
	tar -cvzf $(A_NAME).tar $(A_FILES)

rmtar:
	rm -f $(A_NAME).tar

################### DOC ############################

tex:
	cd doc && make doc NAME=$(P_DOC_NAME)

################# CLEAN ############################

clean:
	# clean binary
	rm -f $(P_NAME)

	# clean doc
	cd ./doc/ && make clean
