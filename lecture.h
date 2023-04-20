#pragma once

#include <elf.h>

#define STB_LOCAL 0
#define STB_GLOBAL 1

#define SHN_ABS 0xfff1
#define SHN_UNDEF 0

typedef struct {
  Elf32_Word sh_name;
  Elf32_Word sh_type;
  Elf32_Word sh_flags;
  Elf32_Addr sh_addr;
  Elf32_Off sh_offset;
  Elf32_Word sh_size;
  Elf32_Word sh_link;
  Elf32_Word sh_info;
  Elf32_Word sh_addralign;
  Elf32_Word sh_entsize;
  unsigned char *nameNotid;
} Elf32_Shdr_notELF;

typedef struct {
  Elf32_Rel *rels;
  int iSection;
  int nb;
  int offset;
} Elf32_RelocSec;

typedef unsigned char** Elf32_Sdumps;
typedef Elf32_Shdr_notELF* Elf32_SHeaders;

typedef struct {
  Elf32_Ehdr *header;
  Elf32_Sdumps secDumps;
  Elf32_SHeaders secHeaders;
  Elf32_Sym *symbolTab;
  unsigned char *strTab;
  int nbSym;
  Elf32_RelocSec *relocSecs;
  int nbRelocSec;
} Elf;

Elf *read_elf(unsigned char *buffer);

/*########## Fonction Swap ##########*/

int swap32(int val);

int swap16(int val);

void swap_header(Elf32_Ehdr *header);

void swap_sections(Elf32_SHeaders secHeaders, Elf32_Ehdr *header);

void swap_symbolTable(Elf32_Sym *symbolTable, int nbSym);

void swap_reloc_secs(Elf32_RelocSec *lRelocSec, int nbRelocSec);

/*########## Fonction Auxiliaire ##########*/

int is32_B_E(Elf32_Ehdr *header);

int get_section_index_from_name(Elf *elf, char *secName);

//Fonction pour recuperer les flags de chaque section
void get_flag(int flag, char *str_flag);

/*########## Fonction Lecture ##########*/

void read_section_headers(Elf32_Ehdr *header, Elf32_SHeaders secHeaders, unsigned char *buffer);

Elf32_Sdumps read_elf_section_dump(Elf32_Ehdr *header, Elf32_SHeaders secHeaders, unsigned char *buffer);

void read_elf_symbol_table(unsigned char *buffer, Elf *elf);

void read_elf_relocation_section(unsigned char *buffer, Elf *elf);

/*########## Fonction Affichage ##########*/

void print_elf_header(Elf32_Ehdr *header);

void print_elf_section_header(Elf32_Ehdr *header, Elf32_SHeaders secHeaders);

void print_elf_section_dump(Elf32_SHeaders secHeaders, Elf32_Sdumps dumps, int num);

void print_elf_symbol_table(Elf32_Shdr_notELF *secHeaders, Elf32_Sym *symbolTab, unsigned char *strTab, int nbSym);

void print_elf_relocations_section(Elf32_Shdr_notELF *secHeaders, Elf32_Sym *symbolTab, unsigned char *strTab, Elf32_RelocSec *lRelocSec, int nbRelocSecs);

/*########## Fonction Initialisation Struct Elf ##########*/

Elf *read_elf(unsigned char *buffer);

/*########## Affichage Global ##########*/

void print_global_elf(Elf *elf, unsigned char *buffer);