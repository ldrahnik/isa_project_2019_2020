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
#include <errno.h>

/*
 * https://www.ietf.org/rfc/rfc1035.txt (3.2.4. CLASS values)
 */
#define CLASS_IN        1 

/*
 * https://www.ietf.org/rfc/rfc1035.txt (3.2.2. TYPE values)
 */
#define TYPE_A          1
#define TYPE_NS         2
#define TYPE_CNAME      5
#define TYPE_PTR        12
/*
 * https://tools.ietf.org/html/rfc3596#section-2.1 (2.1 AAAA record type)
 */
#define TYPE_AAAA       28

/*
 * https://tools.ietf.org/html/rfc1035 (2.3.4. Size limits)
 */
#define MAX_NAME_LENGTH 256

/*
 * https://www.ietf.org/rfc/rfc1035.txt (4.1.1. Header section format) 
 * LE (Little Endian)
 */
typedef struct dns_header
{
    uint16_t id;
 
    uint8_t rd :1;
    uint8_t tc :1;
    uint8_t aa :1;
    uint8_t opcode :4;
    uint8_t qr :1;
 
    uint8_t rcode :4;
    uint8_t z :3;
    uint8_t ra :1;
 
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
    uint16_t qtype;
    uint16_t qclass;
} DNS_Question;

/*
 * https://tools.ietf.org/html/rfc1035 (3.2.1. RR Definitions)
 * pragma required for correct alignment
 */
#pragma pack(push, 1)
// unsigned char *rname; (variable length)
typedef struct dns_rr_data
{
    uint16_t rtype;
    uint16_t rclass;
    int rttl;
    uint16_t rdlength;
} DNS_RR_Data;
// unsigned char *rdata; (variable length)
#pragma pack(pop)

int main(int argc, char *argv[]);
int dnsResolver(TParams params);
int readHostFromResourceRecord(unsigned char* reader, unsigned char* buffer, unsigned char* host, uint32_t* host_length, int debug);
void convertHostFromDNSFormat(unsigned char* dns_host_format, unsigned char* host, int debug);
void cleanAll(TParams params);
void cleanDNSResources(struct addrinfo* server, unsigned char* rname, unsigned char* send_buffer, unsigned char* receive_buffer);
int printfIPv6Record(unsigned char* response, DNS_RR_Data* dns_rr_data, unsigned char* rname);
int printfIPv4Record(unsigned char* response, DNS_RR_Data* dns_rr_data, unsigned char* rname);
int printfNSRecord(unsigned char* response, DNS_RR_Data* dns_rr_data, unsigned char* receive_buffer, unsigned char* rname, uint32_t* host_length, int debug);
int printfDomainNamePointerRecord(unsigned char* response, DNS_RR_Data* dns_rr_data, unsigned char* rname, int debug);
int printfCanonicalNameRecord(unsigned char* response, DNS_RR_Data* dns_rr_data, unsigned char* receive_buffer, unsigned char* rname, uint32_t* rname_length, int debug);

#endif
