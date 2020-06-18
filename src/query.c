/**
 * Name: Drahník Lukáš
 * Project: ISA: DNS resolver
 * Date: 18.6.2020
 * Email: <xdrahn00@stud.fit.vutbr.cz>
 * File: query.c
 */
#include "query.h"

/* converts 3www3fit5vutbr2cz0 -> www.fit.vutbr.cz. */
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

  if(i > 0) {
    host[i - 1] = '.';
    host[i] = '\0';
  } else {
    host[0] = '\0';
  }

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

/* 4321:0:1:2:3:4:567:89ab -> b.a.9.8.7.6.5.0.4.0.0.0.3.0.0.0.2.0.0.0.1.0.0.0.0.0.0.0.1.2.3.4.IP6.ARPA. */
/* https://tools.ietf.org/html/rfc3596#section-2.5 (2.5 IP6.ARPA Domain) */
int convertIPv6ToARPAFormat(char* address, char* address_arpa_format, int debug) {
  struct in6_addr in6_address; 
  uint16_t i; 
  uint8_t arpa_format_length = INET6_ADDRSTRLEN + 20;
  char buffer[arpa_format_length];

  inet_pton(AF_INET6, address, &in6_address);

  sprintf(buffer, "%02x%02x....%02x%02x....%02x%02x....%02x%02x....%02x%02x....%02x%02x....%02x%02x....%02x%02x....",
    in6_address.s6_addr[15], in6_address.s6_addr[14],
    in6_address.s6_addr[13], in6_address.s6_addr[12],
    in6_address.s6_addr[11], in6_address.s6_addr[10],
    in6_address.s6_addr[9], in6_address.s6_addr[8],
    in6_address.s6_addr[7], in6_address.s6_addr[6],
    in6_address.s6_addr[5], in6_address.s6_addr[4],
    in6_address.s6_addr[3], in6_address.s6_addr[2],
    in6_address.s6_addr[1], in6_address.s6_addr[0]
  );

  if(debug) {
    fprintf(stderr, "DEBUG: convertIPv6ToARPAFormat() address after funcs inet_pton() without correct order and with dots: `%s`\n\n", buffer);
  }

  // for example: 9a08.... -> a.9.8.0. (is taken 8 characters every loop)
  for(i = 0; i < arpa_format_length - strlen(IP6_ARPA_TERMINATION); i = i + 8) {

    // 9a08.... -> 9a088...
    // 9a088... -> 9a0.8...
    buffer[i + 4] = buffer[i + 3];
    buffer[i + 3] = '.';

    // 9a0.8... -> 9a0.8.0.
    buffer[i + 6] = buffer[i + 2];

    // 9a0.8.0. -> 9a9.8.0.
    // 9a9.8.0. -> aa9.8.0.
    // aa9.8.0. -> a.9.8.0.
    buffer[i + 2] = buffer[i];
    buffer[i] = buffer[i + 1];
    buffer[i + 1] = '.';
  }

  if(debug) {
    fprintf(stderr, "DEBUG: convertIPv6ToARPAFormat() address in arpa format (without termination string): `%s`\n\n", buffer);
  }

  strcpy(address_arpa_format, buffer);
  strcat(address_arpa_format, IP6_ARPA_TERMINATION);
  strcat(address_arpa_format, "\0");

  if(debug)
    fprintf(stderr, "DEBUG: convertIPv6ToARPAFormat() address in arpa format: `%s`\n\n", address_arpa_format);

  return EOK;
}

/* 147.229.8.12 -> 12.8.229.147.IN-ADDR.ARPA. */
/* https://tools.ietf.org/html/rfc1035 (3.5. IN-ADDR.ARPA domain) */
int convertIPv4ToARPAFormat(char *address, char* address_arpa_format, int debug) {
  struct in_addr in_address;
  uint8_t arpa_format_length = INET_ADDRSTRLEN + strlen(IP4_ARPA_TERMINATION) + 1;
  char buffer[arpa_format_length];

  inet_pton(AF_INET, address, &in_address);

  if(debug)
    fprintf(stderr, "DEBUG: convertIPv4ToARPAFormat() address after funcs inet_pton(): `%s`\n", inet_ntoa(in_address));

  in_address.s_addr = 
    ((in_address.s_addr & 0xff000000) >> 24) | ((in_address.s_addr & 0x00ff0000) >>  8) |
    ((in_address.s_addr & 0x0000ff00) <<  8) | ((in_address.s_addr & 0x000000ff) << 24);

  //buffer = inet_ntoa(in_address, buffer);
  inet_ntop(AF_INET, &in_address, buffer, sizeof(buffer));

  strcat(buffer, ".");

  if(debug)
    fprintf(stderr, "DEBUG: convertIPv4ToARPAFormat() address in arpa format (without termination string): `%s`\n", buffer);

  strcat(buffer, IP4_ARPA_TERMINATION);

  if(debug)
    fprintf(stderr, "DEBUG: convertIPv4ToARPAFormat() address in arpa format: `%s`, length: `%li`\n", buffer, strlen(buffer));

  strcpy(address_arpa_format, buffer);
  strcat(address_arpa_format, "\0");

  return EOK;
}

