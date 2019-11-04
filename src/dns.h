/**
 * Name: Drahník Lukáš
 * Project: ISA: DNS resolver
 * Date: 30.10.2019
 * Email: <xdrahn00@stud.fit.vutbr.cz>
 * File: dns.h
 */

#ifndef _hpac_H_
#define _hpac_H_

#include "params.h"

#include <stdio.h>
#include <stdlib.h>
#include "string.h"
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/ip_icmp.h>

/*
 * https://www.ietf.org/rfc/rfc1035.txt (3.2.4. CLASS values)
 */
#define CLASS_IN        1 

/*
 * https://www.ietf.org/rfc/rfc1035.txt (3.2.2. TYPE values)
 */
#define TYPE_A          1
#define TYPE_PTR        12

/*
 * https://tools.ietf.org/html/rfc3596#section-2.1 (2.1 AAAA record type)
 */
#define TYPE_AAAA       28

/*
 * https://www.ietf.org/rfc/rfc1035.txt (4.1.1. Header section format) 
 * LE (Little Endian)
 */
typedef struct dns_header
{
    uint16_t id;
 
    uint16_t rd :1;
    uint16_t tc :1;
    uint16_t aa :1;
    uint16_t opcode :4;
    uint16_t qr :1;
 
    uint16_t rcode :4;
    uint16_t z :3;
    uint16_t ra :1;
 
    uint16_t qdcount; // specifying the number of entries in the question section
    uint16_t ancount; // specifying the number of resource records in the answer section
    uint16_t nscount; // specifying the number of name server resource records in the authority records section
    uint16_t arcount; // specifying the number of resource records in the additional records section
} DNS_Header;

/*
 * https://www.ietf.org/rfc/rfc1035.txt (4.1.2. Question section format)
 */
// unsigned char *qname; (variable length)
typedef struct dns_question
{
    unsigned short qtype;
    unsigned short qclass;
} DNS_Question;

/*
 * https://tools.ietf.org/html/rfc1035 (3.2.1. RR Definitions)
 */
// unsigned char *name; (variable length)
typedef struct dns_rr_data
{
    uint16_t rtype;
    uint16_t rclass;
    int rttl;
    uint16_t rdlength;
    unsigned char* rdata;
} DNS_RR_Data;

int main(int argc, char *argv[]);
int readHostFromResourceRecord(unsigned char* reader, unsigned char* buffer, unsigned char* host, uint32_t* host_length, int debug);
void convertHostFromDNSFormat(unsigned char* dns_host_format, unsigned char* host, int debug);
void convertHostToDNSFormat(unsigned char* host, unsigned char* dns_host_format, int debug);
void cleanAll(TParams params);

#endif
