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

/* Index Code :
  - Fonction Swap
  - Fonction Auxiliaire a la lecture
  - Fonction Lecture
  - Fonction Affichage
  - Fonction Initialisation Struct Elf
  - Affichage Global
*/

/*########## Fonction Swap ##########*/

static int isBigEndian = 0;

int swap32(int val){
  if(isBigEndian) return __bswap_32(val);
  return val;
}

int swap16(int val){
  if(isBigEndian) return __bswap_16(val);
  return val;
}

void swap_header(Elf32_Ehdr *header){
  header->e_version = swap32(header->e_version);
  header->e_entry = swap32(header->e_entry);
  header->e_phoff = swap32(header->e_phoff);
  header->e_machine = swap16(header->e_machine);
  header->e_type = swap16(header->e_type);
  header->e_shoff = swap32(header->e_shoff);
  header->e_flags = swap32(header->e_flags);
  header->e_ehsize = swap16(header->e_ehsize);
  header->e_phentsize = swap16(header->e_phentsize);
  header->e_phnum = swap16(header->e_phnum);
  header->e_shentsize = swap16(header->e_shentsize);
  header->e_shnum = swap16(header->e_shnum);
  header->e_shstrndx = swap16(header->e_shstrndx);
}

void swap_sections(Elf32_SHeaders secHeaders, Elf32_Ehdr *header){
  for (int ind = 0; ind < header->e_shnum; ind++) {
    secHeaders[ind].sh_name = swap32(secHeaders[ind].sh_name);
    secHeaders[ind].sh_type = swap32(secHeaders[ind].sh_type);
    secHeaders[ind].sh_flags = swap32(secHeaders[ind].sh_flags);
    secHeaders[ind].sh_addr = swap32(secHeaders[ind].sh_addr);
    secHeaders[ind].sh_offset = swap32(secHeaders[ind].sh_offset);
    secHeaders[ind].sh_size = swap32(secHeaders[ind].sh_size);
    secHeaders[ind].sh_link = swap32(secHeaders[ind].sh_link);
    secHeaders[ind].sh_info = swap32(secHeaders[ind].sh_info);
    secHeaders[ind].sh_addralign = swap32(secHeaders[ind].sh_addralign);
    secHeaders[ind].sh_entsize = swap32(secHeaders[ind].sh_entsize);
  }
}

void swap_symbolTable(Elf32_Sym *symbolTable, int nbSym){
  for(int j = 0; j < nbSym; j++){
    symbolTable[j].st_value = swap32(symbolTable[j].st_value);
    symbolTable[j].st_size = swap32(symbolTable[j].st_size);
    symbolTable[j].st_shndx = swap16(symbolTable[j].st_shndx);
    symbolTable[j].st_name = swap32(symbolTable[j].st_name);
  }
}

void swap_reloc_secs(Elf32_RelocSec *lRelocSec, int nbRelocSec){

  for(int a = 0; a < nbRelocSec; a++){
    for(int i = 0; i < lRelocSec[a].nb; i++){
      lRelocSec[a].rels[i].r_offset = swap32(lRelocSec[a].rels[i].r_offset);
      lRelocSec[a].rels[i].r_info = swap32(lRelocSec[a].rels[i].r_info);
    }
  }
}

/*########## Fonction Auxiliaire ##########*/

int is32_B_E(Elf32_Ehdr *header){
  if(header->e_ident[4] == 1 && header->e_ident[5] == 2 && memcmp(header->e_ident, ELFMAG, SELFMAG) == 0){
    isBigEndian = 1;
    return 0;
  }
  isBigEndian = 0;
  return 1;
}

int get_section_index_from_name(Elf *elf, char *secName){

  for(int i = 0; i < elf->header->e_shnum; i++){
    if(strcmp((const char*)elf->secHeaders[i].nameNotid, secName) == 0) return i;
  }
  return -1;
}

//Fonction pour recuperer les flags de chaque section
void get_flag(int flag, char *str_flag){
  int len = 0;
  
  //Si SHF_WRITE est contenu dans flag alors on l'ajoute
  if(flag & SHF_WRITE){
    str_flag[len]='W';
    len++;
  }
  if(flag & SHF_ALLOC){
    str_flag[len]='A';
    len++;
  }
  if(flag & SHF_EXECINSTR){
    str_flag[len]='X';
    len++;
  }
  if(flag & SHF_MERGE){
    str_flag[len]='M';
    len++;
  }
  if(flag & SHF_STRINGS){
    str_flag[len]='S';
    len++;
  }
  if(flag & SHF_INFO_LINK){
    str_flag[len]='I';
    len++;
  }
  if(flag & SHF_LINK_ORDER){
    str_flag[len]='L';
    len++;
  }
  if(flag & SHF_OS_NONCONFORMING){
    str_flag[len]='O';
    len++;
  }
  if(flag & SHF_GROUP){
    str_flag[len]='G';
    len++;
  }
  if(flag & SHF_TLS){
    str_flag[len]='T';
    len++;
  }
  if(flag & SHF_MASKOS){
    str_flag[len]='o';
    len++;
  }
  if(flag & SHF_MASKPROC){
    str_flag[len]='p';
    len++;
  }

  if(len==0){
    str_flag[0]='\0';
  }else{
    str_flag[len]='\0';
  }

}

/*########## Fonction Lecture ##########*/

void read_section_headers(Elf32_Ehdr *header, Elf32_SHeaders secHeaders, unsigned char *buffer){
  //Trouver la section SHT_SYMTAB pour avoir l'offset qui permet de trouver la table des strings
  Elf32_Shdr shstrtab_section;
  for (int i = 0; i < header->e_shnum; i++) {
    memcpy(&shstrtab_section, &buffer[header->e_shoff + i * header->e_shentsize], header->e_shentsize);
    if (shstrtab_section.sh_type == SHT_SYMTAB) {
      break;
    }
  }

  for(long int i = header->e_shoff; i < header->e_shoff+(header->e_shentsize*header->e_shnum); i = i + header->e_shentsize){
    long int j = (i-header->e_shoff)/header->e_shentsize;
    memcpy(&secHeaders[j], &buffer[i], header->e_shentsize); 
    secHeaders[j].nameNotid = &buffer[__bswap_32(shstrtab_section.sh_offset) + __bswap_32(secHeaders[j].sh_name)];
  }
}

Elf32_Sdumps read_elf_section_dump(Elf32_Ehdr *header, Elf32_SHeaders secHeaders, unsigned char *buffer){
  Elf32_Sdumps sectionDumps = malloc(header->e_shnum*sizeof(char *));
  for (int i = 0; i < header->e_shnum; i++) {
    unsigned char *sectionDump = malloc(secHeaders[i].sh_size);
    sectionDumps[i] = sectionDump;
    memcpy(sectionDump, buffer + secHeaders[i].sh_offset, secHeaders[i].sh_size);
  }

  return sectionDumps;
}

void read_elf_symbol_table(unsigned char *buffer, Elf *elf){
  int size = 0;

  //On cherche la table des symbole et de str
  for (int i = 0; i < elf->header->e_shnum; i++){
    //Si table des symboles
    if (elf->secHeaders[i].sh_type == SHT_SYMTAB){
      //Prendre le nombre d'entrees
      size = elf->secHeaders[i].sh_size / sizeof(Elf32_Sym);
      elf->symbolTab = malloc(size * sizeof(Elf32_Sym));
      //Copy de la table symboles
      memcpy(elf->symbolTab, &buffer[elf->secHeaders[i].sh_offset], size * sizeof(Elf32_Sym));
    }
    // Si table des str
    if (elf->secHeaders[i].sh_type == SHT_STRTAB){
      //adresse de la table str
      elf->strTab = &buffer[elf->secHeaders[i].sh_offset];
      break;
    }
  }
  elf->nbSym = size;
}

void read_elf_relocation_section(unsigned char *buffer, Elf *elf){

  elf->relocSecs = malloc(elf->header->e_shnum*sizeof(Elf32_RelocSec *));
  elf->nbRelocSec = 0;

  //On cherche les sections relocations
  for (int i = 0; i < elf->header->e_shnum; i++){
    //Si relocation
    if (elf->secHeaders[i].sh_type == SHT_REL){
      elf->relocSecs[elf->nbRelocSec].nb = elf->secHeaders[i].sh_size / sizeof(Elf32_Rel);
      elf->relocSecs[elf->nbRelocSec].rels = (Elf32_Rel *) elf->secDumps[i];
      elf->relocSecs[elf->nbRelocSec].offset = elf->secHeaders[i].sh_offset;
      elf->relocSecs[elf->nbRelocSec].iSection = i;
      elf->nbRelocSec++;
    }
  }
}

/*########## Fonction Affichage ##########*/

void print_elf_header(Elf32_Ehdr *header){

  //Liste des noms des machines, a completer si on a une machine differente
  char *nomMachine[100];
  for (int i = 0; i < 100; i++){
    nomMachine[i]="Inconnu";
  }
  nomMachine[62]="Advanced Micro Devices X86-64";
  nomMachine[40]="ARM";

  //Liste des noms de type de fichiers
  int typeFichier[9]={0,1,2,3,4,0xfe00,0xfeff,0xff00,0xffff};
  char *typeFichierNom[9];
  typeFichierNom[0]="No file type";
  typeFichierNom[1]="REL (Relocatable file)";
  typeFichierNom[2]="EXEC (Executable file)";
  typeFichierNom[3]="DYN (Position-Independent Executable file)";
  typeFichierNom[4]="CORE (Core file)";
  typeFichierNom[5]="LOOS (Operating system-specific)";
  typeFichierNom[6]="HIOS (Operating system-specific)";
  typeFichierNom[7]="LOPR (Processor-specific)";
  typeFichierNom[8]="HIPR (Processor-specific)";

  //Liste des OS ou extensions ELF specifique aux ABI
  char *nomOSABI[15];
  for (int i = 0; i < 15; i++){
    nomOSABI[i]="Inconnu";
  }
  nomOSABI[0]="UNIX - System V";

  //Liste des type de data
  char *nomData[3];
  nomData[0]="Invalid data encoding";
  nomData[1]="2's complement, little endian";
  nomData[2]="2's complement, big endian";

  //Liste des noms de classe
  char *nomClasse[3];
  nomClasse[0]="Invalid class";
  nomClasse[1]="ELF32";
  nomClasse[2]="ELF64";


  printf("ELF Header:\n");
  printf("  Magic:   ");
  for(int i = 0; i < 16; i++){
    if(header->e_ident[i] < 10) printf("0%x ", header->e_ident[i]);
    else printf("%x ", header->e_ident[i]);
  }
  printf("\n  Class:                             %s\n",nomClasse[header->e_ident[4]]);
  printf("  Data:                              %s\n",nomData[header->e_ident[5]]);
  printf("  Version:                           %x (current)\n",header->e_version);
  printf("  OS/ABI:                            %s\n",nomOSABI[header->e_ident[7]]);
  printf("  ABI Version:                       %d\n",header->e_ident[8]);
  for (int i = 0; i < 9; i++){
    if(header->e_type == typeFichier[i]) printf("  Type:                              %s\n",typeFichierNom[i]);
  }
  printf("  Machine:                           %s\n",nomMachine[header->e_machine]);
  printf("  Version:                           0x%d\n",header->e_version);
  printf("  Entry point address:               0x%d\n",header->e_entry);
  printf("  Start of program headers:          %d (bytes into file)\n",header->e_phoff);
  printf("  Start of section headers:          %d (bytes into file)\n",header->e_shoff);
  printf("  Flags:                             0x%x, Version5 EABI\n",header->e_flags);
  printf("  Size of this header:               %d (bytes)\n",header->e_ehsize);
  printf("  Size of program headers:           %d (bytes)\n",header->e_phentsize);
  printf("  Number of program headers:         %d\n",header->e_phnum);
  printf("  Size of section headers:           %d (bytes)\n",header->e_shentsize);
  printf("  Number of section headers:         %d\n",header->e_shnum);
  printf("  Section header string table index: %d\n",header->e_shstrndx);
  
}

void print_elf_section_header(Elf32_Ehdr *header, Elf32_SHeaders secHeaders){

  int typeSection[17]={0,1,2,3,4,5,6,7,8,9,10,11,0x70000000,0x7fffffff,0x80000000,0xffffffff,0x70000003};
  char *typeSectionNom[17];
  typeSectionNom[0]="NULL            ";
  typeSectionNom[1]="PROGBITS        ";
  typeSectionNom[2]="SYMTAB          ";
  typeSectionNom[3]="STRTAB          ";
  typeSectionNom[4]="RELA            ";
  typeSectionNom[5]="HASH            ";
  typeSectionNom[6]="DYNAMIC         ";
  typeSectionNom[7]="NOTE            ";
  typeSectionNom[8]="NOBITS          ";
  typeSectionNom[9]="REL             ";
  typeSectionNom[10]="SHLIB           ";
  typeSectionNom[11]="DYNSYM          ";
  typeSectionNom[12]="LOPROC          ";
  typeSectionNom[13]="HIPROC          ";
  typeSectionNom[14]="LOUSER          ";
  typeSectionNom[15]="HIUSER          ";
  typeSectionNom[16]="ARM_ATTRIBUTES  ";

  printf("There are %d section headers, starting at offset 0x%x:\n\nSection Headers:\n  [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al\n", header->e_shnum, header->e_shoff);
  for(int i = 0; i < header->e_shnum; i++){

    if (i < 10) printf("  [ %d] ", i); // indice
    else printf("  [%d] ", i);
    
    printf("%-16s  ", secHeaders[i].nameNotid);
    
    for (int j = 0; j < 17; j++){ // type
      if(secHeaders[i].sh_type == typeSection[j]) printf("%s",typeSectionNom[j]);
    }
    
    printf("%8.8x ", secHeaders[i].sh_addr); // Adresse
    printf("%6.6x ", secHeaders[i].sh_offset); // Offset
    printf("%6.6x ", secHeaders[i].sh_size); // Size
    printf("%2.2x ", secHeaders[i].sh_entsize); // EntSize
    char str_flag[10];
    get_flag(secHeaders[i].sh_flags,str_flag);
    printf("%3s ",str_flag); // Flags
    printf("%2d ", secHeaders[i].sh_link); // Link
    printf("%3d ", secHeaders[i].sh_info); // Info
    printf("%2d\n", secHeaders[i].sh_addralign); // Align

  }
  
  printf("Key to Flags:\n  W (write), A (alloc), X (execute), M (merge), S (strings), I (info),\n  L (link order), O (extra OS processing required), G (group), T (TLS),\n  C (compressed), x (unknown), o (OS specific), E (exclude),\n  D (mbind), y (purecode), p (processor specific)\n");
}

void print_elf_section_dump(Elf32_SHeaders secHeaders, Elf32_Sdumps dumps, int num){

  int flag = 0;
  if(secHeaders[num].sh_size != 0 && secHeaders[num].sh_type != 0x8){
    printf("\nHex dump of section '%s':\n", secHeaders[num].nameNotid);

    if(num==1) printf(" NOTE: This section has relocations against it, but these have NOT been applied to this dump.\n");

    char inAscii[17];
    inAscii[16]='\0';
    int affiche=0;
    unsigned char *section_data = dumps[num];
    int max=secHeaders[num].sh_size;
    for (int j = 0; j < max; j++){

      if(j%16==0) printf("  0x%08x ",j);

      printf("%02hhx",section_data[j]);
      inAscii[j%16]=(isprint(section_data[j]))?section_data[j]:'.';
      inAscii[(j%16)+1]='\0';
      affiche=0;

      if((j%4)==3){
        printf(" ");
        flag ++;
      }

      if (flag == 4){
        printf("%s\n",inAscii);
        affiche=1;
        flag = 0;
      }
    }
    int n=(16-(max%16))*2;
    n+=(n%8==0)?(n/8)-1:(n/8);
    if(affiche==0) printf("%*c %s\n",n,' ',inAscii);
    printf("\n");
  }
  else printf("Section '%s' has no data to dump.\n", secHeaders[num].nameNotid);
}

void print_elf_symbol_table(Elf32_Shdr_notELF *secHeaders, Elf32_Sym *symbolTab, unsigned char *strTab, int nbSym){
  
    char *typeSectionNom[19];
    typeSectionNom[0]="NOTYPE  ";
    typeSectionNom[1]="OBJECT  ";
    typeSectionNom[2]="FUNC    ";
    typeSectionNom[3]="SECTION ";
    typeSectionNom[4]="FILE    ";
    typeSectionNom[5]="COMMON  ";
    typeSectionNom[6]="TLS     ";
    typeSectionNom[7]="LOOS    ";
    typeSectionNom[8]="HIOS    ";
    typeSectionNom[9]="LOPROC  ";
    typeSectionNom[10]="LOPROC  ";
    typeSectionNom[11]="HIPROC  ";
    typeSectionNom[16]="NOTYPE  ";
    typeSectionNom[17]="OBJECT  ";
    typeSectionNom[18]="FUNC    ";

    char *bindType[7];
    bindType[0]="LOCAL  ";
    bindType[1]="GLOBAL ";
    bindType[2]="WEAK   ";
    bindType[3]="LOOS   ";
    bindType[4]="HIOS   ";
    bindType[5]="LOPROC ";
    bindType[6]="HIPROC ";

    char *visType[7];
    visType[0]="DEFAULT  ";
    visType[1]="INTERNAL ";
    visType[2]="HIDDEN   ";
    visType[3]="PROTECTED";
    visType[4]="EXPORTED ";
    visType[5]="SINGLETON";
    visType[6]="ELIMINATE";

    printf("\nSymbol table '.symtab' contains %d entries:\n   Num:    Value  Size Type    Bind   Vis      Ndx Name\n", nbSym);

    for (int j = 0; j < nbSym; j++) {
        printf("   %3d: ",j); // Num
        printf("%8.8x  ", symbolTab[j].st_value); // Val
        printf("%4d ", symbolTab[j].st_size); // Size
        printf("%s", typeSectionNom[symbolTab[j].st_info]); // Type
        printf("%s", bindType[symbolTab[j].st_info >> 4]); //Bind
        printf("%s", visType[symbolTab[j].st_other]); // Vis
        
        //Chaque type d'index
        if(symbolTab[j].st_shndx == 0) printf("UND ");
        else if(symbolTab[j].st_shndx == 0xfff1) printf("ABS ");
        else printf("  %i ", symbolTab[j].st_shndx);

        //Si type est section alors s sinon strtab
        if(symbolTab[j].st_info == 3) printf("%s\n", secHeaders[symbolTab[j].st_shndx].nameNotid); // Name
        else printf("%s\n", strTab + symbolTab[j].st_name);
    }
}

void print_elf_relocations_section(Elf32_Shdr_notELF *secHeaders, Elf32_Sym *symbolTab, unsigned char *strTab, Elf32_RelocSec *lRelocSec, int nbRelocSecs) {

  swap_reloc_secs(lRelocSec, nbRelocSecs);

  for(int a = 0; a < nbRelocSecs; a++){
    char numEnt[8];
    if(lRelocSec[a].nb == 1) strcpy(numEnt, "entry");
    else strcpy(numEnt, "entries");
    
    printf("\nRelocation section '%s' at offset 0x%x contains %d %s:\n Offset     Info    Type            Sym.Value  Sym. Name\n", secHeaders[lRelocSec[a].iSection].nameNotid, lRelocSec[a].offset, lRelocSec[a].nb, numEnt);
    for (int i = 0; i < lRelocSec[a].nb; i++){

      printf("%8.8x  ", lRelocSec[a].rels[i].r_offset);
      printf("%8.8x ", lRelocSec[a].rels[i].r_info);
      
      switch (ELF32_R_TYPE(lRelocSec[a].rels[i].r_info)){
      case R_ARM_CALL:
        printf("R_ARM_CALL       ");
        break;
      case R_ARM_ABS32:
        printf("R_ARM_ABS32      ");
        break;
      case R_ARM_V4BX:
        printf("R_ARM_V4BX       ");
        break;
      default:
        break;
      }

      int symInd = (ELF32_R_SYM(lRelocSec[a].rels[i].r_info));
      if(symInd == 0){
        printf("\n");
        continue;
      } 
      printf(" %8.8x   ",symbolTab[symInd].st_value);

      //Si type est section alors shstrtab sinon strtab
      if(symbolTab[symInd].st_name == 0) printf("%s\n", secHeaders[symbolTab[symInd].st_shndx].nameNotid); // Name
      else printf("%s\n", strTab + symbolTab[symInd].st_name);// Name
    }
  }
  swap_reloc_secs(lRelocSec, nbRelocSecs);
}

/*########## Fonction Initialisation Struct Elf ##########*/

Elf *read_elf(unsigned char *buffer){

  Elf *elf = malloc(sizeof(Elf));

  // Initialisation du Header
  Elf32_Ehdr *header = malloc(sizeof(Elf32_Ehdr));
  memcpy(header, buffer, 52);

  if(is32_B_E(header)){
    printf("ERR_ELF_FILE : Le fichier n'est pas un fichier ELF 32bits big endian\n");
    exit(EXIT_FAILURE);
  }

  swap_header(header);
  elf->header = header;

  // Table des headers des sections
  Elf32_SHeaders sectionsHeaders = malloc(header->e_shnum * sizeof(Elf32_Shdr_notELF));
  read_section_headers(header, sectionsHeaders, buffer);
  swap_sections(sectionsHeaders, header);
  elf->secHeaders = sectionsHeaders;

  // Table des symboles et des strings
  read_elf_symbol_table(buffer, elf);
  swap_symbolTable(elf->symbolTab, elf->nbSym);

  // Table des dumps des sections
  Elf32_Sdumps sectionsDumps = read_elf_section_dump(header, sectionsHeaders, buffer);
  elf->secDumps = sectionsDumps;

  // Table Relocation
  read_elf_relocation_section(buffer, elf);
  return elf;
}

/*########## Affichage Global ##########*/

void print_global_elf(Elf *elf, unsigned char *buffer){
  print_elf_header(elf->header);
  print_elf_section_header(elf->header, elf->secHeaders);
  for(int i = 0; i < elf->header->e_shnum; i++){
    print_elf_section_dump(elf->secHeaders, elf->secDumps, i);
  }
  print_elf_symbol_table(elf->secHeaders, elf->symbolTab, elf->strTab, elf->nbSym);
  print_elf_relocations_section(elf->secHeaders, elf->symbolTab, elf->strTab, elf->relocSecs, elf->nbRelocSec);
}