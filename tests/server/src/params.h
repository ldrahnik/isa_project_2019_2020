/**
 * Name: Drahník Lukáš
 * Project: ISA: DNS resolver
 * Date: 14.11.2019
 * Email: <xdrahn00@stud.fit.vutbr.cz>
 * File: params.h
 */

#ifndef _params_H_
#define _params_H_

#include "string.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <ctype.h>

typedef struct params {
  uint8_t ecode :1; // error code (0/1)
  uint8_t show_help_message :1; // show help message
  uint8_t debug :1; // debug
  uint8_t exit_after_one_request :1; // option -e
  uint32_t port; // option -p port
} TParams;

TParams getParams(int argc, char *argv[]);

#endif
