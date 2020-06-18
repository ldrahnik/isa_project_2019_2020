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
    .timeout = 5,
    .ipv6 = 0,
    .server = NULL,
    .port = 53,
    .address = NULL,
  };

  // don't want getopt() writing to stderr
  opterr = 0;

  // getopt
  int c;
  while ((c = getopt(argc, argv, "hdrt:x6s:p:")) != -1) {
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
      case 't':
        params.timeout = atoi(optarg);
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
  if(!isHostValid(params.server)) {
    fprintf(stderr, "Option error. Given server is not valid.\n");
    params.ecode = EOPT;
    return params;
  }

  // for example catch this case: ./dns foo -s 127.0.0.1 bar
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

/* check if host exists in the internet */
int isHostValid(char* node) {
  struct addrinfo hints;
  struct addrinfo* results;

  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_protocol = 0;
  hints.ai_flags = AI_ADDRCONFIG;

  if(getaddrinfo(node, NULL, &hints, &results) == 0) {
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
