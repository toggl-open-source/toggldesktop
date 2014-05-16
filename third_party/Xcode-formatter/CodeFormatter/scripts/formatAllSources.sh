#!/bin/sh

# This script formats files in parameter given sources folder
# Usage : $ sh CodeFormatter/formatAllSources.sh /Main/Classes

if [ -n "$1" ]
	then
	
	# recover directory to format :
	pathToSourcesDirectory=`echo $(pwd)/$1`
	
	# go to current folder :
	scriptDirectory=$(dirname $0)
	cd $scriptDirectory
	
	# find sources files to format :
	echo ""
	echo "==> Getting files to format in directory " + $pathToSourcesDirectory
	mkdir -p temp
	find $pathToSourcesDirectory -name "*.[mh]" > temp/sources_to_uncrustify.txt
	
	# format files :
	echo ""
	echo "==> Format files"	
	/usr/local/bin/uncrustify -F temp/sources_to_uncrustify.txt -c "../uncrustify_objective_c.cfg" --no-backup 
	
	# remove temp files : 
	rm -rf temp/

else 
	echo "Error : You must specify a source folder as first parameter"	
	
fi
	
