#!/usr/bin/env bash

xcrun -sdk macosx metal -c Shaders/Compute.metal -o Compute.air
xcrun -sdk macosx metal -c Shaders/Render.metal  -o Render.air
xcrun -sdk macosx metallib Compute.air Render.air -o Shaders.metallib
rm *.air

swift run