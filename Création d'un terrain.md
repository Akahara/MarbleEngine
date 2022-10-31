# Création d'un terrain

Dans cette page, nous allons aborder les différentes étapes que nous avons mises en oeuvre pour obtenir un terrain en 3 dimensions.


-------------------------------------------------------------------------------

## Le concept

Dans l'idée, on souhaite avoir un terrain généré procéduralement, c'est à dire que nous laissons au programme le soin de produire des formes aléatoires.

Pour se faire, la première étape est de générer une carte de hauteur, appelé **heightmap**, qui est simplement une image en 2 dimensions, dont la valeur des pixels représente la hauteur de l'endroit sur le terrain.

En effet, disons que nous ayons implémenté une méthode pour générer une grille de *vertices* de N^2^ points. Pour chaque point *P(i,j)* de cette grille, on regarde dans notre heightmap la valeur de la couleur du pixel en position *(i,j)*. Cette valeur, comprise entre 0 et 1, multipliée par un attribut **terrainHeight**, est ensuite passée comme coordonnée *y* du point. 

La majorité du travail réside donc dans la génération de cette fameuse **heightmap**, pour qu'elle ait des valeurs lisses, aléatoires et qu'elle donne un terrain réaliste.

-------------------------------------------------------------------------------
## Le *perlin noise*, arme omniprésente

Comment générer une image aléatoire correcte ? 

L'approche naive est de dire que pour chaque pixel de l'image, on choisi une valeur **val** comprise entre 0 et 1, puis on écrit que `pixel = (val, val, val)`.
Cette idée marche pour obtenir du ==véritable bruit== : 

![[Pasted image 20221030225943.png]]

Un problème se pose. Si l'on applique ce bruit à la grille de points dans l'espace, on risque d'obtenir un résultat proche de ceci :

![[Pasted image 20221030230138.png]]

Le problème est que deux points proches ont des valeurs très différentes ! Il nous faut donc un moyen de générer du ==pseudo-bruit==, pour lequel deux points proches ont des valeurs similaires, mais deux points éloignés sont radicalement différents.

Pour répondre à ce problème, l'industrie du jeux-vidéo utilise en grande majorité le ==bruit de Perlin==.
Ce bruit a l'avantage d'être très organique, lisse et convient parfaitement pour la génération de terrain, de feu ou encore de nuage. 

> [!info]
>
> Le bruit de Perlin est inventé par Ken Perlin en 1985, alors qu'il travaillait sur les effets spéciaux de Tron. Il n'appréciait pas le style trop "mécanique" des effets existant de l'époque.

Voici à quoi ressemble du bruit de Perlin :
![[Pasted image 20221030231016.png]]

### Méthode de génération de bruit de perlin

Pour concevoir une image comme celle-ci, on découpe d'abord notre image en une grille carrée, de résolution de notre choix. Plus la résolution est grande, puis l'image sera nette.

On génère généralement 8 vecteurs 2D, dont les directions sont souvents les 8 directions que l'on connait bien.

A chaque intersection / point de cette grille, on choisit au hasard un vecteur parmi notre liste de 8.

Ensuite, pour chaque pixel que contient chaque case de la grille, on interpole la coordonnée x et y dans le systeme de la case : c'est à dire que si une case possède 4 pixels par coté, chaque pixel aura un multiple de 1/4 = 0.25 pour ses coordonnées.
On génère 4 vecteurs à ce pixel qui correspondent à la distance entre les coins de la case qui l'encadre.  On effectue un produit scalaire de chaque vecteur avec celui du coin correspondant, puis on fait un interpolation des valeurs.

> [!Utile]
> Cette vidéo de *Fataho* explique très bien le fonctionnement du bruit de Perlin.
> https://www.youtube.com/watch?v=MJ3bvCkHJtE&

-------------------------------------------------------------------------------
## Génération de la heightmap

Malheureusement, appliquer un simple bruit de perlin donne des terrains trop lisses, et trop peu réalistes.
![[Pasted image 20221031091748.png]]

Le terrain manque cruellement de détails, il est beaucoup trop uniforme.
Une astuce existe : additioner plusieurs bruits de Perlin. 
Cette astuce peut s'apparenter à la décomposition d'un signal en somme de cosinus et sinus grâce à la transformée de Fourier. On applique à notre **heightmap** une somme de bruit dont la fréquence augmente et dont l'intesité diminue.

En augmentant la fréquence, les haut et les bas sont plus présents, comme si on "compressait" le bruit. Et pour éviter de rendre le bruit aléatoire comme nous avons vu plus haut, on diminue l'intesité, c'est à dire à quel point ce nouveau bruit aura de l'effet sur l'ancien. On peut voir ca comme un applatissement de la hauteur du nouveau bruit.

> [!info]
> On parle de Fractale de Surface en utilisant le FBM : 
>https://en.wikipedia.org/wiki/Fractional_Brownian_motion

Nous allons concevoir cette fonction, dont le but est de générer un tableau de flottants compris entre 0 et 1, et qui repose sur l'addition des bruits de Perlin.

Quelques paramètres sont importants :

- *width* et *height* : choisir la zone d'où on récupère les valeurs de bruit. Dans notre cas, nous n'avons pas implémenté le bruit, et utilisons une librairie annexe : `siv::PerlinNoise`
- *scale* : permet de changer l'ordre de "zoomer" sur la zone dont le bruit proviendra
- *octaves* : représente le nombre de fois que l'on va ajouter un nouveau bruit.
- *persistence*  : un facteur entre 0 et 1. L'intensité de chaque nouvel octave est multplié par ce facteur.
- *lacunarity* : La fréquence de chaque octave est multiplié par lacunarity. Plus ce paramètre est grand, plus le terrain aura une apparence "piquante", puisque la fréquence augmente très vite.
- *seed* : permet de choisir un nouveau bruit de perlin aléatoire.

Notre implémentation est la suivante :


```
Fonction generateNoiseMap(w : naturel,
						  h : naturel,
						  scale : flottant,
						  octaves : naturel,
						  persistance : flottant,
						  lacunarity : flottant
						  seed : naturel) : Flottant

Debut :
---------

noiseMap : Flottant[w * h]
perlin   : siv::PerlinNoise{seed}

Pour y allant de 0 à h:
	Pour x allant de 0 à w:
		
		amplitude = 1 : Flottant
		frequency = 1 : Flottant
		noiseValeur = 0 : Flottant
		
		Pour o allant de 0 à octaves:
		
			sampleX, sampleY : Flottant
			
			sampleX = x / scale * frequency
			sampleY = y / scale * frequency
			
			pValeur = perlin.bruit(sampleX, sampleY)
			noiseValeur += pValeur*amplitude
			
			amplitude *= persistance
			frequency *= lacunarity
		
		Fin Pour
		
		noiseMap[y * w + x] = noiseHeight
	Fin Pour
Fin Pour

// Normaliser les valeurs entre 0 et 1 à l'aide d'un minimum et d'un maximum que l'on calcule lors de la génération

retourner noiseMap

Fin
------------

```

Et voila, le tour est joué !

**![](https://lh6.googleusercontent.com/Ott44CSVKgXsJVaYozGFtOKNkg7cYRTGBLpqD_983AZO3_OKCVciRwszkRW91N7P1PFZ2D5o0Uxoa9do1Xa4HTPSnPaDzRBCppOV8NQAJEFJiMz_cFo2MVeTJviq8ZRLLqBu2TGHkZko10ztR0I-KzDWirwJwN_AlAEWf1Hqdrv36IZW3KvwaS4zvA)**

On peut facilement jouer sur les valeurs, pour obtenir un terrain qui nous plait en temps réel.

-------------------------------------------------------------------------------

## L'érosion


Can we get much higher ? So hiiiiiiiiiiiiiigh

WIP c'est dur sa mère