# Introduction à OpenGL

Cette page est consacrée à comprendre ce qu'*est* OpenGL, à savoir comment il fonctionne et comment s'en servir. Nous n'aborderons que les bases ici, les utilisations plus compliquées seront détaillées ailleurs.

## Qu'est ce qu'est OpenGL ?

OpenGL est une *spécification gpu*, une norme qui défini des fonctions disponibles sur les cartes graphiques qui l'implémentent. On utilisera l'API (application programming interface) OpenGL qui permet de dialoguer avec le gpu de son ordinateur. On l'utilise principalement pour l'affichage graphique mais elle peut aussi être utilisé pour le calcul haute performance.

> [!info]
>
> OpenGL n'est pas la seule API graphique disponible, aujourd'hui les jeux utilisent plutôt Vulkan ou DirectX mais OpenGL est une très bonne base pour commencer. Pour ce que nous allons en faire c'est l'idéal.

OpenGL est une *API*, ça signifie qu'il n'y a pas de "code OpenGL" à proprement parler, quand nous appellerons les fonction `glXX` elles s'exécuteront directement sur le GPU.

OpenGL est disponible en Java, C, C++, Python et même en Javascript, pour commencer vous pourrez trouver les fichiers nécessaires facilement sur internet.

## Le paradigme OpenGL

Un code typique ressemblera à ceci :

```cpp
int main() {
  // initialization
  createWindow();
  createGL();

  // chargement des ressources
  loadTextures();
  loadMeshes();
  loadShaders();

  // boucle principale
  while(!window::shouldDispose()) {
    glClear(GL_COLOR_BUFFER_BIT);
    // afficher des éléments
    renderScene();
    // afficher à l'écran
    pollUserInputs();
    sendFrame();
  }

  // destruction des ressources
  disposeGL();
  disposeWindow();
}
```

> [!info]
>
> OpenGL ne gère pas la création de la fenêtre, pour ça on utilisera la librairie **glfw**, disponible pour les mêmes langages que l'api OpenGL.

L'API est de très bas niveau, ce qui implique que c'est à nous de presque tout faire... Pour afficher quoique ce soit il va déjà nous falloir un long moment.

L'API fonctionne avec le concept de *state machine*, les fonctions sont nommées `glXX` et chaque fonction modifie l'*état* du gpu (typiquement `glEnable/Disable(GL_BLEND)` active/désactive la gestion de la transparence). Ce paradigme ne correspond pas vraiment à la programmation en C++ donc nous créerons une abstraction de l'API - concrètement chaque "concept OpenGL" aura sa classe en C++ -.

## Hello, Triangle!

Un gpu n'est capable d'afficher que des points, des lignes et des triangles. Mais même pour afficher un triangle il faut comprendre beaucoup de choses...

La fonction pour afficher des primitives est [`glDrawElements`](https://docs.gl/gl4/glDrawElements). Il lui faut d'abord un *VBO*, un *IBO*, et un *shader* :

### Les Vertices

Un triangle est défini par 3 sommets (un *vertex*, des *vertices* en anglais), mais OpenGL ne défini pas ce qu'*est* un sommet. Pour l'instant nous définirons un sommet par une position et une couleur :

```cpp
struct Vertex {
	glm::vec3 position; // xyz
	glm::vec4 color;    // rgba
}

Vertex triangle[3] = {...};
```

Pour envoyer notre triangle sur le gpu il nous faut un *vertex buffer* (*vbo*), qui définit un espace mémoire sur le gpu :

```cpp
GLint vbo;
glGenBuffers(1, &vbo);
glBindBuffer(GL_ARRAY_BUFFER, vbo);
glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
```

`glGenBuffers` créé le vbo. `glBindBuffer` attache le vbo à l'*état* du système, donc les prochaines commandes qui modifieront `GL_ARRAY_BUFFER` modifieront *ce* vbo. `glBufferData` définie la taille du vbo côté gpu et exporte son contenu.

> Le code pour la création d'un vbo est très similaire à celui des autres buffers, il ne sera pas réécrit à chaque fois.

Maintenant le gpu possède les données de notre triangle, mais il ne sait pas comment les interpréter, il faut lui donner un *layout* pour lui dire :

```cpp
size_t stride = sizeof(Vertex);
size_t posOffset = offsetof(Vertex, position); // = 0
size_t colOffset = offsetof(Vertex, color);    // = 3*sizeof(float)
// 3 floats pour la position
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, *(const void**)&posOffset);
// 4 floats pour la couleur
glEnableVertexAttribArray(1);
glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, *(const void**)&colOffset);
```

Ce que fait ce code sera plus claire quand nous aborderons les shaders, et dans la documentation de [glVertexAttribPointer](https://docs.gl/gl4/glVertexAttribPointer).

### Les indices

Quand on veut afficher un carré on affiche deux triangles, mais pour éviter de dupliquer les sommets on utilise des indices - on stockerait 4 sommets et on afficherait les deux triangles constitués des sommets 0,1,2 et 2,3,0 -.

De la même manière que pour les vertices, il nous faut un *index buffer* (*ibo*). Pour l'instant il nous suffit d'afficher les sommets dans l'ordre 0,1,2 :

```cpp
int indices[3] = { 0, 1, 2 };
// [...]
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
```

### Les shaders

Un *shader* est un morceau de code qui s'exécute sur le gpu plutôt que le cpu. Dans la pipeline d'affichage OpenGL il y a deux shaders nécessaires : le **vertex shader** (*vs*) et le **fragment shader** (ou *pixel shader*) (*fs*).

![[pipeline.excalidraw.svg]]

#### Vertex Shader

Le vs est exécuté pour chaque sommet à afficher, il est responsable du placement du sommet sur l'écran.

> Par défaut les coordonnées de l'écran vont de (-1,-1,-1) à (+1,+1,+1) en xy et z, z étant la profondeur quand on affiche des éléments en 3D.

Pour afficher notre triangle il nous suffit de dire que la position à l'écran de chaque sommet est sa position d'origine. Le fs aura besoin de la couleur, le vs doit donc aussi la lui transmètre puisqu'elle est définie au niveau des sommets.

```glsl
#version 330 core

layout(location = 0) in vec3 i_position;
layout(location = 1) in vec4 i_color;

out vec4 o_color;

void main()
{
  o_color = i_color;
  gl_Position = vec4(i_position, +1.0);
}
```

> [!note] Notes
> - `location=x` fait référence au premier argument de `glVertexAttribPointer`.
> - `gl_Position` est la position du vertex à l'écran, sa 4ème composante est toujours 1.
> - Quand le fs lira la valeur de `o_color`, elle sera interpolé linéairement entre les valeurs des différents vertices. On aura donc un dégradé très facilement.

#### Fragment Shader

Le fs est exécuté pour chaque pixel d'une primitive, ici sur chaque pixel de notre triangle. Il est responsable de la coloration du pixel.

```glsl
#version 330 core

in vec4 o_color;

out vec4 color;

void main()
{
  color = o_color;
}
```

### Afficher un triangle

Maintenant que nous avons - les sommets sur le gpu, l'ordre des sommets sur le gpu, un vertex shader qui sait placer les sommets à l'écran, un fragment shader qui colore les pixels - nous pouvons enfin afficher un triangle !

TODO screen/code

## Les pistes d'amélioration

Pour éviter d'avoir à utiliser 15 fonctions à chaque affichage de triangle on peut créer des classes `VertexBuffer`, `IndexBuffer`, `VertexBufferLayout`, `Shader` qui le feront pour nous.

OpenGL propose aussi d'utiliser les **Vertex Array Objects** (*vao*), qui rassemblent un layout, un vbo et un ibo. Un vao ne contient pas de données mais faire `glBindVertexArray` revient à *bind* son ibo, vbo et réexécuter les fonctions `glVertexAttribPointer`.

A partir de ce code on peut afficher plusieurs triangles, pour faire des rectangles etc...

Pour l'instant le triangle est statique, on peut le déplacer en rajoutant un `uniform` au shader. Un *uniform* est une variable qui peut être modifiée entre appels à `glDrawElements`. On utilisera les uniforms pour spécifier une caméra plus tard.