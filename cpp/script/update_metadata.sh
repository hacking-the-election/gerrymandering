baseName="/Users/svernooy/Desktop/kai/k-vernooy/science/hte/gerrymandering/cpp"
filesList=()

for i in $(ls $baseName/include); do
    filesList+=("$baseName/include/$i")
done

for i in $(ls $baseName/src); do
    filesList+=("$baseName/src/$i")
done

for i in $(ls $baseName/script); do
    if [ "$i" != "update_metadata.sh" ]; then
        filesList+=("$baseName/script/$i")
    fi
done

for i in ${filesList[@]}; do
    lineGrep=$(grep -n "last modified" $i )
    if [ $(echo "$lineGrep" | grep . | wc -l | tr -d ' ') -ne 1 ]; then
       echo "Wrong number of lines in lineGrep: $(echo "$lineGrep" | grep . | wc -l | tr -d ' ')"
    else
        # echo "$lineGrep"
	lineNum=$(echo $lineGrep | cut -d ':' -f 1)
        numSpaces=$(sed "${lineNum}q;d" $i | cut -d ':' -f 2- | awk -F'[^ ]' '{print length($1)}')
	fileDate=$(date -r $i '+%a, %b %d')
        currentDate=$(sed "${lineNum}q;d" $i | cut -d ':' -f 2- | awk '{$1=$1;print}')
	numSpaces=$((numSpaces + $(echo "$currentDate" | grep -o . | wc -l | tr -d ' ')))

	if [ "$fileDate" != "$currentDate" ]; then
	    numSpacesToBeAdded=$((numSpaces - $(echo "$fileDate" | grep -o . | wc -l | tr -d ' ')))
            newLine=" last modified:"
	    for j in $(seq $numSpacesToBeAdded); do
	        newLine+=" "
            done
	    newLine+=$fileDate

            touchStr=$(date -r $i '+%m%d%H%M')
	    perl -i -pe "s/.*/$newLine/ if $.==$lineNum" $i
	    touch -t "$touchStr" $i
        fi
    fi
done
