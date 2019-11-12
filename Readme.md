TODO dokumentace:

1) otestovat pomocí vagrantu a do dokumentace to uvést
2) napsat do dokumentace do části upřesnění zadání, že parametr server jsem dal jako vyžadovaný
3) napsat do dokumentace do části upřesnění zadání přidání debug parametru
4) napsat do dokumentace do části upřesnění zadání, že kombinace -x a -6 není povolená
5) napsat do dokumentace do části upřesnění zadání, že při zobrazování počítám pouze s class INTERNET
6) timeout pro přijetí odpovědi je vyřešen 5 sekundami
7) návratové kódy, pokud aplikace položí dotaz a vypíše úspěšně všechny výsledky i prázdný seznam výsledků, vrací návratový kód 0

EXTENSIONS:

1) udělat rpm balíček, případně tedy man stránku a uvést v dokumentaci

TODO:

1) chyba, vypisuji TTL 0 místo TTL 14400 u dotazu a všude jinde: ./dns -r -s kazi.fit.vutbr.cz www.fit.vut.cz

Co je OK:

1) sudo valgrind --track-origins=yes --leak-check=full ./dns -s 147.229.8.12 www.fit.vutbr.cz

Question section (1):
  www.fit.vutbr.cz, A, IN
Answer section (1):
  www.fit.vutbr.cz, A, IN, 0, 147.229.9.23
Authority section (4):
  fit.vutbr.cz, NS, IN, gate.feec.vutbr.cz
  fit.vutbr.cz, NS, IN, guta.fit.vutbr.cz
  fit.vutbr.cz, NS, IN, kazi.fit.vutbr.cz
  fit.vutbr.cz, NS, IN, rhino.cis.vutbr.cz
Additional section (3):
  guta.fit.vutbr.cz, A, IN, 0, 147.229.9.11
  kazi.fit.vutbr.cz, A, IN, 0, 147.229.8.12
  guta.fit.vutbr.cz, AAAA, IN, 0, 2001:067C:1220:0809:0000:0000:93E5:090B

2) sudo valgrind --track-origins=yes --leak-check=full ./dns -6 -s 147.229.8.12 www.fit.vutbr.cz

Question section (1):
  www.fit.vutbr.cz, AAAA, IN
Answer section (1):
  www.fit.vutbr.cz, AAAA, IN, 0, 2001:067C:1220:0809:0000:0000:93E5:0917
Authority section (4):
  fit.vutbr.cz, NS, IN, gate.feec.vutbr.cz
  fit.vutbr.cz, NS, IN, guta.fit.vutbr.cz
  fit.vutbr.cz, NS, IN, kazi.fit.vutbr.cz
  fit.vutbr.cz, NS, IN, rhino.cis.vutbr.cz
Additional section (3):
  guta.fit.vutbr.cz, A, IN, 0, 147.229.9.11
  kazi.fit.vutbr.cz, A, IN, 0, 147.229.8.12
  guta.fit.vutbr.cz, AAAA, IN, 0, 2001:067C:1220:0809:0000:0000:93E5:090B


3) sudo valgrind --track-origins=yes --leak-check=full ./dns -r -s 8.8.8.8 clients4.google.com
Authoritative: No, Recursive: Yes, Truncated: No

Question section (1):
  clients4.google.com, A, IN
Answer section (2):
  clients4.google.com, CNAME, IN, 0, clients.l.google.com
  clients.l.google.com, A, IN, 0, 216.58.201.110
Authority section (0):
Additional section (0):
