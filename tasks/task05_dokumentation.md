# Dokumentation Aufgabe 5

Diese Datei dokumentiert den aktuellen Implementierungsstand von Aufgabe 5.
Sie wird fortlaufend erweitert, sobald weitere Teilaufgaben umgesetzt werden.

## 5.0 Setup

`task05.cpp` wurde erfolgreich in das bestehende Rahmenprogramm eingebunden und
kann als eigenes Build-Target kompiliert und gestartet werden.

Build unter Windows:

```powershell
cmake -S . -B build
cmake --build build --target task05 --config Debug
.\build\Debug\task05.exe assets\
```

Beim Start wird das `assets/` Verzeichnis gemountet, OpenGL initialisiert und
ein ImGui-Fenster angezeigt. Damit ist das Grundgeruest fuer die weiteren
Cubemap-Aufgaben vorhanden.

## 5.1.0 Erstellen der Cubemap-Geometrie

Als erster Schritt wurde ein Wuerfelmesh direkt in `task05.cpp` erzeugt. Der
Wuerfel ist um den Ursprung zentriert und verwendet eine Dreiecksreihenfolge,
bei der die Frontfaces nach innen zeigen. Das ist wichtig, weil die Kamera bei
einer Cubemap spaeter im Inneren des Wuerfels steht.

### Warum nach innen gerichtete Frontfaces?

Mit aktivem `GL_CULL_FACE` und `glCullFace(GL_BACK)` werden Rueckseiten
verworfen. Eine gewoehnliche Box wuerde ihre sichtbaren Flaechen nach aussen
zeigen. Betrachte ich sie von innen, sehe ich dann nur Rueckseiten. Fuer eine
Skybox oder Cubemap muss es genau umgekehrt sein: die sichtbaren Flaechen
muessen nach innen zeigen.

### GPU-Upload

Die Vertexdaten werden mit einem VBO auf die GPU geladen und ueber ein VAO mit
dem Shader verbunden.

- Location 0: Position
- Location 1: Normalenvektor

Es wird bewusst noch kein Indexbuffer verwendet, weil fuer den Einstieg eine
expandierte Dreiecksliste mit 36 Vertices ausreicht.

### Shader fuer den ersten Test

Der erste Shader rendert den Wuerfel ohne Beleuchtungsmodell. Stattdessen wird
der Normalenvektor farblich visualisiert:

```glsl
vec3 normalColor = normalize(in_Normal) * 0.5 + 0.5;
```

So laesst sich schnell pruefen, dass der Wuerfel tatsaechlich gezeichnet wird
und dass pro Flaeche ein konsistenter Normalenvektor ankommt.

### Aktueller Stand

Der Task rendert jetzt einen innengewendeten Wuerfel und zeigt ihn im Fenster
zusammen mit einem kleinen ImGui-Statusfenster an. Damit ist 5.1.0 in einem
ersten, sichtbaren Zwischenschritt umgesetzt. Die eigentliche Cubemap-Textur
mit sechs Bildern folgt in 5.1.1.

### Richtungsachsen einer Cubemap

Die sechs Bilder einer Cubemap entsprechen sechs festen Raumrichtungen. Dabei
stehen die Achsen fuer die drei kartesischen Richtungen des Raums:

- **X-Achse**: links/rechts
- **Y-Achse**: unten/oben
- **Z-Achse**: hinten/vorne

Das Vorzeichen entscheidet jeweils die Richtung auf dieser Achse:

- **+X**: positive X-Richtung
- **-X**: negative X-Richtung
- **+Y**: positive Y-Richtung
- **-Y**: negative Y-Richtung
- **+Z**: positive Z-Richtung
- **-Z**: negative Z-Richtung

Bei einer Cubemap bedeutet das: jede dieser sechs Richtungen bekommt genau ein
Bild. Ein spaeterer Richtungsvektor im Shader waehlt dann automatisch die
passende Seite der Cubemap aus.

### Glossar

- **Cubemap**: Umgebungstextur aus sechs Bildern, eines pro Wuerfelseite.
- **Frontface**: Vorderseite eines Dreiecks gemaess seiner Vertex-Reihenfolge.
- **Backface Culling**: Verwerfen von Rueckseiten beim Rendern.
- **VBO**: Vertex Buffer Object, GPU-Speicher fuer Vertexdaten.
- **VAO**: Vertex Array Object, beschreibt das Layout der Vertexattribute.
- **Normalenvektor**: Vektor senkrecht auf einer Flaeche.
- **X/Y/Z-Achse**: Die drei Grundachsen des 3D-Raums.

## 5.1.1 Erstellen der 3D Textur

Fuer die Cubemap wurden sechs Bilddateien geladen:

- `posx.jpg`
- `negx.jpg`
- `posy.jpg`
- `negy.jpg`
- `posz.jpg`
- `negz.jpg`

Diese sechs Bilder repraesentieren die sechs Richtungen der Cubemap und werden
in genau dieser Reihenfolge auf die sechs Faces der OpenGL-Cubemap geladen.

### Erzeugen des Texturobjekts

Zuerst wird ein Texturobjekt vom Typ `GL_TEXTURE_CUBEMAP` erzeugt:

```cpp
GLuint cubemapHandle = 0;
glCreateTextures(GL_TEXTURE_CUBEMAP, 1, &cubemapHandle);
```

Danach werden Texturparameter gesetzt. `GL_CLAMP_TO_EDGE` wird fuer alle drei
Achsen `S`, `T` und `R` verwendet, damit es an den Naehten der Cubemap nicht zu
unerwuenschten Randartefakten kommt. Die Filterung ist auf `GL_LINEAR`
gestellt.

### Speicher anlegen

Mit `glTextureStorage2D` wird der GPU-Speicher fuer die Cubemap angelegt:

```cpp
glTextureStorage2D(cubemapHandle, 1, GL_RGBA8, width, height);
```

Obwohl die Cubemap aus sechs Bildern besteht, reicht ein einziger Aufruf, weil
OpenGL durch den Texturtyp bereits weiss, dass Speicher fuer sechs Faces
reserviert werden muss.

### Hochladen der sechs Faces

Die sechs Bilder werden mit `glTextureSubImage3D` hochgeladen. Bei einer
Cubemap bezeichnet der Parameter `zoffset`, welches Face beschrieben wird.
Die Reihenfolge lautet:

- `0` -> `+X`
- `1` -> `-X`
- `2` -> `+Y`
- `3` -> `-Y`
- `4` -> `+Z`
- `5` -> `-Z`

Deshalb spielt die Reihenfolge der hochgeladenen Bilder eine Rolle. Werden die
Seiten vertauscht, ist die Umgebung spaeter räumlich falsch orientiert.

### Bedeutung der Parameter von `glTextureSubImage3D`

```cpp
glTextureSubImage3D(texture,
					level,
					xoffset,
					yoffset,
					zoffset,
					width,
					height,
					depth,
					format,
					type,
					data);
```

- **texture**: Handle der Zieltextur.
- **level**: Mipmap-Level, hier `0`.
- **xoffset**: Startposition innerhalb des Faces in X-Richtung.
- **yoffset**: Startposition innerhalb des Faces in Y-Richtung.
- **zoffset**: Face-Index der Cubemap.
- **width**: Breite des hochzuladenden Bildes.
- **height**: Hoehe des hochzuladenden Bildes.
- **depth**: Anzahl der zu schreibenden Layer, hier `1`.
- **format**: Format der Quelldaten, hier `GL_RGBA`.
- **type**: Datentyp der Quelldaten, hier `GL_UNSIGNED_BYTE`.
- **data**: Zeiger auf die CPU-Bilddaten.

### Aktueller Stand

Die sechs Bilder werden jetzt geladen, als `GL_TEXTURE_CUBEMAP` auf der GPU
angelegt und auf die sechs Cubemap-Faces kopiert. Der Shader verwendet die
Cubemap im Moment noch nicht zum Sampling; dieser Schritt folgt in 5.1.2.

### Glossar

- **Face**: Eine der sechs Seiten einer Cubemap.
- **+X/-X/+Y/-Y/+Z/-Z**: Die sechs festen Richtungen des Cubemap-Raums.
- **Texture Handle**: OpenGL-ID eines Texturobjekts.
- **Sampler**: Shader-Zugriff auf eine Textur.
- **Mipmap-Level**: Aufloesungsstufe einer Textur.
- **Texel**: Ein einzelner Bildpunkt einer Textur.
- **DSA**: Direct State Access, direkter Zugriff auf OpenGL-Objekte ohne Binden.

## 5.1.2 Samplen von der Cubemap-Textur

In diesem Schritt wird die bereits erzeugte Cubemap beim Rendern auch wirklich
verwendet. Dazu bindet die CPU-Seite die Textur an Texture Unit `0`:

```cpp
glBindTextureUnit(0, cubemapHandle);
```

Der Fragmentshader deklariert dazu einen `samplerCube` mit festem Binding:

```glsl
layout(binding = 0) uniform samplerCube u_Cubemap;
```

Dadurch greifen CPU und Shader auf dieselbe Texture Unit zu.

### Richtung statt UV-Koordinaten

Eine Cubemap wird nicht mit klassischen UV-Koordinaten abgetastet. Stattdessen
benoetigt der Shader einen 3D-Richtungsvektor. Im Vertexshader wird daher die
Weltposition des Wuerfel-Vertices berechnet und als Richtung weitergegeben:

```glsl
vec4 worldPos = u_ModelMat * vec4(in_Position, 1.0f);
out_Direction = worldPos.xyz;
```

Im Fragmentshader wird diese Richtung normalisiert und zum Sampling verwendet:

```glsl
vec3 sampleDir = normalize(in_Direction);
outColor = texture(u_Cubemap, sampleDir);
```

OpenGL bestimmt auf Basis dieses Richtungsvektors selbst, welches Face der
Cubemap angesprochen wird und an welcher Stelle darauf gesampelt wird.

### Aktueller Stand

Die Cubemap wird nun nicht mehr nur auf die GPU geladen, sondern auch im Shader
gesampelt und auf dem innenliegenden Wuerfel dargestellt.

### Glossar

- **samplerCube**: GLSL-Typ zum Zugriff auf eine Cubemap-Textur.
- **Texture Unit**: Steckplatz, an den eine Textur fuer das Rendering gebunden wird.
- **Binding**: Feste Zuordnung zwischen Shader-Sampler und Texture Unit.
- **Uniform**: Shader-Variable, die von der CPU gesetzt wird und fuer viele Vertices oder Fragmente gleich bleibt.
- **Sample Direction**: Normalisierter Richtungsvektor, mit dem in die Cubemap geschaut wird.
- **World Position**: Position eines Vertex nach Anwendung der Model-Matrix.

## 5.1.3 Kamera

Damit die Umgebung nicht statisch bleibt, wurde eine einfache freie Kamera mit
Tastatursteuerung ergaenzt. Die Bewegung wird pro Frame ueber
`SDL_GetKeyboardState` abgefragt und mit `deltaSeconds` skaliert. Dadurch bleibt
die Geschwindigkeit weitgehend unabhaengig von der Framerate.

### Bewegung

Verwendete Tasten:

- `W` / `S`: vorwaerts / rueckwaerts
- `A` / `D`: links / rechts
- `Q` / `E`: hoch / runter

Diese Eingaben rufen direkt die vorhandenen Kamerafunktionen auf:

```cpp
camera.MoveForward(...);
camera.MoveRight(...);
camera.MoveUp(...);
```

### Rotation

Zusaetzlich wurde die Rotation auf die Pfeiltasten gelegt:

- Pfeil links / rechts: `Yaw`
- Pfeil hoch / runter: `Pitch`

Dadurch kann die Blickrichtung im Raum veraendert werden, ohne die Kamera nur
entlang einer festen Achse zu verschieben.

### Warum `deltaSeconds`?

Ohne Zeitskalierung waere die Kamera auf schnellen Rechnern schneller und auf
langsamen Rechnern langsamer. Mit

```cpp
float deltaSeconds = (float)(msPerFrame / 1000.0);
```

wird Bewegung als Strecke oder Winkel pro Sekunde statt pro Frame formuliert.

### Aktueller Stand

Die Szene kann jetzt mit der Kamera erkundet werden. Dadurch laesst sich direkt
ueberpruefen, dass die Cubemap beim Blick in verschiedene Richtungen korrekt
gesampelt wird.

### Glossar

- **deltaSeconds**: Vergangene Zeit seit dem letzten Frame in Sekunden.
- **Framerate-unabhaengig**: Verhalten bleibt bei unterschiedlicher FPS moeglichst konstant.
- **Yaw**: Drehung um die lokale Hochachse der Kamera.
- **Pitch**: Drehung um die Seitenachse der Kamera.
- **Forward/Right/Up**: Lokale Vorwaerts-, Rechts- und Hochrichtung der Kamera.