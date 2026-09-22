## Déveoppeur 
> [MBR](https://github.com/MathieuBucher) -> version hardware : A - version firmware 1.0 - version sotware 1.0
>
> [LFO](https://github.com/VolutedPenny235) -> version soft 1.1

## versionning :

> hardware : version A 

> software : V1.0 -> V1.0 

## Outils de développmeent pour le firmware :

> IDE + configurateur graphique : MPLABX IDE (v6.15) + harmony (v2.06)

> compilateur : XC32 (v2.50)

### Emplacement obligatoire pour développment Firmware  

> `C:\microchip\harmony\v2_06\apps\PROJ\2417_B_MaquetteBille2D`

## Architecture Firwmare 
### Structogramme Version 1.0
![Structogramme version 1.0](/soft/Firmware/architecture/Structogrammes/ReadOnly/APP_Task.jpg)

### Structogramme Version 1.1
![Structogramme version 1.1](/soft/Firmware/architecture/Structogrammes/ReadOnly/APP_Task_V2-0.jpg)

## Explications sur les modifications implémentées :
### détection de la bille sur le plateau -> déployement fonctionnel :
le [TSC2046E](/doc/datasheets/tsc2046e.pdf) étant le driver qui s'occupe de lire le plateau, celui-ci doit être configuré d'une certaine manière. Les registres PD1 et PD0 sont tous les deux mis à "0" pour utiliser le mode de "lecture constante" ce qui permet d'utiliser la fonction de la pin PENIRQ -> voir la p16 du datasheet. 

La pin **_PINIRG** (active bas) est dédiée à la détection de bille ; dans ce mode quand la bille est détectée sur le plateau elle reste à 0, par contre quand il n'y a pas de bille cette pin est dans un état indéterminée (aléatoire). Pour contrer, utilisation la libraire `Mc32Debounce (.c et .h)` qui est une étection d'anti-rebond développé à l'ES et utilisée dans différents TPs-Exercices donné en MicroInformatique (***MINF***). Pourquoi utiliser cette librairie, pour s'assurer de la détection de la bille. 

Lorsque la bille est détectée la régulation est activée 

```
Si bille detectee 
  regulation activee 
Sinon 
  regulation stopper 
  ajustement du plateau 
``` 

### Gestion des Boutons (création de drivers complèts pour prochain User) :
Cette librairie est fortement inspirer du travail que l'on as dû faire en MINF pour le TP3_MenuGen. J'y reprend tout le concepte utilisée pour le PEC12 (la molette rotative) et l'ai adaptée pour l'utiliser sur n'importe quelle boutons (à condition de pointer sur les bons ports). J'ai déjà tout préparer pour qu'elle pointe aux bons ports , il ne reste plus qu'à l'utilisée. 

Pour ce faire il faut appeller de manière périodique la fonction ScanBtn() et lui donner comme paramètre d'entrée les boutons que l'on vas lire (dans notre cas je l'ai fait dans le callback du timer qui cadence l'app_task et déjà avec les bons paramètres d'entrée), pour simplifier l'utilisation de cette librairie j'ai fait un premier groupe de macros correspondant aux appels de fonctions PLIB pour la lecture des ports des bontons voulus (BTN_UP_RAW(),BTN_DOWN_RAW(),BTN_LEFT_RAW(),BTN_RIGHT_RAW()), ainsi qu'un deuxième groupe de macros mais cette fois-ci correspondant aux boutons nettoyer et utilisables pour la lib qu'il faudra appeller dans les différents fonctions pour les utilisées (BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT)

voici un exemple d'utilisation :
+ Imaginons que vous voulez tester l'inactivité sur le bouton du haut, alors dans ce cas il vous suffit de faire comme ceci :
  + if( BtnNoActivity(BTN_UP)) { //votre code à éxécuter si le bouton est inactif } else { //votre code à éxécuter si le bouton est actif }.
