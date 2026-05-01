# tldr
`./generate_appendix.sh include_files.txt > appendix.tex` 
transformerer `include_files.txt` til en `appendix.tex` som er en drop-in-replacement for vår foreløpig appendix.
Man kan også skrive for å følgende kommando for å få output direkte til clipboard.
`./generate_appendix.sh include_files.txt | wl-copy` 

# Beskrivelse av syntax
`./include_files.txt` Beskriver hva vi ønsker å få generert.
Linjer med `//` er kommentarer, de blir altså ignorert.
`#<noe>` transformeres til `\section{<noe>}`
`##<noe>` transformeres til `\subsection{<noe>}`
`###<noe>` transformeres til `\subsubsection{<noe>}`
en linje slik som `src/lib/gdt.c` vil bli transformert til en latex kodelisting
med caption `implementation of gdt.c>` og med label `lst:gdt.c`

Skriptet leter etter filer startene fra `<git-root>/src/69_kjebbe/`.
Så når man skriver `src/lib/gdt.c` blir det faktisk letet etter
`<git-root>/src/69_kjebbe/src/lib/gdt.c`
