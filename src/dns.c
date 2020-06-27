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
  "./dns [-h] [-d] [-r] [-x] [-6] -s server [-p port] hostname/IPv4/IPv6\n\n"
  "Any order of options is acceptable but all of them have to be before non-option inputs. Options:\n"
  "-h: Show help message\n"
  "-d: Enable debug mode\n"
  "-r: Recursion is required (Recursion Desired = 1), otherwise no recursion\n"
  "-x: Reverse request is required instead of directly request\n"
  "-6: Use AAAA instead of default A\n"
  "-s: IP address or domain name of server where is request sent\n"
  "-p port: Port number where is request sent, default 53\n"
  "hostname/IPv4/IPv6: Requested hostname (when is active -x valid IPv4/IPv6 address)\n"
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
  uint32_t rttl = ntohl(dns_rr_data->rttl);

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
int printfNSRecord(unsigned char* response, DNS_RR_Data* dns_rr_data, unsigned char* receive_buffer, unsigned char* rname, uint32_t* rname_length, int debug) {

  int ecode;
  uint32_t rttl = ntohl(dns_rr_data->rttl);

  unsigned char* buffer = (unsigned char*)malloc(MAX_NAME_LENGTH + 1);
  if(buffer == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    return EALLOC;
  }

  if((ecode = readHostFromResourceRecord(response, receive_buffer, buffer, rname_length, debug)) != EOK) {
    fprintf(stderr, "Program could not read resource record.\n");
    return ecode;
  }

  printf("  %s, NS, IN, %i, %s\n", rname, rttl, buffer);

  free(buffer);

  return EOK;
}

/* print CNAME type of RR */
int printfCanonicalNameRecord(unsigned char* response, DNS_RR_Data* dns_rr_data, unsigned char* receive_buffer, unsigned char* rname, uint32_t* rname_length, int debug) {

  int ecode;
  uint32_t rttl = ntohl(dns_rr_data->rttl);

  unsigned char* buffer = (unsigned char*)malloc(MAX_NAME_LENGTH + 1);
  if(buffer == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    return EALLOC;
  }

  if((ecode = readHostFromResourceRecord(response, receive_buffer, buffer, rname_length, debug)) != EOK) {
    fprintf(stderr, "Program could not read resource record.\n");
    return ecode;
  }

  printf("  %s, CNAME, IN, %i, %s\n", rname, rttl, buffer);

  free(buffer);

  return EOK;
}

/* print AAAA type of RR */
int printfIPv6Record(unsigned char* response, DNS_RR_Data* dns_rr_data, unsigned char* rname) {

  uint8_t ecode;
  uint32_t rdata_length = ntohs(dns_rr_data->rdlength);
  uint32_t rttl = ntohl(dns_rr_data->rttl);

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

/* print PTR type of RR */
int printfDomainNamePointerRecord(unsigned char* response, DNS_RR_Data* dns_rr_data, unsigned char* rname, int debug) {

  //uint8_t ecode;
  uint32_t rdata_length = ntohs(dns_rr_data->rdlength);
  uint32_t rttl = ntohl(dns_rr_data->rttl);

  unsigned char* buffer = (unsigned char*)malloc(MAX_NAME_LENGTH + 1);
  if(buffer == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    return EALLOC;
  }

  readDataFromResourceRecord(response, buffer, rdata_length);

  convertHostFromDNSFormat(buffer, buffer, debug);

  printf("  %s, PTR, IN, %i, %s\n", rname, rttl, buffer);

  free(buffer);

  return EOK;
}

/* switch for types of RR */
int printfDnsRecords(unsigned char* response, unsigned char* receive_buffer, DNS_RR_Data* dns_rr_data, unsigned char* rname, uint32_t rname_length, int debug) {
  int ecode = EOK;

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
      response += sizeof(DNS_RR_Data);
      if ((ecode = printfCanonicalNameRecord(response, dns_rr_data, receive_buffer, rname, &rname_length, debug)) != EOK) {
        return ecode;
      }
    } else if (ntohs(dns_rr_data->rtype) == TYPE_PTR) {
      if ((ecode = printfDomainNamePointerRecord(response, dns_rr_data, rname, debug)) != EOK) {
        return ecode;
      }
      response += sizeof(DNS_RR_Data);
    }
  }
  
  return ecode;
}

/* https://tools.ietf.org/html/rfc2929#section-2.3 */
void printRCodeErrorMessage(int code) {
  switch(code) {
    case 1:
      fprintf(stderr, "Format error - The name server was unable to interpret the query.\n");
      break;
    case 2:
      fprintf(stderr, "Server failure - The name server was unable to process this query due to a problem with the name server.\n");
      break;
    case 3:
      fprintf(stderr, "Name Error - Meaningful only for responses from an authoritative name server, this code signifies that the domain name referenced in the query does not exist.\n");
      break;
    case 4:
      fprintf(stderr, "Not Implemented - The name server does not support the requested kind of query.\n");
      break;
    case 5:
      fprintf(stderr, "Refused - The name server refuses to perform the specified operation for policy reasons.  For example, a name server may not wish to provide the information to the particular requester, or a name server may not wish to perform a particular operation (e.g., zone.\n");
      break;
    default:
      fprintf(stderr, "Rcode: %d\n", code);
      break;
  }
}

/* handles process of dns request and response */
int dnsResolver(TParams params, int sock, struct sockaddr_in server_addr, struct sockaddr_in6 server_addr6, int serverIsIpv6) {
  int ecode = EOK;

  // allocated variables
  unsigned char* send_buffer = NULL;
  unsigned char* receive_buffer = NULL;
  unsigned char* rname = NULL;

  uint8_t i = 0;

  // create buffer's
  send_buffer = malloc(sizeof(DNS_Header) + strlen((const char*)params.address) + 2 + sizeof(DNS_Question));
  if(send_buffer == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    return EALLOC;
  }
  receive_buffer = malloc(IP_MAXPACKET);
  if(receive_buffer == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    return EALLOC;
  }

  // add address
  unsigned char* qname = (unsigned char*)(send_buffer + sizeof(DNS_Header));
  convertHostToDNSFormat((unsigned char*)params.address, qname, params.debug);

  // header section
  // https://tools.ietf.org/html/rfc1035 (4.1.1. Header section format)
  DNS_Header* dns_header = (DNS_Header*)send_buffer;
  dns_header->id = htons(getpid());
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
  DNS_Question* dns_question = (DNS_Question*)(send_buffer + sizeof(DNS_Header) + (strlen((const char*)qname) + 2));
  if(params.reverse_lookup) {
    dns_question->qtype = TYPE_PTR;
  } else if(params.ipv6) {
    dns_question->qtype = TYPE_AAAA;
  } else {
    dns_question->qtype = TYPE_A;
  }
  dns_question->qclass = CLASS_IN;

  int sendToReturnValue = 0;
  if(serverIsIpv6) {
    sendToReturnValue = sendto(sock, (char*)send_buffer, sizeof(DNS_Header) + (strlen((const char*)qname) + 2) + sizeof(DNS_Question), 0, (struct sockaddr*)&server_addr6, sizeof(server_addr6));
  } else {
    sendToReturnValue = sendto(sock, (char*)send_buffer, sizeof(DNS_Header) + (strlen((const char*)qname) + 2) + sizeof(DNS_Question), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
  }
  if(sendToReturnValue < 0) {
    perror("sendto()");
    cleanDNSResources(rname, send_buffer, receive_buffer);
    return ESENDTO;
  }
  if(params.debug) {
     fprintf(stderr, "\nDEBUG: dns() Packet has been sucessfully send.\n");
  }

  socklen_t size = sizeof(struct sockaddr_in);
  socklen_t size6 = sizeof(struct sockaddr_in6);
  int recvFromReturnValue = 0;
  if(serverIsIpv6) {
    recvFromReturnValue = recvfrom(sock, (char*)receive_buffer, IP_MAXPACKET, 0, (struct sockaddr*)&server_addr6, &size6);
  } else {
    recvFromReturnValue = recvfrom(sock, (char*)receive_buffer, IP_MAXPACKET, 0, (struct sockaddr*)&server_addr, &size);
  }
  if(recvFromReturnValue < 0) {
    perror("recvfrom()");
    cleanDNSResources(rname, send_buffer, receive_buffer);
    return ERECEIVEFROM;
  } 
  if(params.debug) {
    fprintf(stderr, "\nDEBUG: dns() Packet has been sucessfully received.\n");
  }

  DNS_Header* dns_receive_header = (DNS_Header*) receive_buffer;


  if(dns_receive_header->qr != 1) {
    fprintf(stderr, "Received response is not signed as response.\n");
    cleanDNSResources(rname, send_buffer, receive_buffer);
    return EMALFORMEDPACKET;
  }

  if(dns_receive_header->rcode != 0) {
    printRCodeErrorMessage(dns_receive_header->rcode);
    cleanDNSResources(rname, send_buffer, receive_buffer);
    return EMALFORMEDPACKET;
  }

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
    cleanDNSResources(rname, send_buffer, receive_buffer);
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
    cleanDNSResources(rname, send_buffer, receive_buffer);
    return EMALFORMEDPACKET;
  }

  printf("Answer section (%d):\n", ntohs(dns_receive_header->ancount));
  for(i = 0; i < ntohs(dns_receive_header->ancount); i++)
  {
    if((ecode = readHostFromResourceRecord(response, receive_buffer, rname, &rname_length, params.debug)) != EOK) {
      fprintf(stderr, "Program could not read resource record.\n");
      cleanDNSResources(rname, send_buffer, receive_buffer);
      return ecode;
    }
    response += rname_length;

    dns_rr_data = (DNS_RR_Data*)response;
    rdata_length = ntohs(dns_rr_data->rdlength);

    if(params.debug) {
      fprintf(stderr, "DEBUG: RR type: `%i`\n", dns_rr_data->rtype);
      fprintf(stderr, "DEBUG: RR class: `%i`\n", ntohs(dns_rr_data->rclass));
      fprintf(stderr, "DEBUG: RR ttl: `%i`\n", ntohl(dns_rr_data->rttl));
      fprintf(stderr, "DEBUG: RR rdlength: %i\n", ntohs(dns_rr_data->rdlength));
    }

    if((ecode = printfDnsRecords(response, receive_buffer, dns_rr_data, rname, rname_length, params.debug)) != EOK) {
      fprintf(stderr, "Program could not read resource record.\n");
      cleanDNSResources(rname, send_buffer, receive_buffer);
      return ecode;
    }

    response += rdata_length;
  }



  printf("Authority section (%d):\n", ntohs(dns_receive_header->nscount));
  for(i = 0; i < (int)ntohs(dns_receive_header->nscount); i++)
  {
    if((ecode = readHostFromResourceRecord(response, receive_buffer, rname, &rname_length, params.debug)) != EOK) {
      fprintf(stderr, "Program could not read resource record.\n");
      cleanDNSResources(rname, send_buffer, receive_buffer);
      return ecode;
    }
    response += rname_length;

    dns_rr_data = (DNS_RR_Data*)(response);

    if(params.debug) {
      fprintf(stderr, "DEBUG: RR type: `%i`\n", ntohs(dns_rr_data->rtype));
      fprintf(stderr, "DEBUG: RR class: `%i`\n", ntohs(dns_rr_data->rclass));
      fprintf(stderr, "DEBUG: RR ttl: `%i`\n", ntohl(dns_rr_data->rttl));
      fprintf(stderr, "DEBUG: rdlength: %i\n", ntohs(dns_rr_data->rdlength));
    }

    response += sizeof(DNS_RR_Data);

    if((ecode = printfDnsRecords(response, receive_buffer, dns_rr_data, rname, rname_length, params.debug)) != EOK) {
      fprintf(stderr, "Program could not read resource record.\n");
      cleanDNSResources(rname, send_buffer, receive_buffer);
      return ecode;
    }
  }




  printf("Additional section (%d):\n", ntohs(dns_receive_header->arcount));
  for(i = 0; i < ntohs(dns_receive_header->arcount); i++)
  {
    if((ecode = readHostFromResourceRecord(response, receive_buffer, rname, &rname_length, params.debug)) != EOK) {
      fprintf(stderr, "Program could not read resource record.\n");
      cleanDNSResources(rname, send_buffer, receive_buffer);
      return ecode;
    }
    response += rname_length;

    dns_rr_data = (DNS_RR_Data*)(response);
    rdata_length = ntohs(dns_rr_data->rdlength);

    if(params.debug) {
      fprintf(stderr, "DEBUG: RR type: `%i`\n", ntohs(dns_rr_data->rtype));
      fprintf(stderr, "DEBUG: RR class: `%i`\n", ntohs(dns_rr_data->rclass));
      fprintf(stderr, "DEBUG: RR ttl: `%i`\n", ntohl(dns_rr_data->rttl));
      fprintf(stderr, "DEBUG: rdlength: %i\n", ntohs(dns_rr_data->rdlength));
    }

    if((ecode = printfDnsRecords(response, receive_buffer, dns_rr_data, rname, rname_length, params.debug)) != EOK) {
      fprintf(stderr, "Program could not read resource record.\n");
      cleanDNSResources(rname, send_buffer, receive_buffer);
      return ecode;
    }
  }

  // clean
  cleanDNSResources(rname, send_buffer, receive_buffer);

  return ecode;
}

/* clean all variables allocated inside function dns */
void cleanDNSResources(unsigned char* rname, unsigned char* send_buffer, unsigned char* receive_buffer) {
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

  // server address
  struct addrinfo hints;
  struct addrinfo *results = NULL;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = 0;
  hints.ai_flags = AI_ADDRCONFIG;
  if(getaddrinfo(params.server, NULL, &hints, &results) != 0) {
    fprintf(stderr, "Socket can not be created.\n");
    return EGETADDRINFO;
  }

  // IPv4
  int sock;
  uint8_t serverIsIpv6 = 0;
  struct sockaddr_in server_addr;
  struct sockaddr_in6 server_addr_6;

  do
  {
    if(results->ai_family == AF_INET) {

      // create socket
      if((sock = socket(AF_INET , SOCK_DGRAM , IPPROTO_UDP)) == -1) {
        fprintf(stderr, "Socket can not be created.\n");
        return ESOCKET;
      }

      memset(&server_addr, 0, sizeof(struct sockaddr_in));
      server_addr.sin_addr = ((struct sockaddr_in*)results->ai_addr)->sin_addr;
      server_addr.sin_family = AF_INET;
      server_addr.sin_port = htons(params.port);

      break;
    }
    // IPv6
    else if(results->ai_family == AF_INET6) {
      serverIsIpv6 = 1;

      // create socket
      if((sock = socket(AF_INET6 , SOCK_DGRAM , IPPROTO_UDP)) == -1) {
        fprintf(stderr, "Socket can not be created.\n");
        return ESOCKET;
      }

      memset(&server_addr_6, 0, sizeof(struct sockaddr_in6));
      memcpy(&server_addr_6, results->ai_addr, results->ai_addrlen);
      server_addr_6.sin6_family = AF_INET6;
      server_addr_6.sin6_port = htons(params.port);

      break;
    }
  }
  while((results = results->ai_next) != NULL);

  // timeout
  struct timeval tv;
  tv.tv_sec = params.timeout;
  tv.tv_usec = 0;
  if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof tv) != 0) {
    perror("setsockopt()");

    // clean
    close(sock);
    freeaddrinfo(results);
    cleanParams(params);

    return ESETSOCKETOPT;
  }

  // process dns request and response
  if((ecode = dnsResolver(params, sock, server_addr, server_addr_6, serverIsIpv6)) != 0) {

    // clean
    close(sock);
    freeaddrinfo(results);
    cleanParams(params);

    return ecode;
  }

  // clean
  close(sock);
  freeaddrinfo(results);
  cleanParams(params);

  return ecode;
}
