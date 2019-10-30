/**
 * Name: Drahník Lukáš
 * Project: ISA: DNS resolver
 * Date: 30.10.2019
 * Email: <xdrahn00@stud.fit.vutbr.cz>
 * File: params.c
 */

#include "params.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h> // isprint

/**
 * Get TParams structure from terminal options
 *
 * @return TParams
 */
TParams getParams(int argc, char *argv[]) {

  // params
  TParams params = {
    .show_help_message = 0,
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
  while ((c = getopt(argc, argv, "hrx6s:p:")) != -1) {
    switch (c) {
      case 'h':
        params.show_help_message = 1;
 	    return params;
      case 'r':
        params.recursion_desired = 1;
        break;
      case 'x':
        params.reverse_lookup = 1;
        break;
      case '6':
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

  // for example catch this case: ./hpac "A B" -s f ggg
  if(argv[optind + 1] != NULL) {
    fprintf(stderr, "Option error. Please use optional options before non-options.\n");
    params.ecode = EOPT;
    return params;
  }

  params.address = malloc(strlen(argv[optind]) + 1);
  if(params.address == NULL) {
    params.ecode = EALLOC;
	fprintf(stderr, "Option error. Allocation fails.\n");
    return params;
  }
  strcpy(params.address, argv[optind]);

  return params;
}

void cleanParams(TParams params) {
   free(params.server);
   free(params.address);
}
