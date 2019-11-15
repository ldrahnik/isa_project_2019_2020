/**
 * Name: Drahník Lukáš
 * Project: ISA: DNS resolver
 * Date: 14.11.2019
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
    .ecode = 0,
    .port = 53,
    .exit_after_one_request = 0
  };

  // don't want getopt() writing to stderr
  opterr = 0;

  // getopt
  int c;
  while ((c = getopt(argc, argv, "hdep:")) != -1) {
    switch (c) {
      case 'h':
        if(params.show_help_message) {
	      fprintf(stderr, "Option -h is already used.\n");
          params.ecode = EXIT_FAILURE;
          return params;
	    }
        params.show_help_message = 1;
 	    return params;
      case 'd':
        if(params.debug) {
	      fprintf(stderr, "Option -d is already used.\n");
          params.ecode = EXIT_FAILURE;
          return params;
	    }
        params.debug = 1;
        break;
      case 'e':
        if(params.exit_after_one_request) {
	      fprintf(stderr, "Option -e is already used.\n");
          params.ecode = EXIT_FAILURE;
          return params;
	    }
        params.exit_after_one_request = 1;
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
        params.ecode = EXIT_FAILURE;
        return params;
      default:
        fprintf(stderr, "Option error. Options error.\n");
        params.ecode = EXIT_FAILURE;
        return params;
    }
  }

  // for example catch this case: ./server -p 200 foobar
  if(argv[optind] != NULL) {
    fprintf(stderr, "Option error. Please use optional options before non-options.\n");
    params.ecode = EXIT_FAILURE;
    return params;
  }

  return params;
}
