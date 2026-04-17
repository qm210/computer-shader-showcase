Ist unabhängig vom Stammverzeichnis und mein Erstversuch, das OpenGL4.3/C++-Projekt auf Metal/Swift zu migrieren. Schaunmermal ob das läuft. Ich hab hier nur so einen mac mini, der mir nichtmal gehört ;)

## Build 
Ginge vermutlich auch per xcode, aber das finde heraus, wer möchte...

Die Verzeichnisstruktur könnte auch schöner sein (siehe Package.swift "path"), aber das ist hier noch nicht der Punkt.

```bash
xcrun -sdk macosx metal -c Shaders/Compute.metal -o Compute.air
xcrun -sdk macosx metal -c Shaders/Render.metal  -o Render.air
xcrun -sdk macosx metallib Compute.air Render.air -o Shaders.metallib
rm *.air

swift run
```

