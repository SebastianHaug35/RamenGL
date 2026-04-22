# Dokumentation Aufgabe 3

Diese Datei dokumentiert den aktuellen Implementierungsstand von Aufgabe 3.
Sie wird fortlaufend erweitert, sobald weitere Teilaufgaben umgesetzt werden.

## 3.1 Farbattribute

Der Vertex-Typ enthaelt neben Position und Normalenvektor auch eine Farbe.
Damit diese Farbe im Shader ankommt, wurde das VAO um ein drittes Attribut
erweitert:

- Location 0: Position
- Location 1: Normalenvektor
- Location 2: Farbe

Im Vertexshader `assets/shaders/task03.vert` wird `in_Color` an Location 2
eingelesen und als `out_Color` an den Fragmentshader weitergereicht. Der
Fragmentshader kann dadurch die pro Vertex gespeicherte Farbe verwenden.

## 3.2 Primitive: Quader, Zylinder, Kugel

In `tasks_src/task03/task03.cpp` wurden Funktionen zum Erzeugen der geforderten
Grundkoerper ergaenzt:

- `CreateCuboid(const Vec3f& color)`
- `CreateCylinder(const Vec3f& color, int segments = 32)`
- `CreateSphere(const Vec3f& color, int slices = 32, int stacks = 16)`

Alle Funktionen geben einen `std::vector<Vertex>` zurueck. Jeder Vertex bekommt
eine Position, einen Normalenvektor und die uebergebene Farbe.

Beim Quader werden fuer jede Flaeche eigene Vertices erzeugt. Dadurch kann jede
Flaeche einen konstanten Normalenvektor haben und die Kanten bleiben bei der
Beleuchtung hart sichtbar.

Beim Zylinder werden fuer den Mantel radiale Normalen verwendet, damit die Seite
rund und glatt beleuchtet wird. Deckel und Boden verwenden konstante Normalen,
weil sie flache Scheiben sind.

Bei der Kugel wird eine UV-Sphere aus Stacks und Slices erzeugt. Die
Normalenvektoren entsprechen jeweils der normalisierten Position auf der Kugel,
was eine glatte Beleuchtung ergibt.

Die Dreiecksreihenfolge der Kugel ist so gewaehlt, dass die Vorderseiten nach
aussen zeigen. Das ist wichtig, weil `GL_CULL_FACE` aktiv ist und sonst die
falsche Seite der Kugel gezeichnet bzw. beleuchtet werden kann. Die Normalen
zeigen ebenfalls nach aussen.

## 3.3 Rendern der Geometrie

Die erzeugten Vertices werden auf eigene VBOs hochgeladen und in der Kran-Szene
verwendet:

- Quader: Basis, Turm und Ausleger
- Zylinder: festes Seil
- Kugel: haengende Last

Alle Meshes verwenden dasselbe VAO-Layout. Vor jedem Drawcall wird der passende
VBO an das VAO gebunden und die jeweilige Model-Matrix als Uniform gesetzt.
Gerendert wird mit `glDrawArrays(GL_TRIANGLES, ...)`.

## 3.4 Debugging der Normalenvektoren

Fuer das Debugging der Normalenvektoren wurden zwei Methoden umgesetzt. Beide
Methoden sind im Programm beschriftet und koennen im ImGui-Fenster
`Normalen Debug` aktiviert werden.

Bedienung:

- Taste `1` oder Checkbox `1 Normalen als Farben`
- Taste `2` oder Checkbox `2 Normalenlinien`

### Methode 1: Normalen als Farben

Bei dieser Methode werden die transformierten Normalen direkt als Farbe
angezeigt. Da Normalen Werte im Bereich `[-1, 1]` haben, Farben aber im Bereich
`[0, 1]` erwartet werden, werden die Normalen im Fragmentshader umgerechnet:

```glsl
vec3 normalColor = normalize(in_Normal) * 0.5 + 0.5;
outColor = vec4(normalColor, 1.0);
```

Damit koennen Richtungen der Normalen visuell geprueft werden. Wenn benachbarte
Flaechen oder Bereiche unerwartete Farbspruenge zeigen, deutet das auf falsche
Normalen oder falsche Normalentransformation hin.

### Methode 2: Normalenlinien

Bei dieser Methode werden aus den Meshdaten zusaetzliche Linien erzeugt. Jede
Linie startet an einer Vertexposition und zeigt in Richtung des jeweiligen
Normalenvektors:

```cpp
endPosition = position + Normalize(normal) * length;
```

Diese Linien werden mit `GL_LINES` gerendert und gruen dargestellt. Sie helfen
dabei zu pruefen, ob die Normalen nach aussen zeigen und ob sie an Quader,
Zylinder und Kugel plausibel verteilt sind.

Die Normalenlinien werden aus Stichproben der Vertices erzeugt, damit die Szene
nicht mit zu vielen Debug-Linien ueberladen wird.

### Zusaetzlicher Beleuchtungstest

Die rotierende Punktlichtquelle dient als weiterer praktischer Test. Wenn die
Normalenvektoren korrekt sind, wandert die helle Seite der Objekte sichtbar mit
der Lichtposition. Fehlerhafte oder falsch transformierte Normalen wuerden sich
durch falsche Helligkeitsverlaeufe zeigen, zum Beispiel Beleuchtung auf der
Rueckseite oder nicht mitrotierende Helligkeit.

## 3.5 Beleuchtung

Es wurde ein einfaches Lambert-Beleuchtungsmodell mit Punktlichtquelle
implementiert.

Die Lichtposition wird in `task03.cpp` pro Frame berechnet:

- Die Zeit laeuft ueber `SDL_GetPerformanceCounter`.
- Die Lichtquelle bewegt sich auf einem Kreis um die Szene.
- Die Weltposition des Lichts wird mit der View-Matrix in View-Space
  transformiert.
- Die View-Space-Position wird als Uniform `u_LightViewPos` an den
  Fragmentshader uebergeben.

Im Fragmentshader wird die diffuse Helligkeit nach dem Lambert Cosine Law
berechnet:

```glsl
float diffuse = max(dot(normal, lightDir), 0.0);
```

Dabei ist `normal` der normalisierte Flaechennormalenvektor und `lightDir` die
normalisierte Richtung vom Fragment zur Lichtquelle. Der Dot-Product-Wert ist
gross, wenn Flaeche und Licht stark zueinander zeigen, und wird klein bzw. null,
wenn die Flaeche vom Licht wegzeigt.

Zusaetzlich gibt es einen kleinen Ambient-Anteil:

```glsl
vec3 ambient = 0.18 * in_Color;
vec3 litColor = ambient + diffuse * in_Color;
```

Dadurch werden unbeleuchtete Seiten nicht komplett schwarz.

### Transformation der Normalen

Normalen duerfen nicht einfach wie Positionen transformiert werden. Besonders
bei nicht-uniformer Skalierung kann eine normale Model- oder Model-View-Matrix
die Normalenrichtung verfaelschen.

Darum verwendet der Vertexshader eine Normal-Matrix:

```glsl
mat3 normalMat = transpose(inverse(mat3(modelViewMat)));
out_Normal = normalize(normalMat * in_Normal);
```

Die inverse transponierte Matrix erhaelt die Eigenschaft, dass der
Normalenvektor weiterhin senkrecht auf der transformierten Flaeche steht.

### Sichtbarer Licht-Marker

Zur Kontrolle wurde eine kleine weisse Kugel als Licht-Marker ergaenzt. Sie
bewegt sich an derselben Position wie die Punktlichtquelle. Damit kann man direkt
sehen, von welcher Richtung die Beleuchtung kommen sollte.

Der Marker wird mit `u_Unlit = 1` unbeleuchtet gerendert, damit er immer weiss
sichtbar bleibt.

## 3.6 Bewegung der Kamera

Die Kamera wurde um eine framerate-unabhaengige Tastatursteuerung erweitert.
Damit kann sie entlang ihrer eigenen Achsen verschoben und rotiert werden. Das
erfuellt die Aufgabenstellung, weil sowohl Translation als auch Rotation der
Kamera moeglich sind.

Bedienung:

- `W` / `S`: vorwaerts und rueckwaerts entlang der Blickrichtung
- `A` / `D`: links und rechts entlang der Kamera-Rechtsachse
- `Q` / `E`: runter und hoch entlang der Kamera-Hochachse
- Pfeil links / rechts: Yaw, also Drehung nach links und rechts
- Pfeil hoch / runter: Pitch, also Drehung nach oben und unten
- `R` / `F`: Roll, also Drehung um die Blickrichtung
- `Shift`: schnellere Bewegung

Die Steuerung wird pro Frame ueber `SDL_GetKeyboardState` abgefragt. Dadurch
funktionieren auch gehaltene Tasten fluessig, nicht nur einzelne Tastendruecke.

Die Geschwindigkeit wird mit der Framezeit multipliziert:

```cpp
moveStep = moveSpeed * deltaSeconds;
rotateStep = rotateSpeed * deltaSeconds;
```

Dadurch bewegt und rotiert sich die Kamera unabhaengig von der Framerate gleich
schnell. Bei niedriger Framerate werden die Schritte pro Frame groesser, bei
hoher Framerate kleiner.

Fuer die Translation wurden in `ramen/rgl_camera.h` kleine Hilfsmethoden
ergaenzt:

- `MoveForward`
- `MoveRight`
- `MoveUp`

Diese Methoden verschieben die Kameraposition entlang der aktuellen
Kameraachsen. Die vorhandenen Methoden `Yaw`, `Pitch` und `Roll` werden fuer die
Rotation weiterverwendet.

## 3.7 Matrizen-Stack und animierte Szene

Fuer Aufgabe 3.7 wurde eine einfache Kran-Szene umgesetzt. Die Szene ist bewusst
minimal gehalten, damit der MatrixStack gut nachvollziehbar bleibt und schnell
nachgebaut werden kann.

Der Kran besteht aus den drei in Aufgabe 3.2 erzeugten Grundkoerpern:

- Quader fuer Basis, Turm und Ausleger
- Zylinder fuer das feste Seil
- Kugel fuer die haengende Last

Damit werden Quader, Zylinder und Kugel nicht mehr separat als Demo-Objekte
gerendert, sondern direkt in der eigentlichen Szene verwendet. Unterschiedliche
Groessen und Positionen entstehen durch Model-Matrizen.

### MatrixStack-Klasse

In `task03.cpp` wurde eine kleine Klasse `MatrixStack` ergaenzt. Sie speichert
intern einen `std::vector<Mat4f>` und bietet diese Methoden:

- `Push`
- `Pop`
- `Last`
- `Translate`
- `Rotate`
- `Scale`

Die Transformationsmethoden verwenden die vorhandenen Funktionen aus
`rgl_math.h`, zum Beispiel `Translate`, `Rotate` und `Scale`.

### Hierarchie des Krans

Die Kran-Szene nutzt eine einfache Eltern-Kind-Hierarchie:

```text
Kran-Wurzel
  Basis
  Turm
  Ausleger
    Hakenpunkt
      Seil (Zylinder)
      Last (Kugel)
```

Der Ausleger ist ein Kind der Kran-Wurzel und wird am oberen Ende des Turms
rotiert. Der Hakenpunkt ist ein Kind des Auslegers. Seil und Last sind Kinder
dieses Hakenpunkts und erben dadurch die Auslegerrotation. Das Seil ist bewusst
statisch und schwingt nicht, damit die Szene einfach bleibt.

Damit wird eine Stack-Tiefe von mindestens drei erreicht:

```text
Kran-Wurzel -> Ausleger -> Hakenpunkt -> Last
```

### Transformationsarten

Alle drei geforderten Transformationsarten werden verwendet:

- `Translate`: positioniert Basis, Turm, Ausleger, Seil und Last.
- `Rotate`: dreht den Kran-Ausleger und rotiert den Zylinder in die Seilrichtung.
- `Scale`: skaliert Quader, Zylinder und Kugel auf die benoetigten Groessen.

Beispielhaft wird ein Bauteil so gezeichnet:

```cpp
craneStack.Push();
craneStack.Translate(Vec3f{ 0.9f, 0.0f, 0.0f });
craneStack.Scale(Vec3f{ 1.8f, 0.12f, 0.12f });
drawCuboid(craneStack.Last());
craneStack.Pop();
```

### Animation

Die Animation ist framerate-unabhaengig, weil sie auf `elapsedSeconds` basiert.
Diese Zeit wird aus `SDL_GetPerformanceCounter` berechnet.

Animiert werden:

- Drehung des Krans um die Y-Achse

Die Bewegung wird mit `sinf` erzeugt:

```cpp
craneYaw = sinf(elapsedSeconds * 0.7f) * 35.0f;
```

Dadurch bewegt sich der Kran kontinuierlich und unabhaengig von der Framerate.
Seil und Last bleiben relativ zum Ausleger fest. Das reduziert die Komplexitaet,
erfuellt aber weiterhin die geforderte Animation, weil der Ausleger mit Seil und
Last rotiert.

## Noch offen

- Vollstaendigen Build erneut pruefen. Der alte `build/`-Ordner regeneriert
  aktuell nicht sauber, und der frische `build-codex/`-Ordner braucht sehr lange
  beim Kompilieren der SDL-Abhaengigkeiten.

## Fehler und Korrekturen

### Kugel wirkte falsch beleuchtet

Symptom:

Die Kugel schien nicht korrekt mit der rotierenden Lichtquelle beleuchtet zu
werden. Die helle Seite wirkte so, als wuerde sie nicht sauber zur sichtbaren
weissen Lichtkugel passen.

Ursache:

Die Normalen der Kugel wurden zwar aus der normalisierten Position gebildet und
zeigten damit nach aussen. Die Dreiecksreihenfolge der Kugel war aber
gespiegelt. Bei aktivem `GL_CULL_FACE` und `glFrontFace(GL_CCW)` ist diese
Reihenfolge wichtig, weil OpenGL daraus bestimmt, welche Seite eines Dreiecks
die Vorderseite ist.

Wenn Winding und Normalenrichtung nicht zusammenpassen, kann die Kugel visuell
falsch wirken: Flaechen werden unerwartet weggecullt oder die Beleuchtung
scheint nicht zur Geometrie zu passen.

Korrektur:

Die Reihenfolge der Kugel-Dreiecke in `CreateSphere` wurde umgedreht, sodass die
Vorderseiten nach aussen zeigen und zu den nach aussen gerichteten Normalen
passen.

Vorher:

```cpp
AddSmoothTriangle(vertices, p00, p11, p01, color);
AddSmoothTriangle(vertices, p00, p10, p11, color);
AddSmoothTriangle(vertices, p00, p11, p01, color);
```

Nachher:

```cpp
AddSmoothTriangle(vertices, p00, p01, p11, color);
AddSmoothTriangle(vertices, p00, p11, p10, color);
AddSmoothTriangle(vertices, p00, p01, p11, color);
```

Test:

Nach dem Fix sollte die helle Seite der roten Kugel zur kleinen weissen
Lichtkugel zeigen und sichtbar mit ihr um die Kugel wandern.

Falls ein aehnlicher Fehler erneut auftritt, kann man zur Diagnose kurz
`glDisable(GL_CULL_FACE)` testen. Wenn das Bild dadurch stark anders aussieht,
liegt der Fehler wahrscheinlich bei der Dreiecksreihenfolge. Als zweite
Diagnose eignet sich ein Normalen-Farbmodus im Fragmentshader.

## Glossar

### Triangle-Winding-Reihenfolge / Dreiecksreihenfolge

Die Triangle-Winding-Reihenfolge beschreibt, in welcher Reihenfolge die drei
Eckpunkte eines Dreiecks an OpenGL uebergeben werden.

Ein Dreieck kann zum Beispiel so aufgebaut sein:

```cpp
AddTriangle(vertices, a, b, c, normal, color);
```

Wenn man von vorne auf dieses Dreieck schaut, koennen die Punkte entweder gegen
den Uhrzeigersinn oder im Uhrzeigersinn angeordnet sein.

- Counter-clockwise, kurz CCW: gegen den Uhrzeigersinn
- Clockwise, kurz CW: im Uhrzeigersinn

OpenGL nutzt diese Reihenfolge, um zu entscheiden, welche Seite eines Dreiecks
die Vorderseite ist. In diesem Projekt ist `glFrontFace(GL_CCW)` gesetzt. Das
bedeutet: Dreiecke, deren Eckpunkte von vorne gegen den Uhrzeigersinn erscheinen,
gelten als Vorderseiten.

Bei der Kugel war genau diese Reihenfolge wichtig. Die Normalen zeigten nach
aussen, aber die urspruengliche Dreiecksreihenfolge passte nicht dazu. Deshalb
wurde die Reihenfolge der Kugel-Dreiecke korrigiert.

### Vorderseite und Rueckseite eines Dreiecks

Ein Dreieck hat in OpenGL eine Vorderseite und eine Rueckseite. Welche Seite die
Vorderseite ist, haengt nicht vom Normalenvektor ab, sondern von der
Dreiecksreihenfolge und der Einstellung `glFrontFace`.

Das ist wichtig, weil OpenGL Rueckseiten optional gar nicht zeichnet. Diese
Optimierung heisst Face Culling.

### `GL_CULL_FACE`

`GL_CULL_FACE` ist ein OpenGL-Zustand. Wenn er aktiviert ist, zeichnet OpenGL
bestimmte Dreiecksseiten nicht.

Im Code:

```cpp
glEnable(GL_CULL_FACE);
glCullFace(GL_BACK);
```

Das bedeutet: Face Culling ist aktiv und OpenGL entfernt Rueckseiten
(`GL_BACK`). Dadurch werden weniger Dreiecke gerendert, was effizienter ist.

Der Nachteil: Wenn die Dreiecksreihenfolge falsch ist, kann OpenGL versehentlich
die eigentlich sichtbare Seite als Rueckseite behandeln. Dann koennen Flaechen
verschwinden oder optisch falsch wirken.

### `glFrontFace`

`glFrontFace` legt fest, welche Dreiecksreihenfolge als Vorderseite gilt.

Im Code:

```cpp
glFrontFace(GL_CCW);
```

`GL_CCW` bedeutet, dass Dreiecke mit gegen den Uhrzeigersinn angeordneten
Eckpunkten als Vorderseiten gelten. Alternativ gaebe es `GL_CW`, bei dem
Uhrzeigersinn als Vorderseite gilt.

Diese Einstellung muss zur erzeugten Geometrie passen. Fuer selbst erzeugte
Primitive wie Quader, Zylinder und Kugel muss man deshalb darauf achten, die
Eckpunkte der Dreiecke konsistent anzuordnen.

### Normalenvektor

Ein Normalenvektor ist ein Richtungsvektor, der senkrecht auf einer Flaeche
steht. Er wird fuer Beleuchtung gebraucht.

Beim Lambert-Beleuchtungsmodell wird berechnet, wie stark eine Flaeche zur
Lichtquelle zeigt:

```glsl
float diffuse = max(dot(normal, lightDir), 0.0);
```

Wenn Normalen falsch herum zeigen, wird die falsche Seite hell. Wenn Normalen
falsch transformiert werden, passt die Beleuchtung nicht mehr zur sichtbaren
Geometrie.

### Normal-Matrix

Die Normal-Matrix ist die Matrix, mit der Normalenvektoren korrekt transformiert
werden. Man kann Normalen nicht immer einfach mit derselben Matrix wie Positionen
transformieren, weil vor allem nicht-uniforme Skalierung die Senkrechtstellung
der Normalen zerstoeren kann.

Im Vertexshader:

```glsl
mat3 normalMat = transpose(inverse(mat3(modelViewMat)));
out_Normal = normalize(normalMat * in_Normal);
```

Die inverse transponierte Matrix sorgt dafuer, dass der Normalenvektor nach der
Transformation weiterhin senkrecht auf der Flaeche steht.

### Normalenlinien

Normalenlinien sind Debug-Geometrie. Sie werden nicht fuer das eigentliche
Rendering gebraucht, sondern nur zur Kontrolle der Normalenvektoren.

Eine Normalenlinie beginnt an einer Vertexposition und endet ein kleines Stueck
in Richtung des Normalenvektors. Wenn die Linien nach aussen zeigen, sind die
Normalenrichtungen plausibel. Wenn sie nach innen oder chaotisch zeigen, stimmt
die Normalenerzeugung wahrscheinlich nicht.

### Lambert-Beleuchtung / Lambert Cosine Law

Lambert-Beleuchtung ist ein einfaches diffuses Beleuchtungsmodell. Die
Helligkeit einer Flaeche haengt davon ab, wie direkt sie zur Lichtquelle zeigt.

Mathematisch wird dafuer der Dot Product zwischen Normalenrichtung und
Lichtrichtung verwendet:

```glsl
float diffuse = max(dot(normal, lightDir), 0.0);
```

Zeigt die Flaeche direkt zur Lichtquelle, ist der Wert gross. Zeigt sie seitlich
oder weg von der Lichtquelle, wird der Wert kleiner bzw. null.

### Punktlichtquelle

Eine Punktlichtquelle ist eine Lichtquelle mit einer Position im Raum. Das Licht
kommt also nicht aus einer festen Richtung, sondern von einem konkreten Punkt.

In dieser Aufgabe bewegt sich die Punktlichtquelle auf einem Kreis um die Szene.
Die kleine weisse Kugel zeigt die aktuelle Position der Lichtquelle an.

### View-Space

View-Space ist das Koordinatensystem aus Sicht der Kamera. Nachdem ein Punkt mit
der View-Matrix transformiert wurde, liegt er im View-Space.

In dieser Aufgabe werden sowohl Fragmentpositionen als auch Lichtposition in
View-Space verwendet. Das ist wichtig, weil Beleuchtungsrechnungen nur korrekt
sind, wenn alle beteiligten Vektoren im selben Koordinatensystem liegen.

### Delta-Time / Framezeit

Delta-Time ist die Zeit, die seit dem letzten Frame vergangen ist. Sie wird
benutzt, damit Animationen und Eingaben unabhaengig von der Framerate laufen.

Ohne Delta-Time wuerde sich die Kamera auf schnellen Rechnern schneller bewegen,
weil mehr Frames pro Sekunde berechnet werden. Mit Delta-Time wird stattdessen
eine Geschwindigkeit pro Sekunde angegeben und pro Frame auf die vergangene Zeit
umgerechnet.

### Yaw, Pitch und Roll

Yaw, Pitch und Roll beschreiben Rotationen der Kamera:

- Yaw: Drehung nach links oder rechts um die Hochachse
- Pitch: Drehung nach oben oder unten um die Seitenachse
- Roll: Drehung um die Blickrichtung

In Aufgabe 3.6 werden diese drei Rotationsarten per Tastatur gesteuert.

### SDL Scancode

Ein SDL Scancode beschreibt die physische Taste auf der Tastatur. Fuer
kontinuierliche Eingaben wird in dieser Aufgabe `SDL_GetKeyboardState`
verwendet. Damit kann pro Frame abgefragt werden, ob eine Taste gerade gehalten
wird, zum Beispiel `SDL_SCANCODE_W` oder `SDL_SCANCODE_LEFT`.

### MatrixStack / Matrizenstapel

Ein MatrixStack ist ein Stapel von Transformationsmatrizen. Er wird benutzt, um
hierarchische Modelle zu zeichnen. Ein Kindobjekt erbt dabei die Transformation
des Elternobjekts.

Typische Operationen:

- `Push`: aktuelle Matrix kopieren und oben auf den Stapel legen
- `Pop`: oberste Matrix entfernen und zum Elternzustand zurueckkehren
- `Last`: aktuelle oberste Matrix verwenden

Beim Kran wird dadurch zum Beispiel die Last automatisch mit dem drehenden
Ausleger mitbewegt.

### Hierarchische Transformation

Eine hierarchische Transformation bedeutet, dass mehrere Objektteile voneinander
abhaengen. Wenn der Elternteil bewegt oder rotiert wird, bewegen sich die
Kindteile mit.

Beim Kran ist der Ausleger ein Kind der Kran-Wurzel. Der Hakenpunkt ist ein Kind
des Auslegers, und Seil und Last sind Kinder dieses Hakenpunkts. Deshalb reicht
es, den Ausleger am oberen Turm zu rotieren, und Seil und Last rotieren
automatisch mit.
