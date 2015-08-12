#!/bin/sh
# Just a quick test for glslopt (but didn't improve speed in my tests so far)

glslopt -2 -f android/assets/shaders/ES2LightmapModulate.fsh android/assets/shaders/ES2LightmapModulate.fsh 
glslopt -2 -f android/assets/shaders/ES2Solid.fsh android/assets/shaders/ES2Solid.fsh
glslopt -2 -f android/assets/shaders/ES2TransparentAddColor.fsh android/assets/shaders/ES2TransparentAddColor.fsh
glslopt -2 -f android/assets/shaders/ES2TransparentAlphaChannel.fsh android/assets/shaders/ES2TransparentAlphaChannel.fsh
glslopt -2 -f android/assets/shaders/ES2TransparentVertexAlpha.fsh android/assets/shaders/ES2TransparentVertexAlpha.fsh

glslopt -2 -v android/assets/shaders/ES2Solid2.vsh android/assets/shaders/ES2Solid2.vsh
glslopt -2 -v android/assets/shaders/ES2SolidTransparent.vsh android/assets/shaders/ES2SolidTransparent.vsh
glslopt -2 -v android/assets/shaders/ES2Solid.vsh android/assets/shaders/ES2Solid.vsh

