# Projet Analyse Syntaxique
Alves Rayan - Kies Rémy
___

Création d'un analyseur lexical sur l'analyseur syntaxique donné, pouvant créer un arbre abstrait pour un sous langage du C.

- [Projet Analyse Syntaxique](#projet-analyse-syntaxique)
  - [Compilation](#compilation)
  - [Exécution](#exécution)
  - [Tests](#tests)
  - [Répartition du travail](#répartition-du-travail)


## Compilation

Le projet se compile via la commande `make`. L'exécutable `tpcas` se trouve dans le dossier `./bin/`. 

## Exécution

L'analyseur syntaxique s'exécute via `./bin/tpcas`. L'entrée par défaut est l'entrée standard. Un fichier peut être redirigé via les redirections de bash, ou en passant le nom d'un fichier en paramètre.

Les options d'exécution sont :

```
Usage: ./tpcas [OPTION...] FILE
Check if syntax of given file is valid, according to the grammar defined in parser.y

With no FILE, FILE is the standard input

  -t, --tree    print abstract tree of the given file
  -h, --help    display this help message and exit
```

## Tests

Le fichier `runtest.sh` permet de faire tourner l'analyseur syntaxique sur une batterie de tests. Après s'être assuré que l'exécutable existe bien, et que l'utilisateur possède les droits d'exécution du fichier (`chmod +x runtest.sh`), l'analyseur syntaxique peut être testé en exécutant `runtest.sh`.

**Attention** : Travaillant avec WSL et githubn et les espaces n'étants pas les mêmes entre Windows et Linux, il se peut que les retours à la ligne du script bash soient en CRLF et non en LF. Une rapide correction est d'ouvrir le fichier `runtests.sh` avec un éditeur et de le changer en sélectionnant tout le fichier.

Exemple de runtime

```
Starting tests on test/good
Test sur test/good/val_test1.tpc
Test sur test/good/val_test10.tpc
...

Test sur test/good/val_test8.tpc
Test sur test/good/val_test9.tpc

Starting tests on test/syn-err
Test sur test/syn-err/err_test1.tpc
Test sur test/syn-err/err_test10.tpc
Test sur test/syn-err/err_test11.tpc
...
Test sur test/syn-err/err_test8.tpc
Test sur test/syn-err/err_test9.tpc

Successfuls tests : 51/51
```

## Répartition du travail
La quasi-totalité du projet a été fait à deux à l'exception le parsing des arguments en ligne de commande a été réalisé par Rémy

___
Alves Rayan - Kies Rémy
