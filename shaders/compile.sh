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

VERT="$SHADERS_DIR/circle.vert"
FRAG="$SHADERS_DIR/circle.frag"
VERT_OUT="$SHADERS_DIR/circleVert.spv"
FRAG_OUT="$SHADERS_DIR/circleFrag.spv"

echo "Compilando vertex shader..."
$GLSLC "$VERT" -o "$VERT_OUT"

echo "Compilando fragment shader..."
$GLSLC "$FRAG" -o "$FRAG_OUT"

VERT="$SHADERS_DIR/outline.vert"
FRAG="$SHADERS_DIR/outline.frag"
VERT_OUT="$SHADERS_DIR/outlineVert.spv"
FRAG_OUT="$SHADERS_DIR/outlineFrag.spv"

echo "Compilando vertex shader..."
$GLSLC "$VERT" -o "$VERT_OUT"

echo "Compilando fragment shader..."
$GLSLC "$FRAG" -o "$FRAG_OUT"

echo
echo "✅ Compilación exitosa."
