%% fichier de definition de la classe EPUProjetDi
%% auteur : Nicolas Monmarché
%% modif : 21/09/2021

\NeedsTeXFormat{LaTeX2e}

%----- Identification -----
\ProvidesClass{EPUProjetDi}[2021/09/21 - EPUProjetDi document class (NM)]

%----- Initial Code -----
\RequirePackage{fancyhdr}
\RequirePackage{ifthen}
\RequirePackage{ifpdf}            % pdfLaTeX ou LaTeX ?
\RequirePackage[\ifpdf pdftex\else dvips\fi]{graphicx}
\RequirePackage[utf8]{inputenc}
\RequirePackage[T1]{fontenc}
\RequirePackage{amsmath}
\RequirePackage{theorem}
\RequirePackage{alltt}
\RequirePackage{pifont}
\RequirePackage{makeidx}
%\RequirePackage{fancyvrb}
\RequirePackage{url}
%\RequirePackage{marvosym}
\RequirePackage{color}
\RequirePackage{calc} % pour les commandes \multiply, \divide...

%\RequirePackage{lastpage}

%----- Declaration of Options -----

\DeclareOption*{\PassOptionsToClass{\CurrentOption}{book}}

%----- Execute of Options -----
\ProcessOptions

%----- Package Loading -----
\LoadClass[12pt,oneside]{book}
\RequirePackage[french]{babel}


%----- Main Code -----
\setlength{\evensidemargin}{2cm}
\setlength{\oddsidemargin}{2cm}
\setlength{\textheight}{24cm}
\setlength{\textwidth}{17cm} 
\setlength{\topmargin}{0.5cm}
\setlength{\headheight}{2cm}
\setlength{\headsep}{0.5cm}
\setlength{\voffset}{-1in}
\setlength{\hoffset}{-1in}
\setlength{\footskip}{1.5cm}
%\setlength{\marginparwidth}{0cm}
%\setlength{\parskip}{1em}
\setlength{\parskip}{1ex plus 0.5ex minus 0.2ex}

%pour ne pas appliquer le style francais d'espacement dans itemize et enumerate
%\FrenchItemizeSpacingfalse
%\FrenchListSpacingfalse

%pour la fin du rapport : ajout automatique de pages pour que le nb total soit
% un multiple de 4 (mas cela ne fonctionne pas parfaitement...
\newboolean{totalpagemultiplequatre}
\setboolean{totalpagemultiplequatre}{false}

\DeclareGraphicsExtensions{\ifpdf .jpg,.pdf,\else .eps,\fi .pdftex_t,.png,.gif}

%------------------------------------------------------------
%definitiond'une nouvelle longueur
\newlength{\largeurtexteentete}

%qui est fixée a la largeur du texte moins 2.8cm
\setlength{\largeurtexteentete}{\textwidth}
\addtolength{\largeurtexteentete}{-2.9cm}

%------------------------------------------------------------
%entete et pied de pages du package fancyhdr : les deux lignes suivantes semblent equivalentes
%\pagestyle{fancyplain}
\pagestyle{fancy}

%pour mettre a zero le style courant
\fancyhf{}


%on met le logo dans une boite
\newsavebox{\EPUlogo}
\sbox{\EPUlogo}{\includegraphics[width=2cm]{logo_Polytech_Tours_RVB_2cm_300dpi}}

%------------------------------------------------------------
%redéfinition des en-tête de pages
\renewcommand{\chaptermark}[1]%
{%
    \ifthenelse{\value{chapter} > 0}%
    {%
      \markboth%
      {%
	\sf\@shorttitle\\\hfill%
	\chaptername\ \thechapter. #1%
      }{}%
    }%else
    {%
      \markboth%
      {%
	\sf\@shorttitle\\\hfill%
	#1%
      }{}%
    }
}
  \renewcommand{\sectionmark}[1]{\markright{~\sf\thesection\ #1~}}

  %a gauche
  %syntaxe : \lhead[even-paire(verso)]{odd-paire(recto)}
  \lhead%
  [%
    %syntaxe : \fancyplain{page de titre}{page normale}
    \fancyplain{}{\sf\bfseries\thepage}%
  ]%
  {%
    \fancyplain{}%
    {%
      \begin{minipage}{2cm}\usebox{\EPUlogo}\end{minipage}%
      ~\begin{minipage}{\largeurtexteentete}~%
      {%
	\bfseries\rightmark%
      }%
      \end{minipage}%
    }%
  }%
	   
  %a droite	       
  \rhead%
  [%
    \fancyplain{}%
    {%
      \begin{minipage}{\largeurtexteentete}%
      {%
	\bfseries\raggedleft\leftmark%
      }%
      \end{minipage}%
     ~ \begin{minipage}{2cm}\usebox{\EPUlogo}\end{minipage}~%
    }%
  ]%
  {%
    \fancyplain{}%
    {%
      \sf\bfseries\thepage%
    }%
  }%

%au centre des pages de titre uniquement : on met le numero de page
\cfoot[\fancyplain{{-\bfseries\sf\thepage-}}{}]{\fancyplain{{-\bfseries\sf\thepage-}}{}}%

%------------------------------------------------------------

%pour ne pas redemmarrer la numerotation des chapitres a chaque nouvelle partie
\def\noresetatpart{\let\cl@part\empty}
\noresetatpart

%------------------------------------------------------------
%redefinition de la présentation des parties (inspiré de amsbook.cls)

\def\part{\cleardoublepage \thispagestyle{empty}%
  \null\vfil  \markboth{}{}\secdef\@part\@spart}
\def\@part[#1]#2{%
  \ifnum \c@secnumdepth >-2\relax \refstepcounter{part}%
    \addcontentsline{toc}{part}{\partname\ \thepart.
	\protect\enspace\protect\noindent#1}%
  \else
    \addcontentsline{toc}{part}{#1}\fi
  \begingroup\centering
  \ifnum \c@secnumdepth >-2\relax
       {\fontsize{\@xviipt}{22}\bfseries
	 \partname\ \Large\thepart} \vskip 20\p@ \fi
  \fontsize{\@xxpt}{25}\bfseries
      \Huge\textsf{#1}\vfil\vfil\endgroup \newpage\thispagestyle{empty}}


\def\@spart#1{\addcontentsline{toc}{part}{\protect\noindent#1}%
  \begingroup\centering
  \fontsize{\@xxpt}{25}\bfseries
     \Huge\textsf{#1}\vfil\vfil\endgroup \newpage\thispagestyle{empty}}
\def\partrunhead#1#2#3{%
  \@ifnotempty{#2}{\uppercase{\ignorespaces#1 #2\unskip}\@ifnotempty{#3}{. }}%
  \def\@tempa{#3}%
  \ifx\@empty\@tempa\else
    \begingroup \def\\{ \ignorespaces}% defend against questionable usage
    \uppercasenonmath\@tempa\@tempa
    \endgroup
  \fi
}
%------------------------------------------------------------
%redefinition de la présentation dest chapitres (inspiré de amsbook.cls)
%\renewcommand{\thechapter}{\roman{chapter}}
\def\@makechapterhead#1{%\global\topskip 7.5pc\relax
  \begingroup
  \fontsize{\@xivpt}{18}\bfseries\centering
    \ifnum\c@secnumdepth>\m@ne
      \leavevmode \hskip-\leftskip
      \rlap{\vbox to\z@{\vss
	  \centerline{\LARGE\mdseries
	      \uppercase\@xp{\chaptername}\enspace\thechapter}
	  \vskip 3pc}}\hskip\leftskip\fi
     \Huge\textsf{#1}\par\vspace{1cm}\hrule\vspace{1cm} \endgroup
  \skip@34\p@ \advance\skip@-\normalbaselineskip
  \vskip\skip@ }
  
\def\@makeschapterhead#1{%\global\topskip 7.5pc\relax
  \begingroup
  \fontsize{\@xivpt}{18}\bfseries\centering
  \Huge\textsf{#1}\par\vspace{1cm}\hrule\vspace{1cm} \endgroup
  \skip@34\p@ \advance\skip@-\normalbaselineskip
  \vskip\skip@ }

\def\appendix{\par
  \c@chapter\z@ \c@section\z@
  \let\chaptername\appendixname
  \def\thechapter{\@Alph\c@chapter}}

%------------------------------------------------------------

\AtEndDocument{\chaptermark{}
  %\enlargethispage{2cm}
\sectionmark{}\textsf{\small \vfill\begin{center}
Ce document a {\'e}t{\'e} format{\'e} selon le format EPUProjetDi.cls (N. Monmarch{\'e})\\~\\
{\'E}cole Polytechnique de l'Universit\'e de Tours\\
64 Avenue Jean Portalis, 37200 Tours, France\\
\url{http://www.polytech.univ-tours.fr}
\end{center}
}}

%---------------------------------------------------------------------------------
%l'argument #1 sera mis dans l'index sans changement de style
\newcommand{\indexnorm}[1]{#1\index{#1}}

%---------------------------------------------------------------------------------
%argument #1 en italique qui sera dans l'index
\newcommand{\indexit}[1]{\textit{#1}\index{#1}}

%---------------------------------------------------------------------------------
%l'argument #1 sera mis dans l'index
\newcommand{\indexrm}[1]{#1\index{#1}}

%---------------------------------------------------------------------------------
%l'argument #1 en gras sera mis dans l'index
\newcommand{\indexbf}[1]{\textbf{#1}\index{#1}}

%---------------------------------------------------------------------------------
%l'argument #1 en teletype sera mis dans l'index
\newcommand{\indextt}[1]{\texttt{#1}\index{#1}}

%---------------------------------------------------------------------------------
%argument #1 dans le style du langage C qui sera dans l'index
\newcommand{\indexC}[1]{\langc{#1}\index{#1@\langc{#1}}}

%---------------------------------------------------------------------------------
{
\theoremstyle{break}
\theorembodyfont{\rmfamily}
\newtheorem{exercice}{Exercice}[chapter]
}

%---------------------------------------------------------------------------------
{
\theoremstyle{break}
\theorembodyfont{\sffamily}

\newtheorem{exem}{Exemple}[chapter]
}

\newlength{\exemplewidth}
\newlength{\exemplelinewidth}

\newenvironment{exemple}[1][exe:nouveau]{%
\begin{exem}\label{#1} %remarque : la commande \label insere une ligne sinon
\setlength{\exemplewidth}{\linewidth}%
%\addtolength{\exemplewidth}{-0pt}
%\addtolength{\exemplewidth}{-\rightmargin}
\begin{minipage}[t]{\exemplewidth}%
\setlength{\exemplelinewidth}{\linewidth}%
\addtolength{\exemplelinewidth}{-7pt}%on enleve la largeur de la vrule et le hspace qui suit
\noindent{\vrule width 2pt}%
\hspace{5pt}%
\begin{minipage}[t]{\exemplelinewidth}%
}{%
\end{minipage}
\end{minipage}

\end{exem}}


%pour faire passer a la page suivante pour les exemples trop longs
\newcommand{\newpageexemple}{%
\end{minipage}
\end{minipage}
{\vrule width 2pt height 2pt}\\[-10pt]
{\vrule width 2pt height 2pt}\\[-8pt]
{\vrule width 2pt height 2pt}\\[-6pt]
{\vrule width 2pt height 2pt}\\[-4pt]
{\vrule width 2pt height 2pt}
\newpage%
\noindent\hspace{-\leftmargin}%pour compenser le fait que l'on soit toujours dans l'environnement exemple
\setlength{\exemplewidth}{\linewidth}%
%\addtolength{\exemplewidth}{-0pt}
%\addtolength{\exemplewidth}{-\rightmargin}
{\vrule width 2pt height 2pt}\\[-4pt]
{\vrule width 2pt height 2pt}\\[-6pt]
{\vrule width 2pt height 2pt}\\[-8pt]
{\vrule width 2pt height 2pt}\\[-10pt]
{\vrule width 2pt height 2pt}\\
\begin{minipage}[t]{\exemplewidth}%
\setlength{\exemplelinewidth}{\linewidth}%
\addtolength{\exemplelinewidth}{-7pt}%on enleve la largeur de la vrule et le hspace qui suit
\noindent{\vrule width 2pt}%
\hspace{5pt}%
\begin{minipage}{\exemplelinewidth}}

%---------------------------------------------------------------------------------
\newtheorem{theoreme}{Théorème}[chapter]

%---------------------------------------------------------------------------------
\newtheorem{definition}{Définition}[chapter]

%---------------------------------------------------------------------------------
\newtheorem{proposition}{Proposition}[chapter]

%---------------------------------------------------------------------------------
\newenvironment{demonstration}{\paragraph{Démonstration}\begin{small}}{\end{small}\par}
%---------------------------------------------------------------------------------
%page de titre

\renewcommand{\title}[2][titre court]{\def\@shorttitle{#1}\def\@title{#2}}
\newcommand{\projet}[1]{\def\@projet{#1}}
\newcommand{\encadrant}[1]{\def\@encadrant{#1}}

% Redefine \maketitle:
\renewcommand\maketitle{\par%
 \begingroup%
   \renewcommand\thefootnote{\@fnsymbol\c@footnote}%
   \def\@makefnmark{\rlap{\@textsuperscript{\normalfont\@thefnmark}}}%
   \long\def\@makefntext##1{\parindent 1em\noindent
           \hb@xt@1.8em{%
               \hss\@textsuperscript{\normalfont\@thefnmark}}##1}%
 \@maketitle%
 \global\@topnum\z@   % Prevents figures from going at top of page.
 \thispagestyle{empty}%
 \@thanks
 \endgroup
 \setcounter{footnote}{0}%
 \global\let\thanks\relax
 \global\let\maketitle\relax
 \global\let\@maketitle\relax
 \global\let\@thanks\@empty
 %\global\let\@author\@empty
 \global\let\@date\@empty
 %\global\let\@title\@empty
 %\global\let\@projet\relax
 %\global\let\@encadrant\relax
 \global\let\title\relax
 \global\let\author\relax
 \global\let\date\relax
 \global\let\and\relax
 \global\let\stage\relax
 \global\let\encadrant\relax
 \newpage\thispagestyle{empty}%
}
%
\def\@maketitle{%
  \newpage\vskip -1.6cm
  \vbox to 0.8cm {
    \let \footnote \thanks
    {     
     \noindent\begin{minipage}[h]{8cm}%
     \includegraphics[height=2.4cm]{logo_Polytech_Tours_RVB_3cm_300dpi}%
     \end{minipage}
     \hfill
     \begin{minipage}[h]{4cm}
     \begin{flushright}
     \includegraphics[height=2.3cm]{logo_univtours-horizontal.png}
     \end{flushright}
     \end{minipage}
     }
  }
  \begin{flushleft}
  {\small \textsc{\'Ecole Polytechnique de l'Université de Tours}}\\
  {\small 64, Avenue Jean Portalis}\\
  {\small 37200 TOURS, FRANCE}\\
  {\small T\'el. (33)2-47-36-14-14}\\
  {\small Fax  (33)2-47-36-14-22}\\
  {\small \url{www.polytech.univ-tours.fr}}\\
  \end{flushleft}
  \vspace{1cm}
  \begin{center}
  {
  {\normalfont\Large\bfseries\sffamily
  Rapport \par \@projet\par%
  }
  \vskip 1.5cm
  }
  \vbox to 3cm {\normalfont\huge\bfseries\sffamily%
  \@title\par%
  }
  \end{center}
  %\vskip 0.5cm\hrule\vskip 0.5cm
  \vskip 1.5cm
  \begin{minipage}[t]{8cm}
    \begin{flushleft}\noindent Auteur(s)\\\hrule\vskip 2mm
      {\normalfont\bfseries\sffamily \@author }
    \end{flushleft}
  \end{minipage}\hfill
  \begin{minipage}[t]{8cm}
    \begin{flushright}\noindent Encadrant(s)\\\hrule\vskip 2mm
      {\normalfont\bfseries\sffamily \@encadrant}
    \end{flushright}
  \end{minipage}
  %\par\enlargethispage{1cm}
  \vfill
  \hfill {\normalfont Version du \@date}%
}%
%

%---------------------------------------------------------------------------------
%commande à utiliser après les listes de figures, tables, etc. pour lancer le début de la numérotation des pages
\newcommand{\start}{\newpage\typeout{START}\newcounter{NbPageOfTheFirstPartOfTheReport}
\setcounter{NbPageOfTheFirstPartOfTheReport}{\value{page}}
\addtocounter{NbPageOfTheFirstPartOfTheReport}{1}
\message{>  NbPageOfTheFirstPartOfTheReport=\theNbPageOfTheFirstPartOfTheReport}
%\@@end%pour arréter la compilation
\setcounter{page}{1}
\pagenumbering{arabic}}
%---------------------------------------------------------------------------------
\newcommand{\resume}[1]{\def\@resume{#1}}
\newcommand{\motcles}[1]{\def\@motcles{#1}}
\newcommand{\abstract}[1]{\def\@abstract{#1}}
\newcommand{\keywords}[1]{\def\@keywords{#1}}

\newcommand{\makedernierepage}{%
\typeout{MAKEDERNIEREPAGE}
\newpage~
\ifthenelse{\boolean{totalpagemultiplequatre}}{
  \newcounter{A}
  \newcounter{B}
  \newcounter{C}
  \newcounter{D}
  \newcounter{E}
  \newcounter{F}
  \newcounter{G}
  % A = nb total de pages
  \setcounter{A}{\value{NbPageOfTheFirstPartOfTheReport}}
  \addtocounter{A}{\value{page}}
  \message{>  A=\theA}
  % B = A/4
  \setcounter{B}{\value{A}}
  \divide\value{B} by 4
  \message{>  B=\theB}
  % C = B*4
  \setcounter{C}{\value{B}}
  \multiply\value{C} by 4
  \message{>  C=\theC}
  % D = A-C
  \setcounter{D}{\value{A}-\value{C}}
  \message{>  D=\theD}
  % si D>0 Alors 
  \ifthenelse{\value{D} > 0}{%
    % E = B+1
    \setcounter{E}{\value{B}+1}
    % F = E*4
    \setcounter{F}{\value{E}*4}
    % G = F-A
    \setcounter{G}{\value{F}-\value{A}}
    %Ajouter G pages blanches
    \message{>  G=\theG}
    \addtocounter{G}{-1}
    \newif\ifcondstop
    \loop
        \newpage~\thispagestyle{empty}
        \typeout{[+1 page]}
        \addtocounter{G}{-1}
        \ifnum\value{G}>-1
    \repeat
    \newpage~\thispagestyle{empty}
  }%else
  {
    % rien
  }
}%else
{
    % rien
}

\begin{center}
\vbox to 1.8cm {\normalfont\Large\bfseries\sffamily\@title\par}
\end{center}
\hrule
\thispagestyle{empty}
\begin{center}
{\large Rapport : \@projet}%\\~\\
\end{center}
\paragraph{Résumé :} \@resume
\paragraph{Mots clé :} \@motcles
\paragraph{Abstract :} \@abstract
\paragraph{Keywords :} \@keywords
\vspace{1cm}
\par\vspace{0.5cm}
{\small\noindent\begin{minipage}[t]{8cm}
    \begin{flushleft}\noindent Auteur(s)\\\hrule\vskip 2mm
      {\normalfont\bfseries\sffamily \@author }
    \end{flushleft}
  \end{minipage}\hfill
  \begin{minipage}[t]{8cm}
    \begin{flushright}\noindent Encadrant(s)\\\hrule\vskip 2mm
      {\normalfont\bfseries\sffamily \@encadrant}
    \end{flushright}
  \end{minipage}
  }}


%%
% commandes pour le compte-rendu
\newcounter{compteseance}
\setcounter{compteseance}{1}
\newcommand{\CR}[2]{%
\begin{center}
\begin{tabular}{|p{3cm}|p{12cm}|}\hline
 Séance \no \arabic{compteseance} du #1
\addtocounter{compteseance}{1}
 & #2\\\hline
\end{tabular}
\end{center} }


\RequirePackage[pdftex=true,colorlinks=true,linkcolor=red,urlcolor=blue]{hyperref}
