/**
 * Name: Drahník Lukáš
 * Project: ISA: DNS resolver
 * Date: 14.11.2019
 * Email: <xdrahn00@stud.fit.vutbr.cz>
 * File: server.h
 */

#include "server.h"

const char *HELP_MSG = {
  "DNS server\n\n"
  "Usage:\n\n"
  "./server [-h] [-d] [-p port]\n\n"
  "Any order of options is acceptable but all of them have to be before non-option inputs. Options:\n"
  "-h: Show help message\n"
  "-d: Enable debug mode\n"
  "-e: Exit server after one request\n"
  "-p port: Port number where is request sent, default 53\n"
};

/* when is pressed ctrl+c */
static int G_break = 0;

/* signal handler */
void catchsignal(int sig) {
  if(sig == SIGINT) {
    G_break = 1;
  }
}

int dnsServer(TParams params) {

  int s;
  fd_set my_set;
  struct timeval tv;
  socklen_t size;

  // allocated variables
  unsigned char* receive_buffer = NULL;
  unsigned char* qname = NULL;

  // server address 
  struct sockaddr_in server_addr_in;
  memset((char *) &server_addr_in, 0, sizeof (server_addr_in));
  server_addr_in.sin_family = AF_INET;
  server_addr_in.sin_port = htons(params.port);
  server_addr_in.sin_addr.s_addr = inet_addr("127.0.0.1");

  // create socket
  if((s = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
    perror("socket()");
    return EXIT_FAILURE;
  }

  // bind
  if((bind(s, (struct sockaddr *)&server_addr_in, sizeof(server_addr_in)) == -1)) {
    perror("bind()");
    return EXIT_FAILURE;
  }

  // create buffer
  receive_buffer = malloc(IP_MAXPACKET);
  if(receive_buffer == NULL) {
    fprintf(stderr, "Allocation fails.\n");
    return EXIT_FAILURE;
  }

  // create question name
  qname = malloc(MAX_NAME_LENGTH + 1);
  if(qname == NULL) {
    fprintf(stderr, "Allocation fails.\n");
   return EXIT_FAILURE;
  }

  // install signal handler for CTRL+C
  signal(SIGINT, catchsignal);

  for(;;) {

   // CTRL+C handler
    if(G_break == 1)
      break;

    FD_ZERO(&my_set);
    FD_SET(s, &my_set);
    if(select(s + 1, &my_set, NULL, NULL, &tv) < 0) {
      perror("select()");
      break;
    }

    if(FD_ISSET(s, &my_set)) {

      size = sizeof(struct sockaddr_in);
      if(recvfrom(s, receive_buffer, IP_MAXPACKET, 0, (struct sockaddr *) &server_addr_in, &size) == -1) {
        perror("recvfrom()");
        return EXIT_FAILURE;
      }

      // DNS header
      DNS_Header* dns_receive_header = (DNS_Header*) receive_buffer;

      printf("########\n");
      printf("rd %i\n", dns_receive_header->rd);
      printf("tc %i\n", dns_receive_header->tc);
      printf("aa %i\n", dns_receive_header->aa);
      printf("opcode %i\n", dns_receive_header->opcode);
      printf("qr %i\n", dns_receive_header->qr);

      printf("rcode %i\n", dns_receive_header->rcode);
      printf("z %i\n", dns_receive_header->z);
      printf("ra %i\n", dns_receive_header->ra);

      printf("qdcount %i\n", htons(dns_receive_header->qdcount));

      // DNS question
      // qname
      unsigned char* dns_response_qname = (receive_buffer + sizeof(DNS_Header));
      convertHostFromDNSFormat(dns_response_qname, qname);
      printf("qname %s\n", qname);

      // type, class
  DNS_Question* dns_response_question = (DNS_Question*) (receive_buffer + sizeof(DNS_Header) + (strlen((const char*)qname) + 2));
      printf("qtype %i\n", ntohs(dns_response_question->qtype));
      printf("qclass %i\n", ntohs(dns_response_question->qclass));

      printf("########\n\n");

      if(params.exit_after_one_request)
        break;
    }
  }

  // clean
  free(receive_buffer);
  free(qname);

  // close sock
  close(s);

  return EXIT_SUCCESS;
}

/* converts 3www3fit5vutbr2cz0 -> www.fit.vutbr.cz */
void convertHostFromDNSFormat(unsigned char* dns_host_format, unsigned char* host) {
  uint32_t i, j, last_number_index = 0;

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
}

/* main */
int main(int argc, char *argv[]) {

  // get params
  TParams params = getParams(argc, argv);
  if(params.ecode != 0) {
    perror("getParams()");
    return EXIT_FAILURE;
  }

  // help message
  if(params.show_help_message) {
    printf("%s", HELP_MSG);
    return EXIT_SUCCESS;
  }

  // run endless loop
  if((dnsServer(params)) != 0) {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
