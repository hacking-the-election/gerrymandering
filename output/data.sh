#!/bin/bash


average() {
	mod=$(echo  "$1" | tr -d '[' | tr -d ']' | tr ',' '\n' | grep .)
	num=$(echo "$mod" | wc -l)
	printf $(echo "$(echo "$mod" | paste -sd+ - | bc -l) / $num" | bc -l)
}


for i in $(ls); do
   if [ "$i" != "data.sh" ]; then
	printf "$i;"
#	echo $(cat $i/stats/init_config_data.txt | tr '\t' '~' | cut -d '~' -f 1)
	average $(cat $i/stats/init_config_data.txt | tr '\t' '~' | cut -d '~' -f 1)
	printf ";"
	average $(cat $i/stats/init_config_data.txt | tr '\t' '~' | cut -d '~' -f 2)
	printf ";"
	average $(cat $i/stats/communities/data.tsv | tail -n 1 | tr '\t' "~" | cut -d '~' -f 1)
	printf ";"
	average $(cat $i/stats/communities/data.tsv | tail -n 1 | tr '\t' "~    " | cut -d '~' -f 2)
	echo ""
   fi
done
