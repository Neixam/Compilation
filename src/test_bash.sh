#!/bin/bash
vert='\033[0;32m'
rouge='\033[0;31m'
violet='\033[0;35m'
neutre='\033[0m'
res='output/resultat.txt'
rm -rf $res

mkdir -p output output/good output/warn output/sem-err output/syn-err
echo "good tests :" >> $res
let "reussi = 0"
let "echec = 0"
let "nombre = 0"
for i in `ls test/good | grep -e "tpc"`
do
let "nombre = nombre + 1"
echo -e "${violet}$i${neutre}" >> $res
retour=`./bin/tpcc test/good/$i -o output/good/$i.asm 2>&1`
let "valret = $?"
echo " valeur de retour :    $valret" >> $res
echo " retour :" >> $res
echo -e "${rouge}$retour${neutre}" >> $res
if [ $valret == 0 ]
then
	let "reussi = reussi + 1"
else
	let "echec = echec + 1"
fi
done

echo -e "Nombre de tests correctes réussis ${vert}BONNE NOUVELLE${neutre}:" >> $res
printf %.2f%% "$((10000 * $reussi / $nombre))e-2"  >> $res
echo " des $nombre tests" >> $res
echo -e "Nombre de tests correctes échoues ${rouge}MAUVAISE NOUVELLE${neutre}:" >> $res
printf %.2f%% "$((10000 * $echec / $nombre))e-2"  >> $res
echo " des $nombre tests" >> $res
echo \ >> $res
echo "-------------------" >> $res
echo \ >> $res

let "reussi = 0"
let "echec = 0"
let "nombre = 0"
echo "syntaxique error :" >> $res
for i in `ls test/syn-err | grep -e "tpc"`
do
let "nombre = nombre + 1"
echo -e "${violet}$i${neutre}" >> $res
retour=`./bin/tpcc test/syn-err/$i -o output/syn-err/$i.asm 2>&1`
let "valret = $?"
echo " valeur de retour :    $valret" >> $res
echo " retour :" >> $res
echo -e "${rouge}$retour${neutre}" >> $res
if [ $valret == 1 ]
then
	let "reussi = reussi + 1"
else
	let "echec = echec + 1"
fi
done


echo -e "Nombre de tests incorrectes échoues ${vert}BONNE NOUVELLE${neutre}:" >> $res
printf %.2f%% "$((10000 * $reussi / $nombre))e-2" >> $res
echo " des $nombre tests" >> $res
echo -e "Nombre de tests incorrectes réussis ${rouge}MAUVAISE NOUVELLE${neutre}:" >> $res
printf %.2f%% "$((10000 * $echec / $nombre))e-2" >> $res
echo " des $nombre tests" >> $res
echo \ >>$res
echo "-------------------" >> $res
echo \ >>$res

let "reussi = 0"
let "echec = 0"
let "nombre = 0"
echo "warning :" >> $res
for i in `ls test/warn | grep -e "tpc"`
do
let "nombre = nombre + 1"
echo -e "${violet}$i${neutre}" >> $res
retour=`./bin/tpcc test/warn/$i -o output/warn/$i.asm 2>&1`
let "valret = $?"
echo " valeur de retour :    $valret" >> $res
echo " retour :" >> $res
echo -e "${rouge}$retour${neutre}" >> $res
if [ $valret == 0 ]
then
	let "reussi = reussi + 1"
else
	let "echec = echec + 1"
fi
done


echo -e "Nombre de tests incorrectes échoues ${vert}BONNE NOUVELLE${neutre}:" >> $res
printf %.2f%% "$((10000 * $reussi / $nombre))e-2" >> $res
echo " des $nombre tests" >> $res
echo -e "Nombre de tests incorrectes réussis ${rouge}MAUVAISE NOUVELLE${neutre}:" >> $res
printf %.2f%% "$((10000 * $echec / $nombre))e-2" >> $res
echo " des $nombre tests" >> $res

echo \ >>$res
echo "-------------------" >> $res
echo \ >>$res

let "reussi = 0"
let "echec = 0"
let "nombre = 0"
echo "semantic error :" >> $res
for i in `ls test/sem-err | grep -e "tpc"`
do
let "nombre = nombre + 1"
echo -e "${violet}$i${neutre}" >> $res
retour=`./bin/tpcc test/sem-err/$i -o output/sem-err/$i.asm 2>&1`
let "valret = $?"
echo " valeur de retour :    $valret" >> $res
echo " retour :" >> $res
echo -e "${rouge}$retour${neutre}" >> $res
if [ $valret == 2 ]
then
	let "reussi = reussi + 1"
else
	let "echec = echec + 1"
fi
done

echo -e "Nombre de tests incorrectes échoues ${vert}BONNE NOUVELLE${neutre}:" >> $res
printf %.2f%% "$((10000 * $reussi / $nombre))e-2" >> $res
echo " des $nombre tests" >> $res
echo -e "Nombre de tests incorrectes réussis ${rouge}MAUVAISE NOUVELLE${neutre}:" >> $res
printf %.2f%% "$((10000 * $echec / $nombre))e-2" >> $res
echo " des $nombre tests" >> $res
