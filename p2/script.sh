#!/bin/bash

# Ejemplo de salida:
# Instantes de simulacion no consumidos: 0
# Tiempo Consumido: 0.276606
# Nivel Final de Bateria: 2645
# Colisiones: 0
# Muertes: 0
# Objetivos encontrados: 4


imprimir () {
   if [ $# -ne 1 ]
   then
      printf "error\n"
      exit
   fi

   # guarda los datos
   instantes=$(echo $1  | cut -d' ' -f6)
   tiempo=$(echo $1     | cut -d' ' -f9)
   bateria=$(echo $1    | cut -d' ' -f14)
   colisiones=$(echo $1 | cut -d' ' -f16)
   muertes=$(echo $1    | cut -d' ' -f18)
   objetivos=$(echo $1  | cut -d' ' -f21)

   # convierte coma decimal del tiempo
   tiempo=$(echo $tiempo | tr . ,)
   #tiempo=23,12345

   # imprime el resultado
   # printf "(%d - %f) Muerto: %d\n" $objetivos $tiempo $muerto
   printf "%9d %09.5f %7d %10d %7d %9d\n" $instantes $tiempo $bateria $colisiones $muertes $objetivos
}

printf "Mapa    Instantes Tiempo    Bateria Colisiones Muertes Objetivos\n"

res=$(./BelkanSG mapas/mapa30.map   1 4  9 12 1  3  3 | tail -n6)
printf "mapa30  "
imprimir "$res"
res=$(./BelkanSG mapas/mapa50.map   1 4 40 33 1 28 25 | tail -n6)
printf "mapa50  "
imprimir "$res"
res=$(./BelkanSG mapas/mapa75.map   1 4 28 55 1  7 47 | tail -n6)
printf "mapa75  "
imprimir "$res"
res=$(./BelkanSG mapas/mapa100.map  1 4 23 38 2 77 45 | tail -n6)
printf "mapa100 "
imprimir "$res"
res=$(./BelkanSG mapas/isla.map     1 4 47 53 2 74 46 | tail -n6)
printf "isla    "
imprimir "$res"
res=$(./BelkanSG mapas/medieval.map 1 4 47 53 2 89 43 | tail -n6)
printf "medieval"
imprimir "$res"



exit

for i in $(seq 1 10)
do
   # ejecuta el mapa con las mismas condiciones de entrada salvo la seed
   res=$(./BelkanSG mapas/mapa30.map $i 4 9 12 1 3 3 | tail -n6)

   # guarda el tiempo y el número de objetivos
   tiempo=$(echo $res | cut -d' ' -f9 | tr . ,)
   objetivos=$(echo $res | cut -d' ' -f21)
   muerto=$(echo $res | cut -d' ' -f18)

   # imprime el resultado
   printf "(%d - %f) Muerto: %d\n" $objetivos $tiempo $muerto
done