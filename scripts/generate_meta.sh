#!/bin/sh

echo "Generating metadata to meta.h"

output_folder=$1
githash=$(git rev-parse --short HEAD)

echo "GIT Hash: ${githash}"

gitstr="#define META_GITHASH (\"${githash}\")"
datestamp="#define META_DATESTAMP (__DATE__)"
timestamp="#define META_TIMESTAMP (__TIME__)"
echo "#ifndef META_H_" > "$output_folder"meta.h
echo "#define META_H_" >> "$output_folder"meta.h
echo "#include <stdint.h>" >> "$output_folder"meta.h
echo "" >> "$output_folder"meta.h
echo "$gitstr" >> "$output_folder"meta.h
echo "$timestamp" >> "$output_folder"meta.h
echo "$datestamp" >> "$output_folder"meta.h
echo "" >> "$output_folder"meta.h
echo "#endif /* META_H_ */" >> "$output_folder"meta.h
echo "" >> "$output_folder"meta.h
