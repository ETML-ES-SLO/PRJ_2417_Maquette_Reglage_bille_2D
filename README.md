voila g fait un README.

sinon la version utilisée de MPLABX IDE est la v6.15 et pour harmony c'est la v2.06

voici ce que j'ai analysé pour la prochaine foit : 
  la PIn PENIRQ peut être disable selon la configue de PD1 et PD0, se sont des registres dans lesquelles on écrit pas commande SPI dans le TSC. je cherche actuellement quelle valeurs il envoie dans ces registres voir si il est disable ou enable. Car quand je mesure à     
  l'oscillo sur cette pin PENIRQ ça n'arrète pas d'oscillée et c'est handicapant pour faire une détection unique de la présence de la bille . 

  j'ai enfin trouver dans les fichiers du code à peux près où il génère les codes ( c'est dans le Mc32GestSpiTSC2046.c et dans le .h , !! attention je crois pas qu'il initialise le TSC au début et je sais pas si c'est nécessaire !! ). 

  ensuite niveau doc datasheet, les infos pour ces différents registres se trouvent aux pages 16, 15 et 10. 

  Il faut aussi que je trouve si il lis juste le X+ et le Y+ ou si il lis toutes les entrées (donc en mode lecture simple ou différentielle ). 
      -> après analyse g trouver qu'il lisait en mode différentielle.

Update du 04.03.26 :

auj g eu la présentation de mon avancement, ça c'est bien passer en soit. Sinon plus interessant g fait des tests : quand on envoyait par SPI  PD1 = 1 et PD0 = 1 la pin PENIRQ était désactivée et donc faisait n'importe quoi autant quand la bille était sur le plateau que quand y'avait rien (elle avant prèsque la même allure que le signal CS ) ce qui était gènant pour la détection de bille. Cependant g tester en envoyant au SPI le PD1 = 0 et PD0 = 0 et là miracle sa marche mieux ! , genre en gros la pin PENIRQ dans ce mode vas rester constament à l'état 0 tant que la bille est sur le plateau et dès qu'on l'enlève elle repasse en état bancale comme avant. mais cela me permet de détecter la présence de la bille en attendant un peux de voir si la pin PENIRQ est restée à 0 assez longtemps pour justifier la présence de la bille. (peut être utiliser un input capture g vu il y'en avait 1 de dispo pour cette pin (même si m.Bovey m'a dit de faire par polling donc sans interuption) ).
