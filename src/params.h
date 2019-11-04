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
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h> // isprint
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "error.h"

typedef struct params {
  uint8_t ecode; // error code
  uint16_t show_help_message :1; // show help message
  uint16_t debug :1; // debug
  uint16_t recursion_desired :1; // option -r
  uint16_t reverse_lookup :1; // option -x
  uint16_t ipv6 :1; // option -6
  char* server; // option -s server
  uint32_t port; // option -p port
  char* address; // non-option address
} TParams;

TParams getParams(int argc, char *argv[]);
int isHostValid(char* node);
int isIPv4VersionAddress(char *ip_address);
int isIPv6VersionAddress(char *ip_address);
void cleanParams(TParams params);

#endif
