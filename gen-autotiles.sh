#!/usr/bin/bash

mkdir -p tiled-maps/autotiles
rm -f tiled-maps/autotiles/rules.txt

for tmap in data/tilesets/autotiles/*.png
do
    atile="${tmap##*/}"
    atile="${atile%.*}"
    python modified-remex/main.py expand -f -o tiled-maps/autotiles/$atile.expanded.png $tmap
    python modified-remex/main.py maketileset -f -o tiled-maps/autotiles/$atile.tsx -r tiled-maps/autotiles/$atile.expanded.png
    python modified-remex/main.py makerule -f -l map -o tiled-maps/autotiles/$atile.rule.tmx tiled-maps/autotiles/$atile.tsx
    echo $atile.rule.tmx >> tiled-maps/autotiles/rules.txt
done

echo autotiles/rules.txt > tiled-maps/rules.txt