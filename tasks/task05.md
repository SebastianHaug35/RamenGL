# Task 05 - Cubemapping

## 5.0) Setup
Sorgen Sie dafür, dass Sie die neueste Version von RamenGL
installiert haben. Kompilieren und starten Sie nun `task05.cpp`.
Sie sollten ein Fenster sehen mit einem UI Fenster.
Sie werden in der folgenden Aufgabe einige UI Elemente in dieses
Fenster hinzufügen.

## 5.1.0) Erstellen der Cubemap
Erstellen Sie sich die Vertexdaten für einen Würfel. Sie können
dazu natürlich Ihren Würfel aus vergangener Aufgabe nutzen.
Achten Sie aber arauf, dass sich der Ursprung des Würfels in dessen
Schwerpunktmittelpunkt befindet und die Frontfaces des Würfels
nach **innen** zeigen.
Laden Sie dann, wie bisher, die Vertex (und ggf. Index) -Daten
auf die GPU. Schreiben Sie einen Vertex- und Fragmentshader,
der Ihnen den Würfel zeichnet. Sie können die Shader `task05.vert`
und `task05.frag` als Ausgang nutzen. Sie müssen kein Bleuchtungsmodell
implementieren. Es reicht, wenn Sie z.B. erst einmal die
Normalenvektoren visualisieren.

## 5.1.1) Erstellen der 3D Textur
Laden Sie nun sechs Bilddateien (`Image::Load`), die auf die Seiten
des Würfels gemappt werden sollen. In Ihrem `assets/textures`
Ordner befindet sich ein Ordner namens `cubemaps`. Dort ist
bereits ein Ordner mit Cubemaps enthalten. Wählen Sie aber
gerne ein eigenes Texturset, z.B. von der Seite:
https://www.humus.name/index.php?page=Textures.

Erzeugen Sie nun eine neue OpenGL Textur (`glCreateTextures`).
Diesmal handelt es sich aber um den Typ: `GL_TEXTURE_CUBEMAP`.
Diese Texturart erlaubt es Ihnen im Shader mit einem
3D-Richtungsvektor von den sechs Seiten der Cubemap zu samplen.
Den Texturspeicher legen Sie mit `glTextureStorage2D` an.
Nun können Sie die sechs geladenen Bilder mithilfe der
Funktion `glTextureSubImage3D` in den angelegten Speicher
der Grafikkarte laden.
Nehmen Sie in Ihre Dokumentation mit auf für was die Parameter
von `glTextureSubImage3D` stehen und wie Sie vorgegangen sind.
Spielt die Reihenfolge, mit der Sie die einzelnen Bilder hochladen,
eine Rolle?

## 5.1.2) Samplen von der Cubemap Textur
Binden Sie nun die eben erstellte Cubemap Textur und samplen
Sie von dieser im Shader. Nutzen Sie dazu einen `samplerCube` in
Ihrem Fragmentshader. Nutzen Sie die OpenGL Dokumentation, um
herauszufinden, wie Sie auf die korrekten Texturkoordinaten zugreifen.
Vielleicht kommen Sie aber auch von selbst drauf :)
Hinweis: Es bedarf **nicht** der UV Koordinaten aus den Vertexdaten.

## 5.1.3) Kamera
Implementieren Sie Tastaturinput, sodass Sie mit der Kamera
in der Szene herumfahren und rotieren können.

## 5.2) Environment Mapping
Sie können die Cubemap dafür nutzen, um (perfekte) Reflexionen 
an einem Modell zu simulieren. Laden Sie dazu ein Modell
(`Model::Load`) aus `assets/models` und rendern Sie es mit
einem eigenen Paar aus Vertex- und Fragmentshadern. Ermöglichen
Sie dem neuen Shaderpaar ebenfalls auf die Cubemap Textur zuzugreifen.
Lassen Sie nun die Cubemap an Ihrem Modell spiegeln. Nutzen Sie
dafür die GLSL Funktion `reflect`. Achten Sie bei der Implementierung
darauf, dass sich verrechnete Vektoren im gleichen Koordinatensystem
befinden!

## 5.3) FPS Camera
Ändern Sie nun das Verhalten der Cubemap so,
dass Sie diese nicht mehr verlassen können! Dies soll den Eindruck
eines unendlich entfernt liegenden Horizonts simulieren, wie
es auch in vielen Videospielen der Fall ist.  
Nutzen Sie die ImGUI Bibliothek, um ein UI Element zu erstellen, das Sie
zwischen der ursprünglichen Version und dem unendlich weit entfernten
Horizont umschalten lässt.  

Beispielvideo:
[Cubemapping FPS-mode vs Escape-mode](https://syncandshare.lrz.de/getlink/fiNakx3QXFzQPcTKAQWmja/cubemap-fps-vs-escape-1080p-x264-high_compression.mp4) 

*Hinweis*: Es gibt mehrere Möglichkeiten dieses Verhalten zu implementieren.
Für eine der Möglichkeiten benötigen Sie die Funktion `glDepthMask()`
(https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDepthMask.xhtml).  

