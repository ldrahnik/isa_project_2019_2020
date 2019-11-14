DNS Resolver
============

Program zasílá DNS dotazy na DNS server od kterého očekává odpověd. V DNS dotazu program umožnuje požadovat reverzní dotaz typu `PTR` pomocí parametru `-x`, IPv6 dotaz typu `AAAA` pomocí parametru `-6` nebo bez uvedení zmíněných parametrů defaultní typ `A`. Dále lze nastavit jiný port než defaultní `53` pomocí parametru `-p` a lze požadovat rekurzivní typ dotazu.

## Příklad spuštění:

```
./dns -h
DNS resolver

Usage:

./dns [-r] [-x] [-6] -s server [-p port] hostname/IPv4/IPv6

Any order of options is acceptable but all of them have to be before non-option inputs. Options:
-r: Recursion is required (Recursion Desired = 1), otherwise no recursion
-x: Reverse request is required instead of directly request
-6: Use AAAA instead of default A
-s: IP address or domain name of server where is request sent
-p port: port number where is request sent, default 53
hostname/IPv4/IPv6: requested hostname (when is active -x valid IPv4/IPv6 address)
```
```
./dns -x -r -s 8.8.8.8 172.217.23.206
Authoritative: No, Recursive: Yes, Truncated: No

Question section (1):
  206.23.217.172.in-addr.arpa, PTR, IN
Answer section (2):
  206.23.217.172.in-addr.arpa, PTR, IN, 0, prg03s05-in-f206.1e100.net
  206.23.217.172.in-addr.arpa, PTR, IN, 0, prg03s05-in-f14.J
Authority section (0):
Additional section (0): `
```

## Omezení programu:

TTL chyba, program vypisuje TTL 0 místo TTL 14400 atp. u všech dotazů.

Chybějící automatické testy, nefungující `make test` příkaz.

## Rozšíření programu:

Vytvořená man stránka programu.

Vytvořený .spec soubor pro RPM balíček.

Vytvořený příkaz `make install`.

## Odevzdané soubory:

```
tar -C xdrahn00 -xvf xdrahn00.tar
tree xdrahn00
xdrahn00
├── dns.1
├── dns.spec
├── doc
│   └── manual.pdf
├── LICENSE
├── Makefile
├── Readme.md
├── src
│   ├── dns.c
│   ├── dns.h
│   ├── error.h
│   ├── params.c
│   └── params.h
└── tests
    └── tests.sh

2 directories, 11 files
```
