#Copyright Armasu Octavian 315CAa

# Tema 2 - Load_balancer

Am implementat tema folosind un vector de structuri de tipul server_memory
in load_balancer si in fiecare server avem cate un hashtable in care se vor
gasi cheia si valoarea. Pe langa asta fiecare server are un id (identic
pentru el si replicile sale), numarul replicii si hash-ul server-ului.

De fiecare cand adaugam un server, realocam vectorul de servere, de aceea
la pe ultimele 3 pozitii vom avea valori ce pointeaza catre NULL. Astfel,
la toate operatiile mergem pana la numarul de servere - 3, pentru a le exclude
pe acelea de la final.

Pentru a adauga un un element pe un server, cautam primul server cu hash
mai mare sau egal decat hash-ul cheii elementului ce va fi adaugat.
Daca nu exista, atunci elementul va fi stocat pe serverul de pe prima pozitie.

Cand adaugam un server nou, trebuie sa cautam in elementele serverului imediat
urmator ca sa vedem daca trebuie sa redistribuim cheile. Astefel, am folosit
un vector de tip info cu dimensiune size(cate elemente sunt in hashtable)
si am redistribuit elementele folosind functia "rebalansare". La baza,
functia este aceeasi cu cea de store doar ca merge pana la numarul de servere,
in timp ce cea de store merge pana la numar de servere - 3.

In server.c am introdus implementarea de la lab a hashtable-ului. Am folosit
si o functie move_keys care copiaza cheiile din primul server si le pune
si in al doilea. Aceasta functie a fost utila la functia de server_remove.

Pentru majoritatea functiilor am adaugat si comentarii care clarifica
cum functioneaza fiecare.

