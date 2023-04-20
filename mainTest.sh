#!/bin/bash
blanc="\e[0m"
rouge="\e[0;31m"
vert="\e[0;32m"
jaune='\e[0;33m'
rougeB="\e[48;5;1m"
vertB="\e[48;5;2m"
jauneB='\e[48;5;3m'

erreur=0
nb=0
for fichier in $1/*.o
do
  nb=$(expr $nb + 1)
  message=$(./test.sh $fichier)

  if ([[ ! $message =~ *"erreur"* ]] && [[ ! $message =~ "ERR_ELF_FILE" ]] && [[ $fichier =~ "BON" ]]) || ([[ $message =~ "ERR_ELF_FILE" ]] && [[ $fichier =~ "MAUV" ]])
  then
    echo -e "Test $(basename $fichier) :"$vert" BON"$blanc
  else
    echo -e "Test $(basename $fichier) :"$rouge" ERREUR"$blanc
    erreur=$(expr 1 + $erreur)
  fi

done

echo -e "$(expr $nb - $erreur)/$nb"