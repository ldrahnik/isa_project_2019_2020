/**
 * Name: Drahník Lukáš
 * Project: ISA: DNS resolver
 * Date: 30.10.2019
 * Email: <xdrahn00@stud.fit.vutbr.cz>
 * File: dns.h
 */

#ifndef _dns_H_
#define _dns_H_

#include "params.h"
#include "query.h"

#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>

int main(int argc, char *argv[]);
int printfDnsRecords(unsigned char* response, unsigned char* receive_buffer, DNS_RR_Data* dns_rr_data, unsigned char* rname, uint32_t rname_length, int debug);
int dnsResolver(TParams params, int sock, struct sockaddr_in server_addr, struct sockaddr_in6 server_addr6, int serverIsIpv6);
int convertIPv6FromBinaryFormToShortestReadableForm(unsigned char* rdata, unsigned char* shortest_readable);
int readHostFromResourceRecord(unsigned char* reader, unsigned char* buffer, unsigned char* host, uint32_t* host_length, int debug);
void cleanAll(TParams params);
void cleanDNSResources(unsigned char* rname, unsigned char* send_buffer, unsigned char* receive_buffer);
int printfIPv6Record(unsigned char* response, DNS_RR_Data* dns_rr_data, unsigned char* rname);
int printfIPv4Record(unsigned char* response, DNS_RR_Data* dns_rr_data, unsigned char* rname);
int printfNSRecord(unsigned char* response, DNS_RR_Data* dns_rr_data, unsigned char* receive_buffer, unsigned char* rname, uint32_t* host_length, int debug);
int printfDomainNamePointerRecord(unsigned char* response, DNS_RR_Data* dns_rr_data, unsigned char* rname, int debug);
int printfCanonicalNameRecord(unsigned char* response, DNS_RR_Data* dns_rr_data, unsigned char* receive_buffer, unsigned char* rname, uint32_t* rname_length, int debug);
void printRCodeErrorMessage(int code);

#endif
