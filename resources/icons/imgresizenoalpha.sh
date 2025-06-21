#!/bin/sh
inputFilePath=$1
filename=$(basename -- "$inputFilePath")
extension="${filename##*.}"
filename="${filename%.*}"
sizes=${@:2}
for size in $sizes; do
        echo convert "$inputFilePath" -resize "$size!" -alpha off "$filename-$size.$extension"
        convert "$inputFilePath" -resize "$size!" -alpha off "$filename-$size.$extension"
done
