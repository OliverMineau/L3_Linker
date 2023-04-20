#!/bin/bash
blanc="\e[0m"
rouge="\e[0;31m"
vert="\e[0;32m"
jaune='\e[0;33m'
rougeB="\e[48;5;1m"
vertB="\e[48;5;2m"
jauneB='\e[48;5;3m'
nl=$'\n'

function suivant(){
    echo -n $'\n'"Suivant (Entrer) "
    read
    clear
}

if [[ $1 == -h ]]
then
    ./main -h
    exit
fi

#Compilation
echo -e $jaune"Compilation : $nl"$blanc
#CFLAGS='' ./configure
make

suivant

#Affichage header
echo -e $jaune"Affichage du header : $nl"$blanc
./main -l -h tests/test1BON.o

suivant

#Affichage de la table des sections
echo -e $jaune"Affichage de la table des sections : $nl"$blanc
./main -l -S tests/test1BON.o

suivant

#Affichage du contenu d'un section
echo -e $jaune"Affichage du contenu d'une section : $nl"$blanc
./main -l -x tests/test1BON.o 1

suivant

#Affichage de la table des symboles
echo -e $jaune"Affichage de la table des symboles : $nl"$blanc
./main -l -s tests/test1BON.o

suivant

#Affichage des tables de reimplantation
echo -e $jaune"Affichage des tables de reimplantation : $nl"$blanc
./main -l -r tests/test1BON.o

suivant

echo -e $jaune"Tests : $nl"$blanc
./mainTest.sh ./tests/

echo -n "$nl Commande : "

read test
while [[ $test != q ]]
do
    if [[ $test == t ]]
    then
        ./test.sh ./tests/test1BON.o
    elif [[ $test == d ]]
    then
        ./test.sh -d ./tests/test1BON.o
    elif [[ $test == f ]]
    then 
        echo -e $jaune"Fusion de deux fichiers ELF : $nl"$blanc
        ./main -f ./tests/test1BON.o ./tests/test2BON.o ./tests/ResultTest1-2BON.o
    fi
    echo -n "$nl Commande : "
    read test
done