/**
 * Name: Drahník Lukáš
 * Project: ISA: DNS resolver
 * Date: 30.10.2019
 * Email: <xdrahn00@stud.fit.vutbr.cz>
 * File: dns.c
 */
#include "dns.h"

const char *HELP_MSG = {
  "DNS resolver\n\n"	
  "Usage:\n\n"
  "./dns [-r] [-x] [-6] -s server [-p port] address\n\n"
  "Any order of options is acceptable but all of them have to be before non-option inputs. Options:\n"
  "-r: Recursion is required (Recursion Desired = 1), otherwise no recursion\n"
  "-x: Reverse request is required instead of directly request\n"
  "-6: Use AAAA instead of default A\n"
  "-s: IP address or domain name of server where is request sent\n"
  "-p port: port number where is request sent, default 53\n"
  "address: requested address.\n"
};

/* clean */
void cleanAll(TParams params) {
   cleanParams(params);
}

/* www.fit.vutbr.cz -> 3www3fit5vutbr2cz */
/* 29/4/2009 - zdrojový kód - Silver Moon (m00n.silv3r@gmail.com) - https://www.binarytides.com/dns-query-code-in-c-with-linux-sockets/ */
/* 21.10.2019 - upraveno - Lukáš Drahník (xdrahn00@stud.fit.vutbr.cz) */
void convertNameToDNSFormat(unsigned char* host, unsigned char* dns) 
{
    unsigned int lock = 0;
    unsigned int i;
    strcat((char*)host,".");

    for(i = 0 ; i < strlen((char*)host) ; i++) 
    {
        if(host[i]=='.')
        {
            *dns++ = i-lock;
            for(;lock<i;lock++)
                *dns++=host[lock];

            lock++;
        }
    }
    *dns++='0';
    *dns++='\0';
}

/* main */
int main(int argc, char *argv[]) {
  int ecode = EOK;

  // get params
  TParams params = getParams(argc, argv);
  if(params.ecode != EOK) {
    cleanParams(params);
    return params.ecode;
  }

  // help message
  if(params.show_help_message) {
    printf("%s", HELP_MSG);
    cleanParams(params);
    return ecode;
  }

  // create socket
  int s;
  if((s = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP)) == -1) {
    fprintf(stderr, "Socket can not be created.\n");
    cleanParams(params);
    return ESOCKET;
  }

  // destination socket address
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(params.port);
  server_addr.sin_addr.s_addr = inet_addr(params.server);

  // create buffer
  char* buffer = malloc(sizeof(DNS_Header) + (strlen((const char*)params.address) + 3) + sizeof(DNS_Question));
  if(buffer == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    cleanParams(params);
    return EALLOC;
  }

  // convert address to DNS format
  unsigned char* qname = (unsigned char*)(buffer + sizeof(DNS_Header));
  convertNameToDNSFormat((unsigned char*)params.address, qname);

  // header section
  // https://tools.ietf.org/html/rfc1035 (4.1.1. Header section format)
  DNS_Header* dns_header = (DNS_Header*)buffer;
  dns_header->id = (uint16_t)htons(getpid());
  dns_header->qr = 0;
  dns_header->opcode = 0;
  dns_header->aa = 0;
  dns_header->tc = 0;
  dns_header->rd = params.recursion_desired;

  dns_header->ra = 0;
  dns_header->z = 0;
  dns_header->rcode = 0;

  dns_header->qdcount = htons(1);
  dns_header->ancount = 0;
  dns_header->nscount = 0;
  dns_header->arcount = 0;

  // question section
  // https://tools.ietf.org/html/rfc1035 (4.1.2. Question section format)
  DNS_Question* dns_question = (DNS_Question*)(buffer + sizeof(DNS_Header) + (strlen((const char*)qname) + 1));
  if(params.ipv6) {
    dns_question->qtype = htons(TYPE_AAAA);
  } else {
    dns_question->qtype = htons(TYPE_A);
  }
  dns_question->qclass = htons(CLASS_IN);

  if(sendto(s, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
  {
    perror("sendto()");
  }
  if(params.debug) {
     fprintf(stderr, "\nDEBUG: Packet has been sucessfully send.");
  }

  int i = sizeof(server_addr);
  printf("\nDEBUG: Receiving answer...");
  if(recvfrom(s,(char*)buffer, 65535, 0, (struct sockaddr*)&server_addr, (socklen_t*)&i) < 0)
  {
      perror("recvfrom()");
  }
  if(params.debug) {
     fprintf(stderr, "\nDEBUG: Packet has been sucessfully received.");
  }

  dns_header = (DNS_Header*) buffer;

  if(params.debug) {
    printf("\n\nDEBUG: The response contains:\n");
    printf("DEBUG: Question section: %d\n", ntohs(dns_header->qdcount));
    printf("DEBUG: Answer section: %d\n", ntohs(dns_header->ancount));
    printf("DEBUG: Authority records section: %d\n", ntohs(dns_header->nscount));
    printf("DEBUG: Additional records section: %d\n\n", ntohs(dns_header->arcount));
  }

  return ecode;
}
