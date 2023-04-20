# Projet_Prog
PROG5 - Projet, année 2022-2023 Réalisation d’un éditeur de liens — Phase de Fusion

####### Utilisation #######

# Compilation # 

CFLAGS='' ./configure (uniquement lors de la première compilation)
make

# Utilisation du programme 

Affichage du mode d'emploi: 
./main <-h> 

Cas de lecture d'un seul fichier : 
./main <-l> <option> <nom_fichier>
    Options :
        "-a" : Affiche toutes les parties ci-dessous
        "-h" : Affiche l’en-tete d’un fichier ELF (header)
        "-S" : Affiche la table des sections d’un fichier ELF (section header)
        "-s" : Affiche la table des symboles d’un fichier ELF (symbol table)
        "-r" : Afficher les tables de reimplantation d’un fichier ELF pour machine ARM (relocation section)
        "-x" : Affiche le contenu de l’une des sections d’un fichier ELF (section dump)
        NOTE : pour cette option il est necessaire d'ajouter un 4eme argument, le numero de la section que l'on souhaite afficher 
        profil : ./main <-l> <-x> <nom_fichier> <numero de la section a afficher> 
        NOTE 2 : L'option "-a" affiche toutes les sections du fichier.

Cas de fusion de fichiers ELF : 
./main <-f> <nom_fichier_1> <nom_fichier_2> <nom_fichier_resultat>

# Programme de test

Description : 

Nous utilisons un test automatisé pour la lecture de fichier : 
    Nous vérifions les 5 étapes de la phase 1 en même temps, pour cela nous comparons les sorties de notre programme aux sorties 
    des commandes associées (voir section ATTENTION). 
    Pour chaque étape si les deux sorties sont les mêmes le test est réussi et l'étape est passée. 
    Affichage type : <Test_header : test5.o REUSSI.>, en vert. 

    Dans le cas où des différences dans les sorties sont trouvées : 
    une ligne en ROUGE est affichée, c'est la sortie de notre programme.
    En dessous de chaque ligne rouge,  La ligne attendu sera affiché en VERT. 
    Puis il est affiché le nombre d'erreurs trouvées dans cette partie.
    Affichage type : <Test_symb_tab : 2 erreurs trouvées dans test1.o, ECHEC>, en rouge.

    De plus à la fin des lignes vertes et rouges il y a une flèche suivie d'un nombre(->10) indiquant le numéro de la ligne 
    du résultat attendu et du résultat obtenu. Ceci permet de voir si il y a une ligne qui à été oubliée dans notre version. 

    Enfin il y a une option de debug qui va afficher toutes les lignes qui sont présentes dans les deux résultats en plus de
    l'affichage précédent. 


Utilisation du programmme de test  :


./test.sh <dossier>
    On va tester tous les fichiers.o du dossier.
./test.sh <fichier.o>
    On va tester le fichier.o.

./test.sh <-d> <fichier.o>
    On va tester le fichier.o avec l'option de debug.


./mainTest.sh <dossier>
    Affichage simplifié des tests (il est seulement indiqué si le test est passé ou non).


ATTENTION : Pour vérifier que notre programme fonctionne bien nous comparons nos sorties aux sorties de la commande 
readelf. Les sorties de notre programme sont en anglais ainsi, si l'on souhaite les comparer avec les sorties des commandes elles 
doivent êtres également en anglais et donc linux doit être configuré en anglais. 



# Descriptif de la structure 

               /   écriture du fichier dans un buffer en mémoire et écriture des infos du buffer dans une structure elf  
      lecture -
     /         \   affichage des infos sélectionnées  
Main 
     \         /   écriture des fichiers dans des buffer en mémoire et écriture info des buffer dans des structures elf  (appel à lecture)
      fusion   --  fusion des différentes sections, tables des symboles et nom de sections
               \   fonctionnalité manquantes ( étapes 8 et 9 )
    
    ### Main
    Appels aux fonctions de lecture ou de fusion selon les arguments donnés. 


    ### Lecture.c
    Récupère les informations du format ELF d'un fichier, 
    les stocke dans une structure et les affiche selon les options données en argument.
        - Fonction Swap little endian à big endian 
        - Fonction Auxiliaire a la lecture, qui permettent de vérifier que le fichier soit bien en big endian et qui permet 
            de récupérer les flags de chaque section
        - Fonction Lecture en mémoire vers struct Elf
        - Fonction Affichage struct Elf
        - Fonction Initialisation Struct Elf
        - Affichage Global qui appelle selon les arguments donnés les autres fonctions d'affichages (header, section) 

    ### Fusion.c 
        - Fonction de concaténation de sections : Concatène les sections lorsque c'est nécéssaire sinon, les copie dans le fichier résultat. 
        - add_symbol (et fonctions auxiliaires): 
        - fusion_symbol_tables :
        - fusion :
        - Affichage Global qui appelle selon les arguments donnés les autres fonctions d'affichages (header, section) 


    ## Main.c
        - Main : Permet d'appeler le code ci-dessus en fonction des arguments reçus.



# Autres  
Compilation d'un fichier en 32 bits big endian :
arm-none-eabi-gcc -mbig-endian -c <fichier>

Fusion de deux fichiers 32 bits big endian :
arm-none-eabi-ld -EB -r -o <fichier_destination> <fichier1.o> <fichier2.o>

###
Custom ELF Loader intended for pedagogic use
###