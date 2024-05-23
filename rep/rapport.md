# Projet Compilation
Alves Rayan - Kies Rémy
___

Création d'un compilateur pour le langage TPC, un langage dérivé du C. Transcrit TPC en assembleur pour être compilé depuis l'assembleur généré.

- [Projet Compilation](#projet-compilation)
  - [Compilation](#compilation)
  - [Exécution](#exécution)
  - [Fonctions builtins](#fonctions-builtins)
  - [Valeurs de retour du compilateur](#valeurs-de-retour-du-compilateur)
  - [Structure d'un programme TPC](#structure-dun-programme-tpc)
    - [Variables](#variables)
    - [Arrays](#arrays)
  - [Structure de controle](#structure-de-controle)
  - [Gestion des données](#gestion-des-données)
  - [Tests](#tests)
  - [Difficultées rencontrées](#difficultées-rencontrées)
    - [Héritage du projet d'Analyse Syntaxique](#héritage-du-projet-danalyse-syntaxique)
    - [Les types en sémantique](#les-types-en-sémantique)
    - [Blocs d'instructions](#blocs-dinstructions)
    - [Arguments des fonctions](#arguments-des-fonctions)


## Compilation
Le projet se compile via la commande `make`. L'exécutable `tpcc` se trouve dans le dossier `./bin/`.

Il existe aussi les commandes `make clean` pour supprimer les fichiers `'.o'` et `make mrproper` pour nettoyer toute l'archive.

## Exécution
Le compilateur s'exécute via `./bin/tpcc`. L'entrée par défaut est l'entrée standard. Un fichier peut être redirigé via les redirections de bash, ou en passant le nom d'un fichier en paramètre.

Le compilateur crée un fichier nasm dont le nom est le nom du fichier d'entrée ou `_anonymous.asm` si l'entrée standard est utilisée.

Les options d'exécution sont :
```
Usage: ./tpcc [OPTION...] FILE
Check if syntax of given file is valid, according to the grammar defined in parser.y
Input is a .tpc file and output is the generated nasm

With no FILE, FILE is the standard input

  -t, --tree            print abstract tree of the given file
  -s, --symbols         print associated symbol tables
  -h, --help            display this help message and exit
```

## Fonctions builtins
Le compilateur fourni quatre fonctions builtin pour gérées les entrées/sorties en nasm. Ces fonctions ne **doivent pas** être redéfinies en TPC, et une erreur sémantique sera déclenchée sinon.

Les fonctions sont les suivantes:
* `char getchar(void)`: *lit un charactère sur stdin:* [getchar](../builtin/getchar.asm)
* `char getint(void)`: *lit un entier sur stdin:* [getint](../builtin/getint.asm)
* `void putchar(char)`: *affiche un charactère sur stdout* [putchar](../builtin/putchar.asm)
* `void putint(int)`: *affiche un entier sur stdout* [putint](../builtin/putint.asm)

Ses fonctions sont définies en nasm et se trouvent dans le dossier `builint/`.

## Valeurs de retour du compilateur
Lors de la traduction en NASM, plusieurs types d'erreurs peuvent se produire. Elles se traduisent par un message d'erreur ainsi qu'un code de retour :
* `0` si pas d'erreur
* `1` s'il y a une erreur syntaxique
* `2` pour une erreur sémantique
* `3` pour une erreur autre (paramètres en ligne de commande, allocation mémoire)
* `5` erreur d'entrée-sortie (exemple dans getint si l'on donne un caractère non numérique autre que le signe '-')

## Structure d'un programme TPC
Le langage TPC étant un sous langage du C, beaucoup de conventions et éléments de syntaxe sont les mêmes. C'est notamment le cas pour la déclaration des fonctions, les types, passage en paramètres, ...

```c
int fibo(int n) {
    if (n <= 1) {
        return n;
    } 
    return fibo(n - 1) + fibo(n - 2);
}
```
*Exemple avec la suite de Fibonacci*

### Variables
Le langage TPC étant un sous langage du C, il suit certaines conventions ANSI du C, c'est à dire que toutes les variables locales doivent être déclarées avant les instructions des fonctions, et les globales doivent être déclarées avant les fonctions.

```c
int a, b, c;
char d, e;

int array[10], f;
char chars[5];
```
*Exemple de déclaration de variables*

On ne peut déclarer une variable et lui assigner une valeur à la même instruction, ni utiliser les opérateurs sur place type `i++` ou `i += 1`. Ainsi, l'incrémentation d'une variable se fera obligatoirement par la syntaxe `i = i + 1`.

### Arrays
On doit toujours déclarer un tableau en indiquant sa taille (non nulle). Aucune expression n'est tolérée dans la déclaration de la taille du tableau (donc les tailles négatives ne sont pas autorisées) et une taille nulle est une erreur sémantique.

Lorsque les tableaux sont passsés en paramètres, leur taille ne doit pas être indiquées, et on donne l'adresse du tableau (le tableau n'est pas recopié).

```c
int add(int array[]) {
  return array[0] + array[1];
}

int main(void) {
  int array[2];

  array[0] = 1;
  array[1] = 2;
  return add(array);
}
```
*Exemple de déclaration de tableau et de passage en paramètre*

## Structure de controle
Les structures de contrôles sont:
* `if-else`
* `while`

La syntaxe est la même qu'en C. Les bloc d'instructions sans `{...}` sont supportés.

```c
int main(void) {
  int i;

  i = 0;
  while (i < 5) {
    putint(i);
    i = i + 1;
  }
  return 0;
}
```

Les conditions renseignées dans les structures de contrôles doivent être des `char`, des `int` ou des booléens (résultats de comparaisons ou opérations booléennes). Ainsi, le programme suivant déclenchera une erreur sémantique.
```c
int main(void) {
  int array[2];
  if (array) 
    return 1;
  return 0;
}
```
*Erreur sémantique*

## Gestion des données
Les deux types de données supportés sont les charactères `char` et les entiers `int`, ainsi que leur équivalents en array. Ces types sont codés sur 8 octets chacuns. **Toutes** les opérations sont castées en entier.

Il existe également `void` pour les fonctions ne retournant aucune valeur. On ne peut retourner un tableau d'entiers ou de charactères.

Les tableaux sont des adresses mémoires stockées sur 8 octets.

## Tests
Le fichier `runtest.sh` permet de faire tourner le compilateur sur une batterie de tests. Il peut être exécuté directement (`./runtest.sh`, attention aux droits d'exécution !) ou via la commande `make test` (qui donne les droits d'exécution sur le fichier).

Les différents jeux de tests sont les suivants:
* `test/good/` pour tester la validité de la syntaxe et de la sémantique
* `test/sem-err/` pour les différentes erreurs sémantiques
* `test/syn-err/` pour les erreurs de syntaxes
* `test/warn` pour les codes produisant des warnings (mais qui peuvent tout de même être compilés).

Ces tests ne s'assurent pas de l'exécution, mais uniquement de la syntaxe et de la sémantique, donc les tests dans `test/exec` sont uniquement fournis à titre indicatif.

**Attention** : Travaillant avec WSL et Github et les espaces n'étants pas les mêmes entre Windows et Linux, il se peut que les retours à la ligne du script bash soient en CRLF et non en LF. Une rapide correction est d'ouvrir le fichier `runtests.sh` avec un éditeur et de le changer en sélectionnant tout le fichier.

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

Starting tests on test
Test sur test/syn-err/1-empty-file.tpc
Test sur test/syn-err/10-array-w-no-size.tpc

Starting tests on test
Test sur test/warn/1-assignation-type.tpc
Test sur test/warn/2-cast-param.tpc
Test sur test/warn/3-function-decl-after-use.tpc
...

Successfuls tests : 113/113
```

## Difficultées rencontrées

### Héritage du projet d'Analyse Syntaxique
Suite au projet d'analyse syntaxique, beaucoup d'éléments ont changés. Tout d'abord, nous nous sommes aperçus que nous n'avions aucun label renseigné prorement sur les variables, fonctions, structures de contrôles, opérateurs et que les différents symboles étaient mal renseignés. C'est maintenant corrigé.

De plus, il y avait plusieurs labels en Bison qui n'étaient pas renseignés car nous n'en voyons pas l'utilité, mais ils sont essentiels en compilation. Par exemple, pour déterminer si une fonction à des paramètres ou non.

### Les types en sémantique
Au début de la vérification des types, on savait si une variable ou une expression était un entier, un charactère ou un tableau, mais impossible de savoir quel type de tableau. Pour pouvoir supporter ces informations supplémentaires, la représentation des types à changé en cours de projet pour utiliser des masques binaires.

```c
#define T_NONE      0       // 0 << 0 - 0000 0001 
#define T_INT       2       // 1 << 1 - 0000 0010 
#define T_CHAR      4       // 1 << 2 - 0000 0100 
#define T_VOID      8       // 1 << 3 - 0000 1000 
#define T_ARRAY    16       // 1 << 4 - 0001 0000 
#define T_FUNCTION 32       // 1 << 5 - 0010 0000 
```
*Extrait du fichier [types.h](../include/types.h)*

### Blocs d'instructions
Il nous a fallu un peu de temps avant de pouvoir gérer proprement ces différents blocs :

```c
if (...) {
  ...
} else {
  ...
}
```
et
```c
if (...)
  ...
else 
  ...
```
Pour les gérer, nous avons du "déporter" une partie de notre boucle qui itère sur les instructions afin de pouvoir l'appeler depuis d'autres fonctions.

### Arguments des fonctions
Il nous a fallu un peu de temps pour respecter complètement les conventions d'appels AMD 64, et elle a entrainé de grosses modifications dans le code, surtout pour la table des symboles pour les adresses. Deplus, il ne fallait pas oublier de supprimer de la pile les paramètres en trop (dans le cas où on a plus de 7 paramèters) lors du retour de fonction.
___
Alves Rayan - Kies Rémy
