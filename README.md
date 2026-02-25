voila g fait un README.

sinon la version utilisée de MPLABX IDE est la v6.15 et pour harmony c'est la v2.06

voici ce que j'ai analysé pour la prochaine foit : 
  la PIn PENIRQ peut être disable selon la configue de PD1 et PD0, se sont des registres dans lesquelles on écrit pas commande SPI dans le TSC. je cherche actuellement quelle valeurs il envoie dans ces registres voir si il est disable ou enable. Car quand je mesure à     
  l'oscillo sur cette pin PENIRQ ça n'arrète pas d'oscillée et c'est handicapant pour faire une détection unique de la présence de la bille . 

  j'ai enfin trouver dans les fichiers du code à peux près où il génère les codes ( c'est dans le Mc32GestSpiTSC2046.c et dans le .h , !! attention je crois pas qu'il initialise le TSC au début et je sais pas si c'est nécessaire !! ). 

  ensuite niveau doc datasheet, les infos pour ces différents registres se trouvent aux pages 16, 15 et 10. 

  Il faut aussi que je trouve si il lis juste le X+ et le Y+ ou si il lis toutes les entrées (donc en mode lecture simple ou différentielle ). 
      -> après analyse g trouver qu'il lisait en mode différentielle.
