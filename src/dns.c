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


/* 29/4/2009 - zdrojový kód - Silver Moon (m00n.silv3r@gmail.com) - https://www.binarytides.com/dns-query-code-in-c-with-linux-sockets/ */
u_char* ReadName(unsigned char* reader,unsigned char* buffer,int* count)
{
    unsigned char *name;
    unsigned int p=0,jumped=0,offset;
    int i , j;
 
    *count = 1;
    name = (unsigned char*)malloc(256);
 
    name[0]='\0';
 
    //read the names in 3www6google3com format
    while(*reader!=0)
    {
        if(*reader>=192)
        {
            offset = (*reader)*256 + *(reader+1) - 49152; //49152 = 11000000 00000000 ;)
            reader = buffer + offset - 1;
            jumped = 1; //we have jumped to another location so counting wont go up!
        }
        else
        {
            name[p++]=*reader;
        }
 
        reader = reader+1;
 
        if(jumped==0)
        {
            *count = *count + 1; //if we havent jumped to another location then we can count up
        }
    }
 
    name[p]='\0'; //string complete
    if(jumped==1)
    {
        *count = *count + 1; //number of steps we actually moved forward in the packet
    }
 
    //now convert 3www6google3com0 to www.google.com
    for(i=0;i<(int)strlen((const char*)name);i++) 
    {
        p=name[i];
        for(j=0;j<(int)p;j++) 
        {
            name[i]=name[i+1];
            i=i+1;
        }
        name[i]='.';
    }
    name[i-1]='\0'; //remove the last dot
    return name;
}

/* www.fit.vutbr.cz. -> 3www3fit5vutbr2cz */
/* if is last dot missing is added during params procesing */
/* 29/4/2009 - zdrojový kód - Silver Moon (m00n.silv3r@gmail.com) - https://www.binarytides.com/dns-query-code-in-c-with-linux-sockets/ */
/* 21.10.2019 - upraveno - Lukáš Drahník (xdrahn00@stud.fit.vutbr.cz) */
void convertNameToDNSFormat(unsigned char* host, unsigned char* dns_host_format, int debug) 
{
    uint32_t i, last_dot_index = 0;

    if(debug)
        fprintf(stderr, "DEBUG: given host: `%s`\n", host);

    for(i = 0; i < strlen((char*)host); i++) 
    {
        if(host[i] == '.')
        {
            *dns_host_format++ = i - last_dot_index;

            for(;last_dot_index < i; last_dot_index++){
                *dns_host_format++ = host[last_dot_index];
            }

            last_dot_index = i + 1;
        }
    }
    *dns_host_format++='\0';

    if(debug)
        fprintf(stderr,"DEBUG: given host translated to DNS format: `%s`\n", dns_host_format); // TODO:
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
  convertNameToDNSFormat((unsigned char*)params.address, qname, params.debug);

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
  if(params.ipv6) {
    dns_question->qtype = 28; //htons(TYPE_AAAA); TODO:
  } else {
    dns_question->qtype = 1; //htons(TYPE_A); //TODO:
  }
  dns_question->qclass = 1; //htons(CLASS_IN); //TODO:

  if(sendto(s, (char*) send_buffer, sizeof(DNS_Header) + (strlen((const char*)qname) + 2) + sizeof(DNS_Question), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
  {
    perror("sendto()");
    return ESENDTO;
  }
  if(params.debug) {
     fprintf(stderr, "\nDEBUG: Packet has been sucessfully send.\n");
  }

  int ii = sizeof(server_addr);
  if(recvfrom(s,(char*)receive_buffer, IP_MAXPACKET, 0, (struct sockaddr*)&server_addr, (socklen_t*)&ii) < 0)
  {
      perror("recvfrom()");
      return ERECEIVEFROM;
  }
  if(params.debug) {
     fprintf(stderr, "\nDEBUG: Packet has been sucessfully received.\n");
  }

  DNS_Header* dns_receive_header = (DNS_Header*) receive_buffer;
  unsigned char* reader = (receive_buffer + sizeof(DNS_Header) + (strlen((const char*)qname) + 1) + sizeof(DNS_Question));

  int rname_length = 0;
  printf(
    "Authoritative: %s, Recursive: %s, Truncated: %s\n\n",
    dns_receive_header->tc ? "Yes" : "No",
    dns_receive_header->rd ? "Yes" : "No",
    dns_receive_header->tc ? "Yes" : "No"
  );

  uint16_t i;
  unsigned char* rname;
  DNS_RR_Data* dns_rr_data;

  printf("Question section (%d):\n", ntohs(dns_receive_header->qdcount));
  for(i = 0; i < ntohs(dns_receive_header->qdcount); i++)
  {
    rname = ReadName(reader, receive_buffer, &rname_length);
    dns_rr_data = (DNS_RR_Data*)(reader + rname_length);
 
    if(ntohs(dns_rr_data->rtype) == TYPE_A)
    {
       printf("  %s, A, IN\n", rname);
    }

    reader = (reader + rname_length + sizeof(DNS_RR_Data));
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
