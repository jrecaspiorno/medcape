#!/bin/bash
#----------------------------------------------------------
#
# Copyright 2009 Pedro Pablo Gomez-Martin,
#                Marco Antonio Gomez-Martin
# 
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#----------------------------------------------------------

# Actualizamos las im�genes que est�n en el directorio actual.
./update-eps.sh || exit 1

# Y ahora en los subdirectorios.
for DIR in *; do
	if [ -d $DIR ]; then
		if [ $DIR != "CVS" ]; then
			# S�lo si el subdirectorio no es el de
			# la informaci�n del CVS.
			cd $DIR
			if ! ../update-eps.sh; then
				cd ..
				exit 1
			fi
			cd ..
		fi
	fi
done

# Ten en cuenta que estamos entrando en los directorios
# de primer nivel. TeXiS (su Makefile) asume que no habr� 
# subdirectorios dentro de los directorios de imagenes de
# cada cap�tulo precisamente para que este script sea m�s
# simple :-)
# Si realmente se quisieran soportar subdirectorios en
# los directorios de los cap�tulos, habr�a que hacer algo
# como:
#
# for DIR in $(find . -type d); then ...
#
# Pero luego teniendo en cuenta de quitar los CVS y los
# .svn

# Este script tiene una estructura similar a updateAll.sh.
# A su vez, ambos (updateAll.sh y cleanAll.sh) est�n
# copiados iguales en el directorio de im�genes vectoriales.
# Si modificas alguno, tendr�s que modificar los dem�s.