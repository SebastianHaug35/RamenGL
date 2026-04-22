# Task 04 - Texturen

## 4.0) Setup
Clonen Sie das aktuelle Rahmenprogramm.

Nutzen Sie das Sourcefile `task04.cpp` in `./tasks_src/` um die Aufgaben zu implementieren.
Kompilieren Sie es zunächst.
Starten Sie das Programm mit dem `assets/` Pfad als Argument, also, zB:
```bash
./build/task04 assets/
```
auf UNIX. Und
```bash
build\Debug\task04.exe assets\
```
auf Windows. Sie können auch absolute Pfade angeben.
Führen Sie ausserdem das `setup.sh` bzw. `setup.bat` Skript
aus, damit Sie die neuen Assets bekommen.

Sie sollten ein rotes Quadrat sehen.

## 4.1) Hinzufügen von Texturkoordinaten zu einem 'Quad'
In `task04.cpp` sind bereits Vertices und Indices für einen 'Quad'
beschrieben. Ein Quad besteht aus zwei dreiseitigen Polygonen, die
in derselben Ebene liegen. Jede Seite ist gleich lang (ein Quadrat in 3D).
Weisen Sie ihren Quad-Vertices Texturkoordinaten zu, sodass die vier
Eckpunkte eines quadratischen Bildes den Vertices zugeordnet werden.
Nutzen Sie `Vec3f`s für die Texturkoordinaten, auch wenn Sie nur zwei
Komponenten benötigen! Der Grund dafür wird in einer späteren Aufgabe
ersichtlich.

## 4.2) Erstellen einer OpenGL Textur auf der GPU
Nutzen Sie folgenden code, um aus dem geladenen Bild (bereits in `task04.cpp`
implementiert) eine OpenGL-Textur zu erstellen:
```cpp
GLuint textureHandle;
glCreateTextures(GL_TEXTURE_2D, 1, &textureHandle);
glTextureParameteri(textureHandle, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTextureParameteri(textureHandle, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
glTextureParameteri(textureHandle, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
glTextureParameteri(textureHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTextureStorage2D(textureHandle, 1, GL_RGBA8, image.GetWidth(), image.GetHeight());
glTextureSubImage2D(textureHandle, 0, 0, 0, image.GetWidth(), image.GetHeight(), GL_RGBA, GL_UNSIGNED_BYTE, image.Data());
```
Finden Sie heraus, was die einzelnen Befehle bewirken und nehmen Sie
die Erkenntnisse in Ihre Dokumentation mit auf.

## 4.3) Zugänglich machen der Texturkoordinaten im Shader
'Verdrahten' Sie nun das Texturkoordinaten-Attribut über das VAO
mit dem Vertex-Shader, sodass die Texturkoordinaten im in diesem verfügbar werden. Ermöglichen Sie ausserdem
die Weiterleitung der Texturkoordinaten aus dem Vertex- in den Fragment-Shader.

## 4.4) Samplen der OpenGL Textur im Fragment-Shader
Nun müssen Sie noch mithilfe der Texturkoordinaten und der OpenGL-Textur
im Shader die Fragmente mit den Pixeln der Textur versehen.
Fügen Sie Fragment-Shader diese Zeile hinzu:
```glsl
layout(binding = 0) uniform sampler2D u_Texture;
```
Finden Sie nun den entsprechenden CPU-Seitigen Befehl, der
die OpenGL-Textur, welche Sie in 4.4 erstellt haben, mit
dem Shader verbindet.
Sie können nun auf das `u_Texture` uniform im Fragment-Shader zugreifen und
die Texel aus der Textur samplen:
```glsl
vec4 texColor = texture(u_Texture, in_UV.st);
```
Um `in_UV` handelt es sich dabei um die Texturkoordinaten,
die sie aus dem Vertex-Shader an den Fragment-Shader weitergegeben haben.

Wenn Sie nun das Programm starten, sollten Sie die Textur
auf dem Quad sehen.

Je nachdem, wie Sie Ihre Texturkoordinaten erstellt haben, sehen
Sie das Bild auf dem Kopf. Wenn dem so ist, finden Sie eine Erklärung
und einen Weg, dies zu ändern. Aber auch, wenn das Bild nicht
auf dem Kopf steht, versuchen Sie zu verstehen, warum es passt.
Nehmen Sie die Erkenntnisse in Ihre Dokumentation mit auf.

## 4.5) Texturkoordinaten für die Kugel erstellen
Nutzen Sie die Kugel, die Sie in task03 erstellt haben und
weisen Sie dieser so Texturkoordinaten zu, sodass sich eine
Weltkarte auf diese 'mappen' lässt. Rendern Sie die Kugel.

## 4.6.) Rotation der Kugel
Ermöglichen Sie die Rotation der Kugel mithilfe
der Pfeiltasten, sodass Sie die Kugel von allen
Blickwinkeln betrachten können.


