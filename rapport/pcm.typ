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

= Objectifs

= Structure du programme

== Schéma block

= Analyse des résultats


== Avec condition de fin sure 

== Sans condition de fin sure


= Conclusion
