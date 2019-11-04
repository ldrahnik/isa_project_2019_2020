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

/*
 * read host name with variable length, convert from dns format and returns including length
 * https://tools.ietf.org/html/rfc1035 (3.2. RR definitions)
 */
int readHostFromResourceRecord(unsigned char* reader, unsigned char* buffer, unsigned char* host, uint32_t* host_length, int debug)
{
  uint32_t offset, pointer = 0;
  unsigned char *name; 
  // https://tools.ietf.org/html/rfc1035 (2.3.4. Size limits)
  name = malloc(256);
  if(name == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    return EALLOC;
  }
 
  while(*reader != 0) { // TODO:
    if(*reader >= 192) {
      offset = (*reader)*256 + *(reader + 1) - 49152;
      reader = buffer + offset - 1;
    } else {
      name[pointer++] = *reader;
    }
    reader = reader+1;
  }
 
  name[pointer]='\0';
 
  convertHostFromDNSFormat(name, host, debug);
  *host_length = strlen((char*)host);

  // clean
  free(name);

  return EOK;
}

/* converts 3www3fit5vutbr2cz0 -> www.fit.vutbr.cz */
void convertHostFromDNSFormat(unsigned char* dns_host_format, unsigned char* host, int debug) {
  uint32_t i, j, last_number_index = 0;

  if(debug)
    fprintf(stderr, "DEBUG: convertHostFromDNSFormat() given host in dns format: `%s`\n", dns_host_format);

  for(i = 0; i < strlen((char*)dns_host_format); i++) {
    last_number_index = dns_host_format[i];

    for(j = 0; j < (uint32_t)last_number_index; j++)
    {
       host[i] = dns_host_format[i + 1];
       i++;
    }
    host[i] = '.';
  }

  if(i > 0)
    host[i - 1] = '\0';
  else
    host[0] = '\0';

  if(debug)
    fprintf(stderr, "DEBUG: convertHostFromDNSFormat() given host: `%s`\n", host);

}

/* 
 * converts www.fit.vutbr.cz. -> 3www3fit5vutbr2cz0
 * if is last dot missing is added during params procesing
 * https://tools.ietf.org/html/rfc1035 (4.1.2. Question section format)
 */
void convertHostToDNSFormat(unsigned char* host, unsigned char* dns_host_format, int debug) {
  uint32_t i, j, last_dot_index = 0;

  if(debug)
    fprintf(stderr, "DEBUG: convertHostToDNSFormat() given host: `%s`\n", host);

  for(i = 0; i < strlen((char*)host); i++) {
    if(debug)
      fprintf(stderr, "DEBUG: convertHostToDNSFormat() given host char: `%c`\n", host[i]);

    if(host[i] == '.')
    {
      *dns_host_format++ = i - last_dot_index;

      if(debug)
        fprintf(stderr, "DEBUG: convertHostToDNSFormat() `%i` count of characters before dot on position: `%i`\n", i - last_dot_index, i);

      for(j = last_dot_index; j < i; j++) {
        *dns_host_format++ = host[j];
        if(debug)
          fprintf(stderr, "DEBUG: convertHostToDNSFormat() `%c`\n", host[j]);
      }
      last_dot_index = i + 1;
    }
  }

  *dns_host_format++ = 0;
  *dns_host_format++='\0';

  if(debug) {
    fprintf(stderr, "DEBUG: convertHostToDNSFormat() `%c`\n", '0');
    fprintf(stderr, "DEBUG: convertHostToDNSFormat() `%c`\n", '\0');
  }
}

/* handle process of dns request and response */
int dns(TParams params) {
  int ecode = EOK;

  // create buffer's
  unsigned char* send_buffer = malloc(sizeof(DNS_Header) + (strlen((const char*)params.address) + 2) + sizeof(DNS_Question));
  if(send_buffer == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    return EALLOC;
  }
  unsigned char* receive_buffer = malloc(IP_MAXPACKET);
  if(receive_buffer == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    return EALLOC;
  }

  // create socket
  int s;
  if((s = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP)) == -1) {
    fprintf(stderr, "Socket can not be created.\n");
    return ESOCKET;
  }

  // server address
  struct addrinfo hints;
  struct addrinfo *server;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = 0;
  hints.ai_flags = AI_ADDRCONFIG;
  getaddrinfo(params.server, NULL, &hints, &server);

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(params.port);
  server_addr.sin_addr = ((struct sockaddr_in*)server->ai_addr)->sin_addr;
  server_addr.sin_addr.s_addr = inet_addr(params.server);

  // convert address to DNS format
  unsigned char* qname = (unsigned char*)(send_buffer + sizeof(DNS_Header));
  convertHostToDNSFormat((unsigned char*)params.address, qname, params.debug);

  // header section
  // https://tools.ietf.org/html/rfc1035 (4.1.1. Header section format)
  DNS_Header* dns_header = (DNS_Header*)send_buffer;
  dns_header->id = (uint16_t)htons(getpid());
  dns_header->qr = 0;
  dns_header->opcode = 0;
  dns_header->aa = 0;
  dns_header->tc = 0;
  dns_header->rd = params.recursion_desired;

  dns_header->ra = 0;
  dns_header->z = 0;
  dns_header->rcode = 0;

  dns_header->qdcount = (uint16_t)htons(1);
  dns_header->ancount = 0;
  dns_header->nscount = 0;
  dns_header->arcount = 0;

  // question section
  // https://tools.ietf.org/html/rfc1035 (4.1.2. Question section format)
  DNS_Question* dns_question = (DNS_Question*)(send_buffer + sizeof(DNS_Header) + (strlen((const char*)qname) + 2));
  if(params.reverse_lookup) {
        dns_question->qtype = TYPE_PTR;
  } else if(params.ipv6) {
    dns_question->qtype = TYPE_AAAA;
  } else {
    dns_question->qtype = TYPE_A;
  }
  dns_question->qclass = CLASS_IN;

  if(sendto(s, (char*) send_buffer, sizeof(DNS_Header) + (strlen((const char*)qname) + 2) + sizeof(DNS_Question), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
  {
    perror("sendto()");
    return ESENDTO;
  }
  if(params.debug) {
     fprintf(stderr, "\nDEBUG: dns() Packet has been sucessfully send.\n");
  }

  int ii = sizeof(server_addr);
  if(recvfrom(s,(char*)receive_buffer, IP_MAXPACKET, 0, (struct sockaddr*)&server_addr, (socklen_t*)&ii) < 0)
  {
      perror("recvfrom()");
      return ERECEIVEFROM;
  }
  if(params.debug) {
     fprintf(stderr, "\nDEBUG: dns() Packet has been sucessfully received.\n");
  }

  DNS_Header* dns_receive_header = (DNS_Header*) receive_buffer;
  unsigned char* dns_response_rr = (receive_buffer + sizeof(DNS_Header) + (strlen((const char*)qname) + 1) + sizeof(DNS_Question));

  printf(
    "Authoritative: %s, Recursive: %s, Truncated: %s\n\n",
    dns_receive_header->tc ? "Yes" : "No",
    dns_receive_header->rd ? "Yes" : "No",
    dns_receive_header->tc ? "Yes" : "No"
  );

  uint32_t i, rname_length = 0;
  // https://tools.ietf.org/html/rfc1035 (2.3.4. Size limits)
  unsigned char* rname = malloc(256);
  DNS_RR_Data* dns_rr_data;

  printf("Question section (%d):\n", ntohs(dns_receive_header->qdcount));
  for(i = 0; i < ntohs(dns_receive_header->qdcount); i++)
  {
    if((ecode = readHostFromResourceRecord(dns_response_rr, receive_buffer, rname, &rname_length, params.debug)) != EOK) {
      fprintf(stderr, "Program could not read resource record.\n");
      return ecode;
    }
    dns_rr_data = (DNS_RR_Data*)(dns_response_rr + rname_length);
 
    if(ntohs(dns_rr_data->rtype) == TYPE_A)
    {
       printf("  %s, A, IN\n", rname);
    }
    else if(ntohs(dns_rr_data->rtype) == TYPE_AAAA)
    {
       // printf("  %s, AAAA, IN, %s\n", rname, dns_rr_data->rdata); TODO:
    }

    dns_response_rr = (dns_response_rr + rname_length + sizeof(DNS_RR_Data));
  }

  printf("Answer section (%d):\n", ntohs(dns_receive_header->ancount));
/*  for(i = 0; i < ntohs(dns_receive_header->ancount); i++)
  {
    rname = ReadName(reader, receive_buffer, &rname_length);
    dns_rr_data = (DNS_RR_Data*)(reader + rname_length);

    if(ntohs(dns_rr_data->rtype) == TYPE_A)
    {
       printf("  %s, A, IN, %i\n", rname, ntohs(dns_rr_data->rttl));
    }

    reader = reader + rname_length + sizeof(DNS_RR_Data);
  }*/
  printf("Authority section (%d):\n", ntohs(dns_header->nscount));
  printf("Additional section (%d):\n", ntohs(dns_header->arcount));

  // clean
  freeaddrinfo(server);
  free(rname);
  free(send_buffer);
  free(receive_buffer);

  return ecode;
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

  // process dns
  if((ecode = dns(params)) != 0) {
    cleanParams(params);
    return ecode;
  }

  // clean
  cleanParams(params);

  return ecode;
}
