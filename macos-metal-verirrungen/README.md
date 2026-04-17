Ist unabhängig vom Stammverzeichnis und mein Erstversuch, das OpenGL4.3/C++-Projekt auf Metal/Swift zu migrieren. Schaunmermal ob das läuft. Ich hab hier nur so einen mac mini, der mir nichtmal gehört ;)

## Build 
Ginge vermutlich auch per xcode, aber das finde heraus, wer möchte...

Die Verzeichnisstruktur könnte auch schöner sein (siehe Package.swift "path"), aber das ist hier noch nicht der Punkt.

```
# Shader Artifact bauen:
xcrun -sdk macosx metal -c Shaders.metal -o Shaders.air
xcrun -sdk macosx metallib Shaders.air -o Sources/gol-showcase/Shaders.metallib
rm Shaders.air

# Swift 
swift run
```

