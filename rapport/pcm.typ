#import "@preview/bubble:0.2.1": *
#import "@preview/codelst:2.0.1": sourcecode

#show: bubble.with(
  title: "PCM",
  subtitle: "Projet Final",
  author: "Loïc Frossard & Baptiste Dupertuis",
  affiliation: "HES-SO Master",
  date: datetime.today().display(),
  year: "2024-2025",
  class: "Rapport",
  main-color: "4DA6FF", //set the main color
  logo: image("./logo.jpg"), //set the logo
) 

#show raw.where(block: true): it => [
  #block(fill: luma(245), inset: 10pt, radius: 4pt, width: 100%)[
    #it
  ]
]

// Edit this content to your liking
#show figure.where(
  kind: image
): set figure.caption(position: bottom)

#set heading(numbering: "1.1.1.1.1.1.1")

#show outline.entry.where(
  level: 1
): it => {
  v(12pt, weak: true)
  strong(it)
}
 
#outline(indent: auto)
#pagebreak()

#let ImageSize = 70%

= Introduction
Ce rapport présente le projet final du cours de Programmation Concurrente et Multicœur (PCM). Le projet réalisé est capable de résoudre le Problème du voyageur de commerce (TSP, Traveling Salesman Problem) pour 19 villes en environ 13 minutes sur 10 threads. 

== Objectifs
L'objectif de ce projet est de développer un programme qui profite d'une architecture multi-cœur, en utilisant les techniques définies durant le cours de PCM.

Le travail consiste à résoudre le Problème du voyageur de commerce (TSP, Traveling Salesman Problem) avec une méthode branch-and-bound. Le programme doit visiter N villes une seule fois et doit revenir à la ville de départ, en faisant le chemin le plus court. Étant donné que le chemin est circulaire, le choix de la ville de départ n'a pas d'importance. Ceci est un problème difficile d'optimisation combinatoire, car s'il y a N villes alors, au départ d'une ville donnée, il y a (N-1)! circuits différents qui passent par les N-1 autres villes et reviennent à la ville de départ. Il est supposé qu'il y a un chemin indépendant entre toute paire de villes.


= Structure du programme

Le programme se base sur l'utilisation d'une méthode branch-and-bound pour résoudre le problème du voyageur de commerce. La méthode branch-and-bound est une technique de résolution de problèmes d'optimisation combinatoire. Elle consiste à diviser le problème en sous-problèmes plus petits, à évaluer ces sous-problèmes et à éliminer les branches qui ne peuvent pas contenir la solution optimale.

Pour ce faire, le programme utilise une approche parallèle, en divisant le problème en plusieurs sous-problèmes qui sont résolus en parallèle. Lorsqu'un problème est résolu, le programme vérifie si la solution trouvée est meilleure que la meilleure solution actuelle. Si c'est le cas, la meilleure solution est mise à jour. Les autres sous-problèmes sont ensuite évalués en fonction de la meilleure solution actuelle, afin d'éliminer les branches qui ne peuvent pas contenir la solution optimale. Ce processus est répété jusqu'à ce que tous les sous-problèmes soient résolus.

Le programme est écrit en C++ et utilise la bibliothèque atomic pour gérer les accès concurrents aux données partagées. Il utilise également la bibliothèque thread pour créer et gérer les threads. Le programme prend en entrée un fichier contenant les coordonnées des villes à visiter et affiche en sortie le chemin optimal ainsi que la distance totale parcourue.

La communication des problèmes entre les threads se fait à l'aide d'une queue. Cette queue n'utilise pas de lock pour garantir l'accès exclusif aux données partagées, mais utilise des opérations atomiques pour garantir la cohérence des données. L'implémentation de cette queue se base sur celle fournie dans le cours de PCM.

L'indication de la meilleure solution est faite à l'aide d'une variable partagée, cette variable est mise à jour de manière atomique pour garantir la cohérence des données.

#pagebreak()
== Schéma block
Le schéma suivant illustre le fonctionnement du programme. Ici la condition de fin pour un thread est la suivante : si la queue est vide, le thread s'arrête. Cette condition de fin n'est pas sure, car il est possible que d'autres threads ajoutent des éléments à la queue après que le thread ait vérifié qu'elle était vide. Cela peut entraîner une situation où un thread s'arrête alors qu'il reste des éléments à traiter. Cependant, dans la pratique, cette situation ne s'est pas produite lors de nos tests. Nous avons aussi analyser un programme avec une condition de fin sure et analyser l'impact sur les performances. La condiftion de fin sure se base sur la comptage des chemins traités.

#figure(
  image("./images/PCM-empty-queue.drawio.png"),
  caption: "Schéma de fonctionnement du programme"
)

Les cases blues représente les actions qui interagissent avec la queue, les cases vertes représentent les actions qui interagissent avec la variable de la meilleure solution en écriture.

Voici les points clés du schéma :
- Le programme commence par initialiser la queue avec les deux ou trois premiers niveaux de l'arbre de recherche. Cela permet de cérer assez de travail pour tous les threads et d'éviter un démarage lent.
- Chaque thread récupère un sous-problème de la queue, si le sous-problème est suffisanment petit (il contient MAX_DEPTH villes), le thread le résoud et met à jour la meilleure solution si il en trouve une. Si le problème est trop grand, le thread crée le prochain niveau de l'arbre de recherche et ajoute les sous-problèmes à la queue.
- Lorsqu'un thread a terminé de traiter un sous-problème, il en récupère un autre de la queue. Dans la version 1 du code, si la queue est vide, le thread s'arrête. Dans la version 2, le thread s'arrête uniquement si le nombre de sous-problèmes traités est égal au nombre total de sous-problèmes à traiter.

= Analyse des résultats


== Avec condition de fin sure 

=== Temps d'exécution

=== Speedup 

=== Efficacité

=== Variance sur le temps d'exécution des threads

== Sans condition de fin sure

=== Temps d'exécution

=== Speedup 

=== Efficacité

=== Variance sur le temps d'exécution des threads

== Analyse de la concurrence sur la queue




= Conclusion
