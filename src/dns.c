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

/* clean entire TParams structure */
void cleanAll(TParams params) {
   cleanParams(params);
}

/*
 * read host name with variable length, convert from dns format and returns including length
 * https://tools.ietf.org/html/rfc1035 (3.2. RR definitions)
 */
int readHostFromResourceRecord(unsigned char* reader, unsigned char* buffer, unsigned char* host, uint32_t* host_length, int debug)
{
  uint32_t offset, jumped = 0, pointer = 0;
  unsigned char *name;
  // https://tools.ietf.org/html/rfc1035 (2.3.4. Size limits)
  name = malloc(256);
  if(name == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    return EALLOC;
  }

  while (*reader != 0) { // TODO:
    if (*reader >= 192) {
      offset = (*reader)*256 + *(reader + 1) - 49152;
      reader = buffer + offset - 1;
      jumped = 1;
    } else {
      name[pointer++] = *reader;
    }

    if(jumped == 0)
      *host_length = *host_length + 1;

    reader = reader + 1;
  }
 
  name[pointer]='\0';
  if(jumped == 1)
    *host_length = *host_length + 1;

  convertHostFromDNSFormat(name, host, debug);

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

/* handles process of dns request and response */
int dnsResolver(TParams params) {
  int ecode = EOK;

  // allocated variables
  struct addrinfo *server = NULL;
  unsigned char* send_buffer = NULL;
  unsigned char* receive_buffer = NULL;
  unsigned char* rname = NULL;
  unsigned char* rdata = NULL;

  // create buffer's
  send_buffer = malloc(sizeof(DNS_Header) + (strlen((const char*)params.address) + 1) + sizeof(DNS_Question));
  if(send_buffer == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    return EALLOC;
  }
  receive_buffer = malloc(IP_MAXPACKET);
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

  // add address
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

  uint32_t i, j, rname_length = 0;
  // https://tools.ietf.org/html/rfc1035 (2.3.4. Size limits)
  rname = malloc(256);
  if(rname == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    return EALLOC;
  }

  DNS_RR_Data* dns_rr_data;
  unsigned char* dns_rr_data_rdata;

  printf("Question section (%d):\n", ntohs(dns_receive_header->qdcount));
  for(i = 0; i < ntohs(dns_receive_header->qdcount); i++)
  {
    if((ecode = readHostFromResourceRecord(dns_response_rr, receive_buffer, rname, &rname_length, params.debug)) != EOK) {
      fprintf(stderr, "Program could not read resource record.\n");
      cleanDNSResources(server, rname, rdata, send_buffer, receive_buffer);
      return ecode;
    }
    dns_rr_data = (DNS_RR_Data*)(dns_response_rr + rname_length + 1);
    dns_rr_data_rdata = (unsigned char*)(dns_rr_data + sizeof(DNS_RR_Data));

    if(ntohs(dns_rr_data->rtype) == TYPE_A)
    {
       rdata = (unsigned char*)malloc(ntohs(dns_rr_data->rdlength) + 1);
       if(rdata == NULL) {
         fprintf(stderr, "Allocation fails.\n");
         cleanDNSResources(server, rname, rdata, send_buffer, receive_buffer);
         return EALLOC;
       }
       for (j = 0; j < ntohs(dns_rr_data->rdlength); j++)
         rdata[j] = dns_rr_data_rdata[j];
 
       rdata[ntohs(dns_rr_data->rdlength)] = '\0';

       printf("ttl: %i\n", ntohs(dns_rr_data->rttl)); // TODO:
       printf("data length: %i\n", ntohs(dns_rr_data->rdlength)); // TODO:

       struct sockaddr_in addr_in;
       long *p;
       p = (long*)rdata;
       addr_in.sin_addr.s_addr = (*p);

       printf("  %s, A, IN, %i, %s\n", rname, ntohs(dns_rr_data->rttl), inet_ntoa(addr_in.sin_addr));
    }
    else if(ntohs(dns_rr_data->rtype) == TYPE_AAAA)
    {
       // printf("  %s, AAAA, IN, %s\n", rname, dns_rr_data->rdata); TODO:
    }

    dns_response_rr = (dns_response_rr + rname_length + sizeof(DNS_RR_Data));
  }

  printf("Answer section (%d):\n", ntohs(dns_receive_header->ancount));
  printf("Authority section (%d):\n", ntohs(dns_header->nscount));
  printf("Additional section (%d):\n", ntohs(dns_header->arcount));

  // clean
  cleanDNSResources(server, rname, rdata, send_buffer, receive_buffer);

  return ecode;
}

/* clean all variables allocated inside function dns */
void cleanDNSResources(struct addrinfo* server, unsigned char* rname, unsigned char* rdata, unsigned char* send_buffer, unsigned char* receive_buffer) {
  if(server != NULL)
    freeaddrinfo(server);
  if(rname != NULL)
    free(rname);
  if(rdata != NULL)
    free(rdata);
  if(send_buffer != NULL)
    free(send_buffer);
  if(receive_buffer != NULL)
    free(receive_buffer);
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
  if((ecode = dnsResolver(params)) != 0) {
    cleanParams(params);
    return ecode;
  }

  // clean
  cleanParams(params);

  return ecode;
}
