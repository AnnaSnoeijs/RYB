#!/bin/sh

OLDIFS=$IFS
IFS=$'
'

diffcommand="diff -rwB"

updatedifs() {
	for files in $(
			bash -c "$diffcommand $@" |
			grep -P "^\Q$diffcommand" |
			sed "s/$diffcommand //"
		)
		do bash -c "
			echo $files;
			diff -yd $files;
			cp -i $files"
		 done
}

cd ~/git/RYB/applications
echo Pulling git
git pull
updatedifs "~/git/RYB/applications/  ~/libpynq-5EWC0-2023-v0.2.6/applications/"

# recompile everything
rm ~/libpynq-5EWC0-2023-v0.2.6/applications/*/main.d
rm ~/libpynq-5EWC0-2023-v0.2.6/applications/*/main.o
for i in $(ls ~/libpynq-5EWC0-2023-v0.2.6/applications/); do
	cd ~/libpynq-5EWC0-2023-v0.2.6/applications/$i && make
done;
cd ~/git/RYB/applications

IFS=$OLDIFS
