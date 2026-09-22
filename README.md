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

### Gestion des Boutons :
Une librairie `GestBtn (.c et .h)`a été développée, celle-ci n'est pas encore été implémentée dans le code actuel, puis testée et validée. Cette librairie est reprise d'un ancien TP de MINF où il s'agissait de gérér un menu en fonction d'un encodaeur rotatif - cette librairie a été modifiée pour s'adapter à n'importe quels switchs. 

#### Fonctions développées
```C
//-- nom fct :                    --> ScanBtn <--  
//-- paramètre entrée :             - bool ValSW0, 
//                                  - bool ValSW1, 
//                                  - bool ValSW2, 
//                                  - bool ValSW3 
//-- paramètre sortie :             void 
//-- paramètre référence (IN-OUT) : aucun 

//-- description : Fonction principale à appeler cycliquement. Elle gère l'anti-rebond des 4 boutons puis met à jour leurs états internes via GestBtn. 

//-- aide - référence - lien : Nécessite l'appel de DoDebounce (Mc32Debounce.h) 
//-- implémenté - testé - validé : non - non - non 
```
```C
//-- nom fct :                  --> GestBtn <--
//-- paramètre entrée :             aucun 
//-- paramètre sortie :             void  
//-- paramètre référence (IN-OUT) : - S_Btn_Descriptor* ptrSW,      
//                                  - S_SwitchDescriptor* - ptrDescrSW 

//-- description : Évalue l'état d'un bouton spécifique. 
//                 Incrémente les compteurs de pression, détermine si l'appui est simple (SEL) ou long (HOLD) au relâchement, et gère le délai de déclenchement de l'inactivité.
//-- aide - référence - lien : Utilise les macros PRESS_DURATION et MAX_INACTIVITY_DELAY 
//-- implémenté - testé - validé : non - non - non 
```
```C
//-- nom fct :                  --> BtnInit <--  
//-- paramètre entrée :             aucun 
//-- paramètre sortie :             void 
//-- paramètre référence (IN-OUT) : aucun 

//-- description : Initialise à zéro les structures matérielles (anti-rebond) et logicielles (SEL, HOLD, compteurs) des 4 boutons.
//-- aide - référence - lien : Doit être appelée à l'initialisation du système. 
//-- implémenté - testé - validé : non - non - non 
```
```C
//-- nom fct :                    --> BtnIsSEL <--  
//-- paramètre entrée :               aucun 
//-- paramètre sortie :               bool État du flag SEL 
//-- paramètre référence (IN-OUT) :   S_Btn_Descriptor* pBtn 

//-- description : Retourne true si une action d'appui simple (SEL) a été détectée sur le bouton passé en paramètre.
//-- démonstration : N/A 
//-- aide - référence - lien : N/A 
//----------------------------------------------------------------------------------//
```




Pour ce faire il faut appeller de manière périodique la fonction ScanBtn() et lui donner comme paramètre d'entrée les boutons que l'on vas lire (dans notre cas je l'ai fait dans le callback du timer qui cadence l'app_task et déjà avec les bons paramètres d'entrée), pour simplifier l'utilisation de cette librairie j'ai fait un premier groupe de macros correspondant aux appels de fonctions PLIB pour la lecture des ports des bontons voulus (BTN_UP_RAW(),BTN_DOWN_RAW(),BTN_LEFT_RAW(),BTN_RIGHT_RAW()), ainsi qu'un deuxième groupe de macros mais cette fois-ci correspondant aux boutons nettoyer et utilisables pour la lib qu'il faudra appeller dans les différents fonctions pour les utilisées (BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT)

voici un exemple d'utilisation :
+ Imaginons que vous voulez tester l'inactivité sur le bouton du haut, alors dans ce cas il vous suffit de faire comme ceci :
  + if( BtnNoActivity(BTN_UP)) { //votre code à éxécuter si le bouton est inactif } else { //votre code à éxécuter si le bouton est actif }.
