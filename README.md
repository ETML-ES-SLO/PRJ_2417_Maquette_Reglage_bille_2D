## versionning :
### IDE :
  MPLABX IDE (v6.15)
### Configurateur graphique :
  harmony (v2.06)
### Compilateur :
  XC32 (v2.50)

## Caractéristique du projet :
### Emplacement :
+ code (liée à la session LFO) :
  + C:\microchip\harmony\v2_06\apps\PROJ\2417_B_MaquetteBille2D
+ projet en général :
  + K:\ES\PROJETS\SLO\2417_MaquetteBille2D\2417B_POBJ

## Explications sur les fonction implémenté :
### détection de la bille sur le plateau (fonctionnel):
le TSC (le driver qui s'occupe de lire le plateau) doit être configurer d'une certaine manière. Les registres PD1 et PD0 sont tous les deux mis à "0" pour utiliser le mode de "lecture constante" ce qui permet d'utiliser la fonction de la pin PENIRQ. 

Celle-ci est dédiée à la détection de bille, dans ce mode quand la bille est détectée sur le plateau elle reste à 0, par contre quand il n'y a pas de bille alors elle est dans un état un peux indéterminée (aléatoire). Pour contrer ce problème on utilise la libraire Mc32Debounce donnée en MINF pour faire de l'anti-rebond sur le signal afin de valider après 3 états bas que la bille est réellement présente.

Lorsque la bille est détectée la régulation est activer (avec un simple if()), et si la bille est absente alors la condition du If() n'est plus respecter et donc on arrête de réguler et on recentre les plateaux.

Voici le structogramme :
![Structogramme de l'App_Task d'origine lorsque j'ai récupéré le projet](/doc/Structogrammes/ReadOnly/APP_Task_V2-0.png)

### Gestion des Boutons (création de drivers complèts pour prochain User) :
Cette librairie est fortement inspirer du travail que l'on as dû faire en MINF pour le TP3_MenuGen. J'y reprend tout le concepte utilisée pour le PEC12 (la molette rotative) et l'ai adaptée pour l'utiliser sur n'importe quelle boutons (à condition de pointer sur les bons ports). J'ai déjà tout préparer pour qu'elle pointe aux bons ports , il ne reste plus qu'à l'utilisée. 

Pour ce faire il faut appeller de manière périodique la fonction ScanBtn() et lui donner comme paramètre d'entrée les boutons que l'on vas lire (dans notre cas je l'ai fait dans le callback du timer qui cadence l'app_task et déjà avec les bons paramètres d'entrée), pour simplifier l'utilisation de cette librairie j'ai fait un premier groupe de macros correspondant aux appels de fonctions PLIB pour la lecture des ports des bontons voulus (BTN_UP_RAW(),BTN_DOWN_RAW(),BTN_LEFT_RAW(),BTN_RIGHT_RAW()), ainsi qu'un deuxième groupe de macros mais cette fois-ci correspondant aux boutons nettoyer et utilisables pour la lib qu'il faudra appeller dans les différents fonctions pour les utilisées (BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT)

voici un exemple d'utilisation :
+ Imaginons que vous voulez tester l'inactivité sur le bouton du haut, alors dans ce cas il vous suffit de faire comme ceci :
  + if( BtnNoActivity(BTN_UP)) { //votre code à éxécuter si le bouton est inactif } else { //votre code à éxécuter si le bouton est actif }.
