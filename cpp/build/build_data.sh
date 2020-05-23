#!/bin/bash

data_size=47
states=("alabama" "alaska" "arizona" "arkansas" "california" "colorado" "connecticut" "delaware" "georgia" "hawaii" "hawaii" "idaho" "illinois" "indiana" "iowa" "kansas" "louisiana" "maine" "maryland" "massachusetts" "michigan" "minnesota" "mississippi" "missouri" "nebraska" "nevada" "new hampshire" "new jersey" "new mexico" "new york" "north carolina" "north dakota" "ohio" "oklahoma" "pennsylvania" "rhode island" "south carolina" "south dakota" "tennessee" "texas" "utah" "vermont" "virginia" "washington" "wisconsin" "wyoming");
has_data=false

printf "checking for data in ../data/raw... "

for i in "${states[@]}"; do
    i=$(echo "$i" | tr ' ' ',');
    has_data_2=false;
    for j in $(ls ../data/raw | tr ' ' ','); do
        if [ "$i" == "$j" ]; then
            has_data_2=true;
        fi
    done
    if [ "$has_data_2" == false ]; then
        has_data=false;
        i=$(echo "$i" | tr ',' ' ')
        echo "missing data for $i in ../data/raw"
        exit 1;
    fi
done

echo "all data present"
IFS=$'\n'

for i in "${states[@]}"; do
    if [[ "$(echo "${i}" | cut -c 1)" == [u-z] ]]; then
        echo "serializing ${i}..."
        build/serialize.sh "$i"
    fi
    # printf "\nbuilding $(echo "$i" | cut -d ' ' -f 2 | rev | cut -d '/' -f2 | rev)\n\n"
    # eval "$i"
done
