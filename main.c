#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <byteswap.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "lecture.h"
#include "fusion.h"

void help(){
    printf("Cas de lecture d'un seul fichier :\n\
    ./main <-l> <option> <nom_fichier>\n\
    Options : \n\
        -a : Affiche toutes les parties ci-dessous \n\
        -h : Affiche l'en-tete d'un fichier ELF (header) \n\
        -S : Affiche la table des sections d'un fichier ELF (section header)\n\
        -s : Affiche la table des symboles d'un fichier ELF (symbol table)\n\
        -r : Afficher les tables de reimplantation d'un fichier ELF pour machine ARM (relocation section)\n\
        -x : Affiche le contenu de l'une des sections d'un fichier ELF (section dump) \n\
        NOTE : pour cette option il est necessaire d'ajouter un 4eme argument, le numero de la section que l'on souhaite afficher\n\
        profil : ./main <-l> <-x> <nom_fichier> <numero de la section a afficher> \n\
        NOTE 2 : L'option -a affiche toutes les sections du fichier.\n\
        \n\
    Cas de fusion de fichiers ELF :\n\
    ./main <-f> <nom_fichier_1> <nom_fichier_2> <nom_fichier_resultat> \n");
}

int main(int argc, char *argv[]){

    if( argc < 2 ) {
        printf("Erreur d'arguments\n");
        return EXIT_FAILURE;
    }

    if(!strcmp(argv[1], "-h")) {
        help(); 
        return EXIT_SUCCESS;
    }

    if(!strcmp(argv[1], "-f")){

        if(argc < 5){
            printf("Erreur d'arguments\n");
            return EXIT_FAILURE;
        }
        //file1.o file2.o fileResult.o
        fusion(argv[2],argv[3],argv[4]);
    }
    else if(!strcmp(argv[1], "-l")) {
        //-l choix_lecture nom_fichier (option pour -x)

        if(argc < 4){
            printf("Erreur d'arguments\n");
            return EXIT_FAILURE;
        }

        FILE* file = fopen(argv[3], "rb");
        if(file){

            // Initialisation du Buffer
            struct stat fileInfo;
            stat(argv[3], &fileInfo);
            unsigned char buffer[fileInfo.st_size];
            fread(&buffer, fileInfo.st_size, 1, file);
            fclose(file);

            Elf *elf = read_elf(buffer);

            if (!strcmp(argv[2], "-a")) print_global_elf(elf, buffer);
            else if (!strcmp(argv[2], "-h")) print_elf_header(elf->header);
            else if (!strcmp(argv[2], "-S")) print_elf_section_header(elf->header, elf->secHeaders);
            else if (!strcmp(argv[2], "-x") && argc == 5){
                if( (elf->header->e_shnum > atoi(argv[4]) ) && ( atoi(argv[4]) >= 0 ) ) print_elf_section_dump(elf->secHeaders, elf->secDumps, atoi(argv[4]));
                else printf("L'argument < x > n'est pas compris en 0 et %d inclus\n", elf->header->e_shnum-1);
            }
            else if (!strcmp(argv[2], "-s")) print_elf_symbol_table(elf->secHeaders, elf->symbolTab, elf->strTab, elf->nbSym);
            else if (!strcmp(argv[2], "-r")) print_elf_relocations_section(elf->secHeaders, elf->symbolTab, elf->strTab, elf->relocSecs, elf->nbRelocSec); 
            else printf("Erreur nombre d'arguments\n");
            liberation_elf(elf);
        }
        else printf("ERR_ELF_FILE : Erreur lecture du fichier\n");
        
        return EXIT_SUCCESS;
    }
    else {
        printf("Erreur d'arguments\n");
        return EXIT_FAILURE;
    }
}