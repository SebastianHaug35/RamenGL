# Task 03

## 3.0) Setup
Clonen Sie das aktuelle Rahmenprogramm aus GitLab.

**ACHTUNG: Die Updates im aktuellen Rahmenprogramm brechen
einige Annahmen aus task02. Clonen Sie das Rahmenprogramm deshalb
unbedingt neu, damit Sie aus task02 nichts kaputt machen.**

Nutzen Sie das Sourcefile `task03.cpp` um die Aufgaben zu implementieren.
Kompilieren Sie es zunächst.
Starten Sie das Programm mit dem `assets/` Pfad als Argument, also, zB:
```bash
./build/task03 assets/
```
auf UNIX. Und
```bash
build\Debug\task03.exe assets\
```
auf Windows. Sie können auch absolute Pfade angeben.
Sie sollten nun ein Koordinatensystem sehen, welches
alle drei Achsen rot eingefärbt hat.

## 3.1) Nutzen der Farbattribute.
Alle drei Achsen des Koordinatensystems werden im Moment in rot
gerendert. Erweitern Sie das Programm, sodass der Vertexshader
das Farbattribut der Vertices erhält. Erweitern Sie den Shadercode
ebenfalls, damit diese Farbattribute an den Fragmentshader weitergereicht
werden.

## 3.2) Zylinder, Quader, Kugeln
Bauen Sie sich Funktionen, die Ihnen **mindestens** die folgenden
Geometrien erzeugen: Zylinder, Kugeln, Quader. 
Die Funktionen sollen ausserdem einen Parameter zur Bestimmung
der Farbe entgegennehmen. Beispiel:
```bash
std::vector<Vertex> CreateCylinder(const Vec3f& color);
```
**Achtung**: Vergessen Sie nicht, sinnvolle Normalenvektoren
zu erzeugen. Sie dürfen Normalenvektoren erzeugen, die perfekt
senkrecht auf die Polygone stehen.

## 3.3) Rendern der Geometrie
Testen Sie Ihre geometrischen Körper, indem Sie diese rendern.
Das Hochladen der Daten auf die GPU und das Rendern erfolgt wie
schon in Tasks 02. 

## 3.4) Debugging der Normalenvektoren
Wie testen Sie, ob Ihre Normalenvektoren korrekt sind und auch korrekt
im Shader sind? Nutzen Sie mindestens zwei Möglichkeiten.
Dokumentieren Sie Ihr Vorgehen.

## 3.5) Beleuchtung
Im Moment ist die Szene noch sehr flach, da kein Beleuchtungsmodell
implementiert wurde. Erweitern Sie ihren Shadercode, sodass
die Szene durch eine Punktlichtquelle beleuchtet wird.
Recherchieren Sie hierzu das *Lambert cosine law*.
Gibt es bei der Transformation von Modellen hinsichtlich
der Normalenvektoren etwas zu beachten? Falls ja,
erleutern Sie was das Problem ist und leiten Sie
eine geeignete mathematische Lösung her.

## 3.6) Bewegung der Kamera
Erweitern Sie das Programm, sodass Sie die Kamera mit 
der Tastatur steuern können. Sie sollten in der Lage sein,
die Kamera entlang ihrer Achsen zu verschieben und um einen Winkel zu rotieren.
Nutzen Sie für die Tastaturabfrage das Gerüst in der Eventloop und
die Funktionalität von SDL3.  
Beispiel:
```c

while ( isRunning )
{
    SDL_Event e;
    while ( SDL_PollEvent(&e) )
    {
        ImGui_ImplSDL3_ProcessEvent(&e);
        pRamen->ProcessInputEvent(e);

        if ( e.type == SDL_EVENT_QUIT )
        {
            isRunning = false;
        }

        if ( e.type == SDL_EVENT_KEY_DOWN )
        {
            switch ( e.key.key )
            {
            case SDLK_ESCAPE:
            {
                isRunning = false;
            }
            break;

            /* Camera movement */
            case SDLK_UP:
            {
                /* TODO: Pitch camera */
            }
            break;
            case SDLK_DOWN:
            {
                /* TODO: Pitch camera */
            }
            break;

            /* TODO: also implement camera Yaw and Roll */

            default:
            {
            }
            }
        }
    }
    
    // ...
    // set uniforms, rendering, etc.
    // ...

}
```
Dokumentieren Sie welches Winkelvorzeichen die Kamera in welche Richtung
rotieren bzw. verschiebt. Erklären Sie warum die Kamera sich so verhält.  

**Hinweis**: Die Kameraklasse aus `rgl_camera.h` hält bereits einige
Methoden parat mit der Sie die Kamera im Raum bewegen und rotieren
können ;)

## 3.7) Matrizen-Stack
Bauen Sie eine animierte Szene mithilfe eines Matrizenstapels und
den von Ihnen erstellten primitiven Körpern. Natürlich dürfen Sie
weitere geometrische Körper erstellen, falls Sie diese für
Ihre Szene benötigen (beispielsweise einen Kegel).  
Implementieren Sie hierzu am besten eine Klasse, welche die 
Methoden aus der Vorlesung zur Verfügung stellt.

Beispiel
```c
MatrixStack matrixStack{};
matrixStack.Push();
for ( int i = 0; i < numModels; i++ )
{
    matrixStack.rotate(RAMEN_WORLD_FORWARD, angle);
    matrixStack.translate(Vec3f{ 1.0f, 0.0f, -0.5f });
    glUniformMatrix4fv(0, 1, GL_FALSE, matrixStack.Last().Data());
    glDrawArrays(GL_TRIANGLES, 0, model.NumVertices());
}
matrixStack.Pop();
```
Nutzen Sie intern die Methoden aus der Mathematik-Bibliothek `rgl_math.h`.

Nutzen Sie alle drei Transformationsarten (Rotate, Scale, Translate) und
bauen Sie eine Animation, die den Matrixstack mindestens auf eine Tiefe 
(bzw. Höhe) von **drei** Matrizen ausbaut. Beispielsweise können Sie
die Bewegung eines Fingers simulieren.

Damit die Animation unabhängig von der Framerate läuft, stoppen Sie die
Zeit, die ein Frame benötigt. Hier können Sie folgenden Code nutzen:
```c
Uint64      ticksPerSecond = SDL_GetPerformanceFrequency();
Uint64      startCounter   = SDL_GetPerformanceCounter();
Uint64      endCounter     = SDL_GetPerformanceCounter();
while ( isRunning )
{
    double ticksPerFrame = (double)endCounter - (double)startCounter;
    double msPerFrame    = (ticksPerFrame / (double)ticksPerSecond) * 1000.0;
    startCounter         = SDL_GetPerformanceCounter();

    // Event loop, rendering, etc.

    endCounter = SDL_GetPerformanceCounter();
}
```
Selbstverständlich können Sie auch Funktionen aus der C++ Standardbibliothek
nutzen, wenn Ihnen das lieber ist.

## 3.8) Ship It.
Nutzen Sie das Script `shipit.sh` auf UNIX, bzw. `shipit.bat` auf Windows,
um einen Release-build Ihres Programms zu erstellen und es mit den assets
zu zippen.

**UNIX**:
```bash
./shipit.sh tasks_src/task03/
```
**Windows**:
```bash
shipit.bat tasks_src\task03\
```

Die gezippte Exe mitsamt libraries und `assets/` Ordner
ist nun im `release_build/` Ordner zu finden.
