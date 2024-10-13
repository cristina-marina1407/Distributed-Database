**Postelnicu Cristina-Marina**
**313CA**

## Distributed Database TEMA 2 ##

### Descriere:

* Tema este alcatuita din functii ce au rolul de a dezvolta o bază de date
distribuită în care se păstrează documente. Programul foloseste functiile
de creare a unei liste dublu inlantuite, de adaugare a unui nod pe pozitia n,
de stergere a unui nod de pe pozitia n si de eliberare a memoriei folosite 
pentru o lista o lista dublu inlantuita. De asemenea, foloseste astfel de
functii pentru un hashtable si o coada.
* Functiile necesare pentru implementarea cache ului sunt:
    * init_lru_cache
        Aceasta functie are rolul de a crea un cache ce contine un hashtable
    si o lista de recente, folosind functiile de creare ale acestora.
    * lru_cache_is_full
        Functia verifica daca cache ul esre plin, verificand daca dimensiunea
    hashtable ului este mai mare decat dimensiunea maxima.
    * free_lru_cache
        Functia free_lru_cache elibereaza memoria alocata pentru cache, dand
    free campurilor acestuia.
    * lru_cache_put
        Functia lru_cache_put adaugă un element în cache. Dacă cheia există
    deja, elementul este mutat la sfârșitul listei de recente pentru a indica
    faptul că a fost recent accesat. Dacă cache-ul este plin, cel mai puțin
    recent utilizat element este eliminat pentru a face loc noului element.
    * lru_cache_get
        Această funcție returnează valoarea asociată cu o anumită cheie din
    cache. Dacă cheia este găsită, elementul este mutat la sfârșitul listei
    pentru a indica faptul că a fost recent accesat, precum se procedeaza
    in functia lru_cache_put.
    * lru_cache_remove
        Functia lru_cache_remove elimină un element din cache folosind cheia
    acestuia. Dacă cheia este găsită, elementul este eliminat atât din
    hashtable cât și din lista de recente. Memoria alocată pentru element
    este eliberată.
* Functiile necesare pentru implementarea unui server sunt:
    * server_edit_document
        Functia server_edit_document se ocupa de request urile de tip EDIT.
    În primul rând, verifică dacă documentul există deja în cache. Dacă da,
    actualizează conținutul documentului în cache și în baza de date a 
    serverului. Dacă documentul nu există în cache, dar există în baza de date,
    actualizează conținutul documentului în baza de date și îl adaugă în cache.
    Dacă documentul nu există nici în cache, nici în baza de date, adaugă
    documentul în ambele locații. În toate cazurile, funcția actualizează
    log-ul și răspunsul serverului.
    * server_get_document
        Functia server_get_document se ocupa de request urile de tip GET.
    În primul rând, verifică dacă documentul solicitat există în cache. Dacă
    da, returnează conținutul documentului și actualizează log-ul serverului.
    Dacă documentul nu există în cache, dar există în baza de date, adaugă
    documentul în cache și returnează conținutul acestuia. Dacă documentul
    nu există nici în cache, nici în baza de date, returnează un răspuns NULL
    și actualizează log-ul serverului.
    * init_server
        Functia init_server creeaza un server ce contine o coada de request uri
    database ul si cache ul, folosind functiile de creare ale acestora
    * server_handle_request
        Funcția server_handle_request gestionează cererile primite de la un
    server. Aceasta returnează un răspuns în funcție de tipul request ului.
    Dacă tipul request ului  este "EDIT", creează o copie a cererii, adaugă
    copia în coadă și actualizeaza campurile din response. Dacă tipul request
    ului este "GET", procesează toate cererile din coadă, execută operațiile
    corespunzătoare și printează response ul pentru fiecare operație. După ce a
    terminat de procesat toate request urile din coadă, apelează funcția
    server_get_document pentru a obține documentul solicitat și returnează
    răspunsul.
    * free_server
        Functia free_server elibereaza memoria alocata pentru un server, dand
    free campurilor acestuia.
