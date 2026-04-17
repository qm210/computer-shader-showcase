#!/usr/bin/env bash

# könnte auch eine einzige .metal-Datei sein, oder drei, ...
xcrun -sdk macosx metal -c Shaders/Compute.metal -o Compute.air
xcrun -sdk macosx metal -c Shaders/Render.metal  -o Render.air
xcrun -sdk macosx metallib Compute.air Render.air -o Shaders.metallib
rm *.air

swift run