# (Wielka) interpretacja zadania


## Zadanie 1
Tworzymy biblioteke, ktora umozliwa:  
* Stworzenie glownej tablicy wskaznikow na bloki operacji edycjnych polecenia diff, jedna para plikow - jeden blok (jedno miejsce w glownej tablicy) []
* Zadanie sekwencji par plikow []
* Wywolanie diff a.txt b.txt do pliku tymczasowego (w tmp.txt jedna para naraz) []
* Na podstawie tmp.txt stworzyc blok operacji edycyjnych []
* Usuniecie z pamieci (z tablicy glownej) bloku o indeksie i []
* Usuniecie danej operacji z bloku o indeksie i []
* Makefile ma tworzyc biblioteke w dwoch wersjach: statycznej i dzielonej []

## Zadanie 2
Testowanie biblioteki:  
* Argumenty do wywolania programu - zadania do wykonania, najpierw tworzymy tablice glowna o zadanym rozmiarze []
* create_table size - tworzy tablice glowna o rozmiarze size
* copare_pairs file1A.txt:file1B.txt file2A.txt:file2B.txt ... - porownanie parami, zapisanie wynikow do blokow operacyjnych []
* remove_block i - usuwa i-ty blok []
* remove_operation block_i oper_j - usuwa z i-tego bloku j-ta operacje []
Zas na konsole i do pliku z raportem wypisujemy czas dzialania dla:  
* malo(1-5), srednio (undefined) i duzo (undefined) par o roznych stopniach podobienstwa - maly, sredni i duzy (all undefined) []
* Zapisywanie roznych rozmiarow blokow w pamieci []
* Usuwanie z pamiecie roznych rozmiarow blockow []
* Naprzemienne dodawanie i usuwanie zadanej liczby blokow []
* (Ostatnie 3 bardzo undefined)
* Mierzymy czas: rzeczywisty, uzytkwonika, systemowy []
* Rezultaty w raport2.txt []

## Zadanie 3  
* a) Makefile uruchamiajacy testy i polecenia kompilacji na 3 sposoby:  
    + biblioteki statyczne []
    + biblioteki dzielone (ladowane przy uruchomieniu programu) []
    + biblioteki ladowane dynamicznie (ladowane przez program) []
    + wyniki pomiarow w results3a.txt, z komentarzem []
* b) Rozszerzyc Makefile z 3a) aby dalo sie skompilowac plik na trzech roznych poziomach optymalizacji: -o0 ... -os:
    + ponownie przeprowadzic pomiary po optymalizacji []
    + wyniki w results3b.txt []
    
