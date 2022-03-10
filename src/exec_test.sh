#!/bin/bash
vert='\033[0;32m'
rouge='\033[0;31m'
violet='\033[0;35m'
neutre='\033[0m'
gras='\e[1m'
res='output_exe/resultat.txt'
rm -rf $res

mkdir -p output_exe output_exe/good output_exe/warn
o='output_exe'
pag="$o/good/"
paw="$o/warn/"

echo -e "${vert}GOOD TEST :${neutre}" 1>> $res
for i in `ls output/good | grep -e "asm"`
do
    echo -e "" >> $res
    echo -e "${violet}assembling $i${neutre}" 1>> $res
    nasm -f elf64 -o $pag$i.o output/good/$i
    gcc -o $pag$i.exe $pag$i.o -no-pie
    echo -e "${vert}retour $i :${neutre}\n${rouge}output =>${neutre}" 1>> $res
    ret=`echo "55" | ./$pag$i.exe`
    rez="$?"
    echo -e "${gras}$ret${neutre}" >> $res
    echo -e "${vert}return value => ${gras}$rez${neutre}" 1>> $res
done

echo -e "${violet}WARNING TEST :${neutre}" 1>> $res
for i in `ls output/warn | grep -e "asm"`
do
    echo -e "" >> $res
    echo -e "${violet}assembling $i${neutre}" 1>> $res
    nasm -f elf64 -o $paw$i.o output/good/$i
    gcc -o $paw$i.exe $paw$i.o -no-pie
    echo -e "${vert}retour $i :\n${rouge}output =>${neutre}" 1>> $res
    ret=`echo 55 | ./$paw$i.exe`
    rez="$?"
    echo -e "${gras}$ret${neutre}" >> $res
    echo -e "${vert}return value => ${gras}$rez${neutre}" 1>> $res
done
