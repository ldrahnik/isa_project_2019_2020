/**
 * Name: Drahník Lukáš
 * Project: ISA: DNS resolver
 * Date: 30.10.2019
 * Email: <xdrahn00@stud.fit.vutbr.cz>
 * File: error.h
 */

#ifndef _error_H_
#define _error_H_

enum ecodes {
  EOK = 0, // ok
  EOPT = 1, // invalid option (unknown option, unknown option character, required value when is option used is missing)
  EALLOC = 2, // allocation problem
  ESOCKET = 3, // socket problem
  ESENDTO = 4, // sendto() function problem
  ERECEIVEFROM = 5, // receivefrom() function problem
  EMALFORMEDPACKET = 6, // returned packet is not correctly received
  ESETSOCKETOPT = 7, // setsockopt()
};

#endif
