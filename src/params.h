/**
 * Name: Drahník Lukáš
 * Project: ISA: DNS resolver
 * Date: 30.10.2019
 * Email: <xdrahn00@stud.fit.vutbr.cz>
 * File: params.h
 */

#ifndef _params_H_
#define _params_H_

#include "string.h"

#include "error.h"

typedef struct params {
  int ecode; // error code
  int show_help_message; // show help message
  int debug; // debug
  int recursion_desired; // option -r
  int reverse_lookup; // option -x
  int ipv6; // option -6
  char* server; // option -s server
  int port; // option -p port
  char* address; // non-option address
} TParams;

TParams getParams(int argc, char *argv[]);
void cleanParams(TParams params);

#endif
