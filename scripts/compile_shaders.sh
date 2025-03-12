#!/bin/bash

src=lib/glsl
dst=assets/shaders
compiler=glslc

hash $compiler &> /dev/null || { echo "$compiler not found"; exit 1; }
[[ -d $src ]] || { echo "cannot locate '$src'"; exit 1; }

[[ -d $dst ]] || mkdir -p $dst

glsl_files=$(ls $src | grep -e .vert -e .frag)
[[ "$glsl_files" != "" ]] || { echo "no GLSL sources in $src"; exit; }

for file in $glsl_files; do
  $compiler $src/$file -o $dst/$file$ext || exit 1
  echo "== compiled '$dst/$file' =="
done

exit
