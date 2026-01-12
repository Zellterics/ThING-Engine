#!/bin/bash
set -e  # si algo falla, aborta el script

GLSLC=glslc
SHADERS_DIR=.

VERT="$SHADERS_DIR/basic.vert"
FRAG="$SHADERS_DIR/basic.frag"
VERT_OUT="$SHADERS_DIR/basicVert.spv"
FRAG_OUT="$SHADERS_DIR/basicFrag.spv"

echo "Compilando vertex shader..."
$GLSLC "$VERT" -o "$VERT_OUT"

echo "Compilando fragment shader..."
$GLSLC "$FRAG" -o "$FRAG_OUT"

# ===== POST =====
VERT="$SHADERS_DIR/post.vert"
FRAG="$SHADERS_DIR/post.frag"
VERT_OUT="$SHADERS_DIR/postVert.spv"
FRAG_OUT="$SHADERS_DIR/postFrag.spv"

echo "Compilando post vertex shader..."
$GLSLC "$VERT" -o "$VERT_OUT"

echo "Compilando post fragment shader..."
$GLSLC "$FRAG" -o "$FRAG_OUT"

# ===== JFA (COMPUTE) =====
COMP="$SHADERS_DIR/jfa.comp"
COMP_OUT="$SHADERS_DIR/jfaComp.spv"

echo "Compilando JFA compute shader..."
$GLSLC "$COMP" -o "$COMP_OUT"

echo
echo "✅ Compilación exitosa."
