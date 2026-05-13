voila g fait un README.

sinon la version utilisée de MPLABX IDE est la v6.15 et pour harmony c'est la v2.06
## travaille fait le 25.02.26 :
+ voici ce que j'ai analysé pour la prochaine foit : 
  + la PIn PENIRQ peut être disable selon la configue de PD1 et PD0, se sont des registres dans lesquelles on écrit pas commande SPI dans le TSC.
  + je cherche actuellement quelle valeurs il envoie dans ces registres voir si il est disable ou enable. Car quand je mesure à l'oscillo sur cette pin PENIRQ ça n'arrète pas d'oscillée et c'est handicapant pour faire une détection unique de la présence de la bille.
  + ensuite niveau doc datasheet, les infos pour ces différents registres se trouvent aux pages 16, 15 et 10. 
  + Il faut aussi que je trouve si il lis juste le X+ et le Y+ ou si il lis toutes les entrées (donc en mode lecture simple ou différentielle ). 
  > après analyse g trouver qu'il lisait en mode différentielle.
  + j'ai enfin trouver dans les fichiers du code à peux près où il génère les codes
  > c'est dans le Mc32GestSpiTSC2046.c et dans le .h
> [!WARNING]
> attention je crois pas qu'il initialise le TSC au début et je sais pas si c'est nécessaire.

## Update du 04.03.26 :
auj g eu la présentation de mon avancement, ça c'est bien passer en soit. 

Sinon plus interessant j'ai fait des tests : 
1. quand on envoyait par SPI  PD1 = 1 et PD0 = 1
   + la pin PENIRQ était désactivée et donc faisait n'importe quoi autant quand la bille était sur le plateau que quand y'avait rien (elle avant prèsque la même allure que le signal CS)
     ce qui était gènant pour la détection de bille.
2. En envoyant au SPI le PD1 = 0 et PD0 = 0
   + et là miracle sa marche mieux ! en gros la pin PENIRQ dans ce mode vas rester constament à l'état 0 tant que la bille est sur le plateau et dès qu'on l'enlève elle repasse en état d'oscillation comme avant.
     > mais cela me permet de détecter la présence de la bille en attendant un peux de voir si la pin PENIRQ est restée à 0 assez longtemps pour justifier la présence de la bille.
> [!NOTE]
> peut être utiliser un input capture g vu il y'en avait 1 de dispo pour cette pin (même si m.Bovey m'a dit de faire par polling donc sans interuption).

## Update du 11.03.26 :
aujourd'hui j'ai que réussi à faire le structogramme de l'APP_Task , mais il est fini et au propre. Il est stocker ici :  K:\ES\PROJETS\SLO\2417_MaquetteBille2D\2417B_POBJ\doc

le voici : 
![Structogramme de l'App_Task d'origine lorsque j'ai récupéré le projet](/doc/Structogrammes/ReadOnly/APP_Task.png)

## Update du 18.03.26 :
aujourd'hui j'ai fait un bout du structogramme pour la détection de bille. J'ai aussi fait un bout de modif du code pour cette détection, il suffit juste d'implémenter le code et tester.

## Update du 25.03.26 :
aujourd'hui j'ai mis un peux au propre le readme pour que ça soit plus agréable à lire. je vais aussi tester le code voir si l'idée fonctionne.

## Update du 01.04.26 :
aujourd'hui j'ai tester le code et ça ne fonctionnait pas comme prévu alors j'ai fait 2 ou 3 ajustements et maintenant la détection de bille fonctionne et le structogramme est déjà à jour. Cependant quand on modifie les gains Kp ou autre le système ne régule plus, je ne sais pas si c'est liée à ma détection de bille ou si c'est un problème récurent de l'ancienne version qui doit être règler aussi.

## Update du 22.04.26 :
j'ai fait la présentation de ma détection de bille terminée. ensuite j'ai fait le structogramme du callback liée au timer 1 pour terminer la doc des modifications liée à la détection de bille. Ensuite j'ai commencer à faire l'issu de l'affichage en mettant en place un message de bienvenu et un message indiquant la présence de la bille.

sinon plus interessant j'ai peut être trouver pourquoi la carte crash quand on modifie les paramètres PID, c'est sûrement lier au atof() qui se trouve dans la fonction menuGestionBouton() dans GestMenu.c

## Update du 29.04.26 :
j'ai fait la partie affichage du mode remote ou locale pour les settings, j'ai pas encore tester le code pour voir si ça marche car il redessine tellement souvent la page que c'est dure de trouver un endroit où il efface pas tout l'écran, c'est vraiment quelquechose qu'il faut que je fasse pour la prochaine fois c'est de m'occupper de cet affichage car sinon on vas aller droit dans le mur vu comment c'est fait actuellement.

J'ai égaleent mis à jour mes structogrammes par rapport aux mssage de bienvenu et 2 ou 3 correction que j'avais fais dans le code mais pas dans le structo.

## Update du 06.05.26 :
auj on m'a dis de m'occupper de l'affichage et du menu, faire 1 nouveau fichier .c et .h pour le menu et si g du temps pour les boutons. Le but c'est de refaire un affichage propre en partant d'un truc déjà aproximativement fonctionnel , donc on garde quand même les anciens fichiers de menu pour avoir une backup fonctionnel.  J'ai donc décider de reprendre mes fichiers que j'avais fais pour le TP3 de MINF (gestBtn et MenuGen) et les modifiers pour les adaptées à ce code.

## Update du 13.05.26 :
auj j'ai enfin fini ma librairie universelle pour les boutons (alors que c'était seulement si j'avais du temps ;-;) du coup j'ai enfin commencer pour de vrais à faire le menu , j'ai bien commencer la nouvelle structure mais y'aura sûrement des paramètres qui vont s'ajouter au fil du temps, j'ai également déjà mis à jour la gestion de l'inactivité.

par contre j'ai fait aucun structogramme donc je vais essayer d'avancer ce menu du mieux que je peux pendant mes pauses histoire d'avoir un truc à montrer aux profs mais les prochaines fois faudra je mette la gomme sur la documentation.
