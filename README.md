# FSH Sifaks Ilias Rayan

### Fonctionnalités principales

1. **Commandes de base**
    - Support de commandes intégrées comme `cd`, `pwd`, `clear`, `kill`, `ftype`, `for`, `if`, `history`
    - Exécution de commande externe commandes externes.
2. **Redirection d'entrée et de sortie**
    - Redirection des flux d'entrée et de sortie avec `>`, `>>`,`<`,`2>`,`<<`,`2>>`,`>|`,`2>|`
3. **Pipelines**
    - Exécution de commandes connectées par des pipelines (par ex. `command1 | command2`).
4. **Gestion des processus**
    - Commandes pour  gérer les processus en cours (`kill`).
5. **Scripts personnalisés (Demander)**
    - Support d'interprétation pour des boucles ou conditions simples (`for`, `if`).
6. **Gestion des erreurs** 
    - Gestion des erreurs via `perror` et affichage des messages directement dans le terminal via `write`.
7. **Complétion automatique**  
    - appuyer sur `tab` affiche les commande créer (voir 1. **Commandes de base)**
    - et si vous avez déjà taper une commande avant sur votre prompte sa affiche les fichier disponible dans le repertoire courant
8. **Non utilisation de fonction d’ordre sup**é**rieur**
    - Nous avons qu’utiliser des fonction n’utilisant que un appelle système, sauf dans certain cas comme l’affiche des erreur, la création de chaîne de caractère, ou la gestion du prompte de basse
9. **Commande non fini**  
    - Dans le dossier `commande non fini` il y a 7 commande non terminer, avec `compgen`,`fg`,`read`,`set`,`source`,`unset`, `jobs`
10. **Commande bonus**
    - Nous avons implémenté en plus les commande:
        - `forkbomb`: qui génère un fork bomb
        - `echos`; (pour echo scintillant) qui affiche le message donner avec des étoiles

---

## Fonctionnement des Commandes

- **`cd <path>`** : Change le répertoire courant (pour aller dans le répertoire `nom composé` alors vous devez taper `cd nom\ composé`)
- **`pwd`** : Affiche le répertoire courant.
- **`clear`** : Nettoie le terminal.
- **`history`** : Affiche l'historique des commandes lancé
- **`exit <int>`** : quitte le terminal avec la valeur de retour `0` ou `<int>` (ou ctrl + d).
- **`kill <pid> <signal>`** : envoie au processus `<pid>` le signal `<signal>`
- **`ftype <ref>`** : affiche le type du fichier de référence `<ref>`
- **`for F in <rep> [-A] [-r] [-e EXT] [-t TYPE] [-p MAX] { CMD }`** : Sans option, exécute la commande structurée `CMD` pour chaque (référence de) fichier non caché du répertoire `<rep>`. `CMD` peut dépendre de la valeur de la variable de boucle `F`, désignée par `$F`.
    
    Effet des options :
    
    - `A` : étend la portée de la boucle aux fichiers cachés (sauf `.` et
    `..`);
    - `r` : parcours récursif de toute l'arborescence de racine `REP`;
    - `e EXT` : filtrage des résultats selon l'extension : les fichiers dont
    le nom de base ne se termine pas par `.EXT` sont ignorés; pour les
    autres fichiers, la variable de boucle `F` prend pour valeur la
    référence du fichier *amputée de son extension.*
    - `t TYPE` : filtrage des résultats selon le type de fichier : `f` pour
    les fichiers ordinaires, `d` pour les répertoires, `l` pour les liens
    symboliques, `p` pour les tubes;
    - `p MAX` : permet le traitement en parallèle d'un maximum de `MAX`
    tours de boucle.
- **`if TEST { CMD_1 } else { CMD_2 }`**  : exécute le pipeline `TEST` puis, selon sa valeur de retour, exécute l'une ou l'autre des commandes structurées entre accolades. (`else` n’est pas obligatoire)
- **`echos <msg>`**   : Affiche `<msg>` avec des étoiles
- **`forkbomb`**  : Affiche l'historique des commandes lancé
- **`history`** : Affiche l'historique des commandes lancé

---

## Structure des fichiers

### Liste des fichiers sources

- **`forkbomb.c`** : code pour générer un fork bomb (ne pas exécuter sans précautions).
- **`echos.c`** : Implémentation de la commande `echo`.
- **`if.c`** : Support pour exécuter des conditions dans le terminal.
- **`cd.c`** : Implémentation de la commande `cd`.
- **`clear.c`** : Implémentation de la commande `clear`.
- **`ftype.c`** : Détecte et affiche le type d'un fichier donné.
- **`executable.c`** : Gère l'exécution de commandes externes.
- **`main.c`** : Point d'entrée principal pour l'exécution du terminal.
- **`pipeline.c`** : Gère les pipelines entre plusieurs commandes.
- **`redirection.c`** : Implémente les redirections d'entrée et de sortie.
- **`pwd.c`** : Implémente la commande `pwd`.
- **`kill.c`** : Implémente la commande `kill`.
- **`for.c`** : Permet des boucles simples au sein du terminal.
- **`Makefile`** : Automatisation de la compilation et de l'exécution du projet.
- **`AUTHORS.md`** : Liste des contributeurs au projet.

---

### Compilation et Exécution

1. **Prérequis**
    - Un compilateur C, comme `gcc`.
    - Un environnement Linux/Unix.
2. **Cloner**
Utilisez la commande suivante pour cloner le projet :
    
    ```bash
    git clone https://moule.informatique.univ-paris-diderot.fr/poulalho/sy5-2024-2025.git
    ```
    
3. **Déplacez vous** 
    
    ```bash
    cd terminal
    ```
    
4. **Compilation**
Utilisez la commande suivante pour compiler le projet :
    
    ```bash
    make all
    ```
    
5. **Exécution**
Lancez le terminal avec :
    
    ```bash
    ./fsh
    ```
    

## Collaboration et Auteurs

Ce projet a été réalisé par les auteurs suivants, étudiants en informatique :

- **Belhassen Rayan** - 22215583 - @belhasse
- **Bencheikh Ilias** - 22202353 - @bencheik
- **Lounici Sifaks** - 22214651 - @lounici

Le fichier `AUTHORS.md` contient une description détaillée des contributions.

---

## Développement futur

1. Ajout/terminer de nouvelles commandes.
2. Optimisation des performances.
3. Implémentation des Alias 

---