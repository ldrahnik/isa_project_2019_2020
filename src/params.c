/**
 * Name: Drahník Lukáš
 * Project: ISA: DNS resolver
 * Date: 30.10.2019
 * Email: <xdrahn00@stud.fit.vutbr.cz>
 * File: params.c
 */

#include "params.h"

/**
 * Get TParams structure from terminal options
 *
 * @return TParams
 */
TParams getParams(int argc, char *argv[]) {

  // params
  TParams params = {
    .show_help_message = 0,
    .debug = 0,
    .ecode = EOK,
    .recursion_desired = 0,
    .reverse_lookup = 0,
    .ipv6 = 0,
    .server = NULL,
    .port = 53,
    .address = NULL,
  };

  // don't want getopt() writing to stderr
  opterr = 0;

  // getopt
  int c;
  while ((c = getopt(argc, argv, "hdrx6s:p:")) != -1) {
    switch (c) {
      case 'h':
        if(params.show_help_message) {
	      fprintf(stderr, "Option -h is already used.\n");
          params.ecode = EOPT;
          return params;
	    }
        params.show_help_message = 1;
 	    return params;
      case 'd':
        if(params.debug) {
	      fprintf(stderr, "Option -d is already used.\n");
          params.ecode = EOPT;
          return params;
	    }
        params.debug = 1;
        break;
      case 'r':
        if(params.recursion_desired) {
	      fprintf(stderr, "Option -r is already used.\n");
          params.ecode = EOPT;
          return params;
	    }
        params.recursion_desired = 1;
        break;
      case 'x':
        if(params.reverse_lookup) {
	      fprintf(stderr, "Option -x is already used.\n");
          params.ecode = EOPT;
          return params;
	    }
        params.reverse_lookup = 1;
        break;
      case '6':
        if(params.ipv6) {
	      fprintf(stderr, "Option -6 is already used.\n");
          params.ecode = EOPT;
          return params;
	    }
        params.ipv6 = 1;
        break;
      case 's':
        if(params.server != NULL) {
	      fprintf(stderr, "Option error. Option -s is already used.\n");
          params.ecode = EOPT;
          return params;
	    }
	    if(strcmp(optarg, "") == 0) {
	      fprintf(stderr, "Option error. Remove option -s, value can not be empty.\n");
          params.ecode = EOPT;
          return params;
	    }
	    params.server = malloc(strlen(optarg) + 1);
	    if(params.server == NULL) {
          params.ecode = EALLOC;
	      fprintf (stderr, "Allocation fails.\n");
          return params;
        }
        strcpy(params.server, optarg);
        params.server[strlen(optarg)] = '\0';
        break;
      case 'p':
        params.port = atoi(optarg);
        break;
      case '?':
        if(optopt == 's' || optopt == 'e') {
          fprintf(stderr, "Option error. Option -%c requires an argument.\n", optopt);
        } else if(isprint (optopt)) {
          fprintf(stderr, "Option error. Unknown option `-%c'.\n", optopt);
        } else {
          fprintf (stderr, "Option error. Unknown option character `\\x%x'.\n", optopt);
        }
        params.ecode = EOPT;
        return params;
      default:
        fprintf(stderr, "Option error. Options error.\n");
        params.ecode = EOPT;
        return params;
    }
  }

  // address is required
  if(argv[optind] == NULL) {
    fprintf(stderr, "Option error. Address is required.\n");
    params.ecode = EOPT;
    return params;
  }

  // combination of -x and -6 is not allowed
  if(params.reverse_lookup && params.ipv6) {
    fprintf(stderr, "Option error. Type in DNS question can not be at the same time PTR (-x) and AAAA (-6). Please remove one of mentioned types.\n");
    params.ecode = EOPT;
    return params;
  }

  // server is required
  if(params.server == NULL) {
    fprintf(stderr, "Option error. Server is required.\n");
    params.ecode = EOPT;
    return params;
  }

  // validate given server
  if(isHostValid(params.server) != 0) {
    fprintf(stderr, "Option error. Given server is not valid.\n");
    params.ecode = EOPT;
    return params;
  }

  // for example catch this case: ./hpac "A B" -s f ggg
  if(argv[optind + 1] != NULL) {
    fprintf(stderr, "Option error. Please use optional options before non-options.\n");
    params.ecode = EOPT;
    return params;
  }

  // translate given ip address to ARPA format
  if(params.reverse_lookup) {
    /* https://tools.ietf.org/html/rfc1035 (3.5. IN-ADDR.ARPA domain) */
    if(isIPv4VersionAddress(argv[optind])) {
      params.address = malloc(INET_ADDRSTRLEN + strlen(IP4_ARPA_TERMINATION) + 1);
      if(params.address == NULL) {
        params.ecode = EALLOC;
        fprintf(stderr, "Option error. Allocation fails.\n");
        return params;
      }
      if(convertIPv4ToARPAFormat(argv[optind], params.address, params.debug) != EOK) {
        fprintf(stderr, "Option error. Address is not valid.\n");
        params.ecode = EOPT;
        return params;
      }
    /* https://tools.ietf.org/html/rfc3596#section-2.5 (2.5 IP6.ARPA Domain) */
    } else if(isIPv6VersionAddress(argv[optind])) {
      params.address = malloc(INET6_ADDRSTRLEN + 20 + strlen(IP6_ARPA_TERMINATION) + 1);
      if(params.address == NULL) {
        params.ecode = EALLOC;
        fprintf(stderr, "Option error. Allocation fails.\n");
        return params;
      }
      if(convertIPv6ToARPAFormat(argv[optind], params.address, params.debug) != EOK) {
        fprintf(stderr, "Option error. Address is not valid.\n");
        params.ecode = EOPT;
        return params;
      }
    } else {
      fprintf(stderr, "Option error. Address is not valid.\n");
      params.ecode = EOPT;
      return params;
    }
  // or host
  } else {
    // add last dot if not exists
    uint8_t required_space_for_last_dot = 1;
    if(argv[optind][strlen(argv[optind]) - 1] == '.') {
      required_space_for_last_dot = 0;
    }

    params.address = malloc(strlen(argv[optind]) + 1 + required_space_for_last_dot);
    if(params.address == NULL) {
      params.ecode = EALLOC;
      fprintf(stderr, "Option error. Allocation fails.\n");
      return params;
    }
    strcpy(params.address, argv[optind]);
    if(required_space_for_last_dot) {
      strcat(params.address,".");
    }
  }

  return params;
}

/* check if type of given ip address is IPv4 or not */
int isIPv4VersionAddress(char *ip_address) {
  char buffer[INET_ADDRSTRLEN];

  if(inet_pton(AF_INET, ip_address, buffer))
    return 1;

  return 0;
}

/* check if type of given ip address is IPv6 or not */
int isIPv6VersionAddress(char *ip_address) {
  char buffer[INET6_ADDRSTRLEN];

  if(inet_pton(AF_INET6, ip_address, buffer))
    return 1;

  return 0;
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

/* check if host exists in the internet */
int isHostValid(char* node) {
  struct addrinfo hints;
  struct addrinfo* results;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = 0;
  hints.ai_flags = AI_ADDRCONFIG;

  if(getaddrinfo(node, NULL, &hints, &results) != 0) {
    freeaddrinfo(results);
    return 1;
  }

  freeaddrinfo(results);
  return 0;
}

/* clean */
void cleanParams(TParams params) {
   free(params.server);
   free(params.address);
}
