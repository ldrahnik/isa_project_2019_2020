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
int readHostFromResourceRecord(unsigned char* response, unsigned char* buffer, unsigned char* host, uint32_t* host_length, int debug)
{
  uint32_t offset, jumped = 0, pointer = 0;
  unsigned char *name;
  name = (unsigned char*)malloc(MAX_NAME_LENGTH + 1);
  if(name == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    return EALLOC;
  }

  *host_length = 1;

  while (*response != 0) {
    if (*response >= 192) {
      offset = (*response)*256 + *(response + 1) - 49152;
      response = buffer + offset - 1;
      jumped = 1;
    } else {
      name[pointer++] = *response;
    }

    if(jumped == 0)
      *host_length = *host_length + 1;

    response += 1;
  }

  if(jumped == 1)
    *host_length = *host_length + 1;

  name[pointer]='\0'; 

  convertHostFromDNSFormat(name, host, debug);

  // clean
  free(name);

  return EOK;
}

/* 
 * 2001:db8::1:0:0:0:1 -> 2001:db8:0:1::1
 * (https://tools.ietf.org/html/rfc5952)
 */
int convertIPv6FromBinaryFormToShortestReadableForm(unsigned char* rdata, unsigned char* shortest_readable) {
  char buffer[INET6_ADDRSTRLEN];
  struct in6_addr in_addr;

  sprintf(buffer, "%02X%02X:%02X%02X:%02X%0X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X", 
    rdata[0], rdata[1],
    rdata[2], rdata[3],
    rdata[4], rdata[5],
    rdata[6], rdata[7],
    rdata[8], rdata[9], 
    rdata[10], rdata[11], 
    rdata[12], rdata[13], 
    rdata[14], rdata[15]
  );

  if(inet_pton(AF_INET6, buffer, &in_addr) != 1)
  {
    fprintf(stderr, "IPv6 addr of one of received records in packet is malformed.\n");
    return EMALFORMEDPACKET;
  }

  if(inet_ntop(AF_INET6, &in_addr, buffer, INET6_ADDRSTRLEN) == NULL) {
    fprintf(stderr, "IPv6 addr of one of received records in packet is malformed.\n");
    return EMALFORMEDPACKET;
  }

  strcpy((char*)shortest_readable, buffer);
  strcat((char*)shortest_readable, "\0");

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

    if(host[i] == '\0') {
      break;
    }
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

void readDataFromResourceRecord(unsigned char* response, unsigned char* buffer, uint32_t rdata_length)
{
  unsigned char* rdata = (unsigned char*)(response + sizeof(DNS_RR_Data));
  uint32_t j;

  for(j = 0; j < rdata_length; j++)
    buffer[j] = rdata[j];

  buffer[rdata_length] = '\0';
}

/* print A type of RR */
int printfIPv4Record(unsigned char* response, DNS_RR_Data* dns_rr_data, unsigned char* rname) {

  uint32_t rdata_length = ntohs(dns_rr_data->rdlength);
  uint32_t rttl = ntohs(dns_rr_data->rttl);

  struct sockaddr_in addr_in;
  long *p;

  unsigned char* buffer = (unsigned char*)malloc(rdata_length + 1);
  if(buffer == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    return EALLOC;
  }

  readDataFromResourceRecord(response, buffer, rdata_length);

  p = (long*)buffer;
  addr_in.sin_addr.s_addr = (*p);
  printf("  %s, A, IN, %i, %s\n", rname, rttl, inet_ntoa(addr_in.sin_addr));

  free(buffer);

  return EOK;
}

/* print NS type of RR */
int printfNSRecord(unsigned char* response, unsigned char* receive_buffer, unsigned char* rname, uint32_t* rname_length, int debug) {

  int ecode;

  unsigned char* buffer = (unsigned char*)malloc(MAX_NAME_LENGTH + 1);
  if(buffer == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    return EALLOC;
  }

  if((ecode = readHostFromResourceRecord(response, receive_buffer, buffer, rname_length, debug)) != EOK) {
    fprintf(stderr, "Program could not read resource record.\n");
    return ecode;
  }

  printf("  %s, NS, IN, %s\n", rname, buffer);

  free(buffer);

  return EOK;
}

/* print CNAME type of RR */
int printfCanonicalNameRecord(unsigned char* response, unsigned char* receive_buffer, unsigned char* rname, uint32_t* rname_length, int debug) {
  
  int ecode;

  unsigned char* buffer = (unsigned char*)malloc(MAX_NAME_LENGTH + 1);
  if(buffer == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    return EALLOC;
  }

  if((ecode = readHostFromResourceRecord(response, receive_buffer, buffer, rname_length, debug)) != EOK) {
    fprintf(stderr, "Program could not read resource record.\n");
    return ecode;
  }

  printf("  %s, CNAME, IN, %s\n", rname, buffer);

  free(buffer);

  return EOK;
}

/* print AAAA type of RR */
int printfIPv6Record(unsigned char* response, DNS_RR_Data* dns_rr_data, unsigned char* rname) {

  uint32_t rdata_length = ntohs(dns_rr_data->rdlength);
  uint32_t rttl = ntohs(dns_rr_data->rttl);
  uint8_t ecode;

  unsigned char* rdata_buffer = (unsigned char*)malloc(rdata_length + 1);
  if(rdata_buffer == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    return EALLOC;
  }

  unsigned char* ip_buffer = (unsigned char*)malloc(INET6_ADDRSTRLEN);
  if(ip_buffer == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    return EALLOC;
  }

  readDataFromResourceRecord(response, rdata_buffer, rdata_length);

  if((ecode = convertIPv6FromBinaryFormToShortestReadableForm(rdata_buffer, ip_buffer)) != EOK) {
     return ecode;
  }
  printf("  %s, AAAA, IN, %i, %s\n", rname, rttl, ip_buffer);

  free(rdata_buffer);
  free(ip_buffer);

  return EOK;
}

/* handles process of dns request and response */
int dnsResolver(TParams params) {
  int ecode = EOK;

  // allocated variables
  struct addrinfo *server = NULL;
  unsigned char* send_buffer = NULL;
  unsigned char* receive_buffer = NULL;
  unsigned char* rname = NULL;

  uint8_t i = 0;

  // create buffer's
  uint8_t required_space_for_host_in_dns_format = 2;
  if(params.reverse_lookup) {
    required_space_for_host_in_dns_format = 1;
  }
  send_buffer = malloc(sizeof(DNS_Header) + strlen((const char*)params.address) + required_space_for_host_in_dns_format + sizeof(DNS_Question));
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
  memset(&server_addr, 0, sizeof(struct sockaddr_in));
  server_addr.sin_addr = ((struct sockaddr_in*)server->ai_addr)->sin_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(params.port);


  // add address
  unsigned char* qname = (unsigned char*)(send_buffer + sizeof(DNS_Header));
  if(params.reverse_lookup) {
    strcpy((char*)qname, params.address);
  } else {
    convertHostToDNSFormat((unsigned char*)params.address, qname, params.debug);
  }

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

  dns_header->qdcount = htons(1);
  dns_header->ancount = 0;
  dns_header->nscount = 0;
  dns_header->arcount = 0;

  // question section
  // https://tools.ietf.org/html/rfc1035 (4.1.2. Question section format)
  DNS_Question* dns_question = (DNS_Question*)(send_buffer + sizeof(DNS_Header) + (strlen((const char*)qname + 1) + required_space_for_host_in_dns_format));
  if(params.reverse_lookup) {
    dns_question->qtype = TYPE_PTR;
  } else if(params.ipv6) {
    dns_question->qtype = TYPE_AAAA;
  } else {
    dns_question->qtype = TYPE_A;
  }
  dns_question->qclass = CLASS_IN;

  if(sendto(s, (char*) send_buffer, sizeof(DNS_Header) + (strlen((const char*)qname) + required_space_for_host_in_dns_format) + sizeof(DNS_Question), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
  {
    perror("sendto()");
    return ESENDTO;
  }
  if(params.debug) {
     fprintf(stderr, "\nDEBUG: dns() Packet has been sucessfully send.\n");
  }

  socklen_t size = sizeof(struct sockaddr_in);
  if(recvfrom(s,(char*)receive_buffer, IP_MAXPACKET, 0, (struct sockaddr*)&server_addr, &size) < 0)
  {
    perror("recvfrom()");
    cleanDNSResources(server, rname, send_buffer, receive_buffer);
    return ERECEIVEFROM;
  }
  if(params.debug) {
     fprintf(stderr, "\nDEBUG: dns() Packet has been sucessfully received.\n");
  }

  DNS_Header* dns_receive_header = (DNS_Header*) receive_buffer;

  if(dns_receive_header->qr != 1) {
    fprintf(stderr, "Received response is not signed as response.\n");
    cleanDNSResources(server, rname, send_buffer, receive_buffer);
    return EMALFORMEDPACKET;
  }

  if(dns_receive_header->rcode == 1) {
    fprintf(stderr, "Format error - The name server was unable to interpret the query.\n");
    cleanDNSResources(server, rname, send_buffer, receive_buffer);
    return EMALFORMEDPACKET;
  }
  else if(dns_receive_header->rcode == 2) {
    fprintf(stderr, "Server failure - The name server was unable to process this query due to a problem with the name server.\n");
    cleanDNSResources(server, rname, send_buffer, receive_buffer);
    return EMALFORMEDPACKET;
  }
  else if(dns_receive_header->rcode == 3) {
    fprintf(stderr, "Name Error - Meaningful only for responses from an authoritative name server, this code signifies that the domain name referenced in the query does not exist.\n");
    cleanDNSResources(server, rname, send_buffer, receive_buffer);
    return EMALFORMEDPACKET;
  }
  else if(dns_receive_header->rcode == 4) {
    fprintf(stderr, "Not Implemented - The name server does not support the requested kind of query.\n");
    cleanDNSResources(server, rname, send_buffer, receive_buffer);
    return EMALFORMEDPACKET;
  }
  else if(dns_receive_header->rcode == 5) {
    fprintf(stderr, "Refused - The name server refuses to perform the specified operation for policy reasons.  For example, a name server may not wish to provide the information to the particular requester, or a name server may not wish to perform a particular operation (e.g., zone.\n");
    cleanDNSResources(server, rname, send_buffer, receive_buffer);
    return EMALFORMEDPACKET;
  }

  printf("%i", dns_receive_header->qr);

  unsigned char* dns_response_qname = (receive_buffer + sizeof(DNS_Header)); 
  DNS_Question* dns_response_question = (DNS_Question*) (receive_buffer + sizeof(DNS_Header) + (strlen((const char*)qname) + 1));
  unsigned char* response = (receive_buffer + sizeof(DNS_Header) + (strlen((const char*)qname) + 1) + sizeof(DNS_Question));
  uint32_t rdata_length = 0;

  printf(
    "Authoritative: %s, Recursive: %s, Truncated: %s\n\n",
    dns_receive_header->tc ? "Yes" : "No",
    dns_receive_header->rd ? "Yes" : "No",
    dns_receive_header->tc ? "Yes" : "No"
  );

  uint32_t rname_length = 0;
  rname = malloc(MAX_NAME_LENGTH + 1);
  if(rname == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    cleanDNSResources(server, rname, send_buffer, receive_buffer);
    return EALLOC;
  }

  DNS_RR_Data* dns_rr_data;



  printf("Question section (%d):\n", ntohs(dns_receive_header->qdcount));

  convertHostFromDNSFormat(dns_response_qname, rname, params.debug);
  if(ntohs(dns_response_question->qclass) == CLASS_IN) {
    if((ntohs(dns_response_question->qtype) == TYPE_A)) { 
        printf("  %s, A, IN\n", rname);
    } else if(params.ipv6 && ntohs(dns_response_question->qtype) == TYPE_AAAA) {
        printf("  %s, AAAA, IN\n", rname);
    } else if (params.reverse_lookup && ntohs(dns_response_question->qtype) == TYPE_PTR) {
        printf("  %s, PTR, IN\n", rname);
    }
  } else {
    fprintf(stderr, "Question of received packet is malformed.\n");
    cleanDNSResources(server, rname, send_buffer, receive_buffer);
    return EMALFORMEDPACKET;
  }



  printf("Answer section (%d):\n", ntohs(dns_receive_header->ancount));
  for(i = 0; i < ntohs(dns_receive_header->ancount); i++)
  {
    if((ecode = readHostFromResourceRecord(response, receive_buffer, rname, &rname_length, params.debug)) != EOK) {
      fprintf(stderr, "Program could not read resource record.\n");
      cleanDNSResources(server, rname, send_buffer, receive_buffer);
      return ecode;
    }
    response += rname_length;

    dns_rr_data = (DNS_RR_Data*)response;
    rdata_length = ntohs(dns_rr_data->rdlength);

    if(params.debug) {
      fprintf(stderr, "DEBUG: RR type: `%i`\n", ntohs(dns_rr_data->rtype));
      fprintf(stderr, "DEBUG: RR class: `%i`\n", ntohs(dns_rr_data->rclass));
      fprintf(stderr, "DEBUG: RR ttl: `%i`\n", ntohs(dns_rr_data->rttl));
      fprintf(stderr, "DEBUG: RR rdlength: %i\n", ntohs(dns_rr_data->rdlength));
    }

    if(ntohs(dns_rr_data->rclass) == CLASS_IN) {
      if(ntohs(dns_rr_data->rtype) == TYPE_A)
      {
        if ((ecode = printfIPv4Record(response, dns_rr_data, rname)) != EOK) {
          return ecode;
        }
        response += sizeof(DNS_RR_Data);
      } else if (ntohs(dns_rr_data->rtype) == TYPE_AAAA) {
        if ((ecode = printfIPv6Record(response, dns_rr_data, rname)) != EOK) {
          return ecode;
        }
        response += sizeof(DNS_RR_Data);
      } else if (ntohs(dns_rr_data->rtype) == TYPE_CNAME) {
        if ((ecode = printfCanonicalNameRecord(response, receive_buffer, rname, &rname_length, params.debug)) != EOK) {
          return ecode;
        }
        response += sizeof(DNS_RR_Data);
      }
    }

    response += rdata_length;
  }



  printf("Authority section (%d):\n", ntohs(dns_receive_header->nscount));
  for(i = 0; i < (int)ntohs(dns_receive_header->nscount); i++)
  {
    if((ecode = readHostFromResourceRecord(response, receive_buffer, rname, &rname_length, params.debug)) != EOK) {
      fprintf(stderr, "Program could not read resource record.\n");
      cleanDNSResources(server, rname, send_buffer, receive_buffer);
      return ecode;
    }
    response += rname_length;

    dns_rr_data = (DNS_RR_Data*)(response);

    if(params.debug) {
      fprintf(stderr, "DEBUG: RR type: `%i`\n", ntohs(dns_rr_data->rtype));
      fprintf(stderr, "DEBUG: RR class: `%i`\n", ntohs(dns_rr_data->rclass));
      fprintf(stderr, "DEBUG: RR ttl: `%i`\n", ntohs(dns_rr_data->rttl));
      fprintf(stderr, "DEBUG: rdlength: %i\n", ntohs(dns_rr_data->rdlength));
    }

    response += sizeof(DNS_RR_Data);

    if(ntohs(dns_rr_data->rclass) == CLASS_IN) {
      if(ntohs(dns_rr_data->rtype) == TYPE_NS) {
        if ((ecode = printfNSRecord(response, receive_buffer, rname, &rname_length, params.debug)) != EOK) {
          return ecode;
        }
        response += rname_length;
      }
    }
  }




  printf("Additional section (%d):\n", ntohs(dns_receive_header->arcount));
  for(i = 0; i < ntohs(dns_receive_header->arcount); i++)
  {
    if((ecode = readHostFromResourceRecord(response, receive_buffer, rname, &rname_length, params.debug)) != EOK) {
      fprintf(stderr, "Program could not read resource record.\n");
      cleanDNSResources(server, rname, send_buffer, receive_buffer);
      return ecode;
    }
    response += rname_length;

    dns_rr_data = (DNS_RR_Data*)(response);
    rdata_length = ntohs(dns_rr_data->rdlength);

    if(params.debug) {
      fprintf(stderr, "DEBUG: RR type: `%i`\n", ntohs(dns_rr_data->rtype));
      fprintf(stderr, "DEBUG: RR class: `%i`\n", ntohs(dns_rr_data->rclass));
      fprintf(stderr, "DEBUG: RR ttl: `%i`\n", ntohs(dns_rr_data->rttl));
      fprintf(stderr, "DEBUG: rdlength: %i\n", ntohs(dns_rr_data->rdlength));
    }

    if(ntohs(dns_rr_data->rclass) == CLASS_IN) {
      if(ntohs(dns_rr_data->rtype) == TYPE_A) {
        if ((ecode = printfIPv4Record(response, dns_rr_data, rname)) != EOK) {
          return ecode;
        }
        response += sizeof(DNS_RR_Data);
        response += rdata_length;
      } else if (ntohs(dns_rr_data->rtype) == TYPE_AAAA) {
        if ((ecode = printfIPv6Record(response, dns_rr_data, rname)) != EOK) {
          return ecode;
        }
        response += sizeof(DNS_RR_Data);
        response += rdata_length;
      } else if(ntohs(dns_rr_data->rtype) == TYPE_NS) {
        if ((ecode = printfNSRecord(response, receive_buffer, rname, &rname_length, params.debug)) != EOK) {
          return ecode;
        }
        response += sizeof(DNS_RR_Data);
        response += rname_length;
      }
    }
  }

  // clean
  cleanDNSResources(server, rname, send_buffer, receive_buffer);

  return ecode;
}

/* clean all variables allocated inside function dns */
void cleanDNSResources(struct addrinfo* server, unsigned char* rname, unsigned char* send_buffer, unsigned char* receive_buffer) {
  if(server != NULL)
    freeaddrinfo(server);
  if(rname != NULL)
    free(rname);
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
