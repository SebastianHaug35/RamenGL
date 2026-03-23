# Task 02

## 1.) Vertex Daten auf GPU laden und rendern.

Laden Sie sich die Modell herunter: https://syncandshare.lrz.de/getlink/fiXxtfc7YgH8Dg6u2paF4G/  

und entpacken Sie die Modelle in den Ordner `models` im
root-directory des Rahmenprogramms.

Komplieren Sie das Programm `task02.cpp`:
```bash
cmake -B build -S .
cmake --build build
```

Unter UNIX ist das Programm unter `build/task02` zu finden.  
Unter Windows unter `build/Debug/task02`.

Das Programm lädt das Modell `stormtrooper.obj` aus dem
eben herunterlgeladenen Modellset.

**Achtung**: Den Pfad zum Modell `stormtrooper.obj` müssen Sie,
je nach Plattform, anpassen. Achten Sie etwa darauf, welches
`working directory` in Visual Studio eingestellt ist.

Ihre Aufgabe ist es nun, die Vertex-Daten des Modells mithilfe
eines Vertex-Buffers auf die GPU zu laden und dieses zu rendern.

Recherchieren Sie hierzu mithilfe der OpenGL Dokumentation
(oder anderweitigen Quellen) die Funktionen  
```c
glCreateBuffers(...)
glNamedBufferSubData(...)
glCreateVertexArrays(...)
glVertexArrayVertexBuffer(...)
glVertexArrayAttribBinding(...)
glVertexArrayAttribFormat(...)
glEnableVertexArrayAttrib(...)
glBindVertexArray(...)
glDrawArrays(...)
```

## 2.) Dokumentation
Fertigen Sie eine kurze Dokumentation an, welche erklärt, wozu diese
Funktionen gebraucht werden, wie sie zusammenhängen und welche
Dinge unverständlich für Sie sind. Nehmen Sie die Dokumentation ernst.
Sie dient nicht nur zum Bestehen des Praktikums sondern auch für
Sie selbst zum Nachschlagen. Wie Sie vielleicht schon gemerkt haben
ist die Benamung der Funktionen nicht unbedingt aufschlussreich.  
Recherchieren Sie ausserdem wie die Funktion
`glCreateBuffers` sich von `glGenBuffers` unterscheidet.
**Achtung**: Bitte nutzen Sie *nicht* `glGenBuffers`.

## Hinweis:
Nutzen Sie RenderDoc, falls Sie der Meinung sind im Code
alles korrekt implementiert zu haben und machen Sie sich mit
der Funktionsweise vertraut.
Nutzen Sie RenderDoch **auch** wenn alles geklappt hat :)





