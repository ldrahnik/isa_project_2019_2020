/**
 * Name: Drahník Lukáš
 * Project: ISA: DNS resolver
 * Date: 14.11.2019
 * Email: <xdrahn00@stud.fit.vutbr.cz>
 * File: server.h
 */

#ifndef _server_H_
#define _server_H_

#include "params.h"
#include "dns.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <netdb.h>

int main(int argc, char *argv[]);
int dnsServer(TParams params);
void convertHostFromDNSFormat(unsigned char* dns_host_format, unsigned char* host);

#endif
