#!/bin/bash

tool=./embed-spirv

src=lib/glsl
spirv_dst=spir_v
cpp_dst=lib/src/embedded
vert=default.vert
frag=default.frag
ext=.spv
compiler=glslc

hash $compiler &> /dev/null || { echo "$compiler not found"; exit 1; }
[[ -d $src ]] || { echo "cannot locate '$src'"; exit 1; }

[[ $# > 0 ]] && { tool=$1; echo "Using tool: $tool"; }
[[ -f $tool ]] || { echo "cannot locate '$tool'"; exit 1; }

[[ -d $spirv_dst ]] || mkdir -p $spirv_dst

function fail() {
  rm -rf $spirv_dst
  exit 1
}

function compile() {
  $compiler $src/$1 -o $spirv_dst/$1$ext || fail
  echo "== compiled '$src/$1' =="
}

function embed() {
  $tool -n=le::embedded -f=spirv_$2 $spirv_dst/$1$ext > $cpp_dst/spirv_$2.cpp || fail
  clang-format -i $cpp_dst/spirv_$2.cpp || fail
  echo "== embedded '$1' into '$cpp_dst/spirv_$2.cpp' =="
}

compile $vert
compile $frag

embed $vert vert
embed $frag frag

rm -rf $spirv_dst

echo "== done =="

exit
