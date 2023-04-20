#!/bin/bash
blanc="\e[0m"
rouge="\e[0;31m"
vert="\e[0;32m"
jaune='\e[0;33m'
rougeB="\e[48;5;1m"
vertB="\e[48;5;2m"
jauneB='\e[48;5;3m'


debug=1
nb_section=0

#Compilation
#gcc -g lecture.c -o lecture
#clear

function line_test(){
  fichier=$1
  read_type=$2
  test_num=$3
  err=0

  if [[ "$read_type" == "-x" ]]
  then
    sortieA=$(./main -l $read_type $fichier $test_num)
  else
    sortieA=$(./main -l $read_type $fichier)
  fi

  #Test si le fichier est un fichier ELF
  if [[ $sortieA =~ "ERR_ELF_FILE" ]] || ! [[ -f $fichier ]]
  then
    rt=-1
    return
  fi

  if [[ $read_type == "-x" ]]
  then
    sortieB=$(readelf $read_type $test_num $fichier)
  else
    sortieB=$(readelf $read_type $fichier)
  fi

  END=$(echo "$sortieB" | wc -l) 
  for ((i=1;i<=END;i++)); do
    
    ligneA=$(echo "$sortieA" | sed -n "$i p")
    ligneB=$(echo "$sortieB" | sed -n "$i p")

    if [ "$ligneA" != "$ligneB" ]
    then
      echo -e $rouge"$ligneA"$blanc"->$i"$'\n'$vert"$ligneB"$blanc"->$i"
      err=$(expr $err + 1)
    elif [[ $debug == 0 ]]
    then
      echo -e $ligneA
    fi

    #On recupere le nombre de sections.
    num=$(echo $ligneA | grep -oh 'Section header string table index: [0-9][0-9]*')
    num=$(echo $num | awk '{ print $NF }')
    if [[ $num != "" ]]
    then
      nb_section=$(($num))
    fi

  done

  rt=$err
}

function section_reloc_test(){
  if [[ $rt -eq -1 ]]
  then
    return
  fi

  fichier=$1
  echo -e $jaune"(Etape 5) : Test de la section de relocation de $(basename $fichier) :"$blanc
  line_test $fichier $2

  if [[ $rt -eq 0 ]]
  then
    echo -e $vertB"Test_reloc : $(basename $fichier) REUSSI."$blanc$'\n'
  elif [[ $rt -ne -1 ]]
  then
    echo -e $rougeB"Test_reloc : $rt erreurs trouvées dans $(basename $fichier), ECHEC"$blanc$'\n'
  fi
}

function symbol_table_test(){
  if [[ $rt -eq -1 ]]
  then
    return
  fi

  fichier=$1
  echo -e $jaune"(Etape 4) : Test de la table de symbole de $(basename $fichier) :"$blanc
  line_test $fichier $2

  if [[ $rt -eq 0 ]]
  then
    echo -e $vertB"Test_symb_tab : $(basename $fichier) REUSSI."$blanc
  elif [[ $rt -ne -1 ]]
  then
    echo -e $rougeB"Test_symb_tab : $rt erreurs trouvées dans $(basename $fichier), ECHEC"$blanc
  fi
}

function section_content_test(){
  if [[ $rt -eq -1 ]]
  then
    return
  fi

  fichier=$1
  echo -e $jaune"(Etape 3) : Test du contenu de section de $(basename $fichier) :"$blanc
  
  err_num=0
  for i in $(seq 1 $nb_section)
  do
    line_test $fichier $2 $i
    err_num=$(expr $rt + $err_num)
  done

  if [[ $err_num -eq 0 ]]
  then
    echo -e $vertB"Test_content_sec : $(basename $fichier) REUSSI."$blanc
  elif [[ $err_num -ne -1 ]]
  then
    echo -e $rougeB"Test_content_sec : $err_num erreurs trouvées dans $(basename $fichier), ECHEC"$blanc
  fi
}

function header_test(){
  fichier=$1
  line_test $fichier $2

  if [[ $rt -eq -1 ]]
  then
    echo -e $rouge"ERR_ELF_FILE : Le fichier $blanc$fichier$rouge n'est pas un fichier ELF 32bits big endian."$blanc$'\n'
  else
    echo -e $jaune"(Etape 1) : Test du header de $(basename $fichier) :"$blanc
  fi

  if [[ $rt -eq 0 ]]
  then
    echo -e $vertB"Test_header : $(basename $fichier) REUSSI."$blanc
  elif [[ $rt -ne -1 ]]
  then
    echo -e $rougeB"Test_header : $rt erreurs trouvées dans $(basename $fichier), ECHEC"$blanc
  fi
}

function section_header_test(){
  if [[ $rt -eq -1 ]]
  then
    return
  fi

  fichier=$1
  echo -e $jaune"(Etape 2) : Test du section header de $(basename $fichier) :"$blanc
  line_test $fichier $2

  if [[ $rt -eq 0 ]]
  then
    echo -e $vertB"Test_section_header : $(basename $fichier) REUSSI."$blanc
  elif [[ $rt -ne -1 ]]
  then
    echo -e $rougeB"Test_section_header : $rt erreurs trouvées dans $(basename $fichier), ECHEC"$blanc
  fi
}

for fichier in $@
do

  if [[ $fichier == "-d" ]]
  then
    debug=0
    continue
  fi

  if [[ -d $fichier ]]
  then
    for fich in $fichier/*.o
    do
      #Test de recuperation du header
      header_test $fich "-h"
      #Test de recuperation de la table des sections
      section_header_test $fich "-S"
      #Test d'affichage du contenu de sections
      section_content_test $fich "-x"
      #Test d'affichage de la table des symboles
      symbol_table_test $fich "-s"
      #Test d'affichage de la section de relocation
      section_reloc_test $fich "-r"
    done
  else
    #Test de recuperation du header
    header_test $fichier "-h"
    #Test de recuperation de la table des sections
    section_header_test $fichier "-S"
    #Test d'affichage du contenu de sections
    section_content_test $fichier "-x"
    #Test d'affichage de la table des symboles
    symbol_table_test $fichier "-s"
    #Test d'affichage de la section de relocation
    section_reloc_test $fichier "-r"
  fi

done