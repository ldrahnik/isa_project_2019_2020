DNS Server
============

Server přijímá zaslané DNS dotazy na localhostu na konkrétním portu a ukládá je to log souboru uvedeného z parametrů.

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

