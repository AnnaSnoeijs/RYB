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

echo Updating git repo
updatedifs "~/libpynq-5EWC0-2023-v0.2.6/applications/ ./"

git add --all .
git commit -m "Synced to $(hostname)"
git push

IFS=$OLDIFS
