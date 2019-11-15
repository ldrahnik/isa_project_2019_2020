# Name: Drahník Lukáš
# Project: ISA: DNS resolver
# Date: 14.11.2019
# Email: <xdrahn00@stud.fit.vutbr.cz>
# File: Makefile

################# PROJECT #########################

P_NAME = server
P_MAKEFILE = Makefile
P_README = Readme.md
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

################# CLEAN ############################

clean:
	# clean binary
	rm -f $(P_NAME)
