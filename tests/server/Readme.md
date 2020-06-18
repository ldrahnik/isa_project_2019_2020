DNS Server
============

Server na localhostu na uvedeném portu přijímá DNS dotazy a vypisuje jejich obsah na standartní výstup.

## Příklad spuštění:

```
./server -h
DNS server

Usage:

./server [-h] [-d] [-p port]

Any order of options is acceptable but all of them have to be before non-option inputs. Options:
-h: Show help message
-d: Enable debug mode
-e: Exit server after one request
-p port: Port number where is request sent, default 53
```

```
./server -e -p 53
```

