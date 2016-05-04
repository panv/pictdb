# TODO

- [ ] GROS BUG : essayer de read une image non encore resizée fait crasher le programme;
      problème de free avec vips, probablement dans lazily resize.
- [x] Réparer read
- [x] Réparer get_resolution: le free (commenté) fait crasher le programme
- [x] Réparer dedup: insérer la même image 2 fois insère 2 images au lieu de dédupliquer
- [ ] Vérifier les coordonnées x/y-width/height : l'affichage avec list donne l'impression que les
  coordonnées sont inversées
- [ ] Dans dedup.c, si on trouve un hash identique à l'index i, on ne parcourt pas toute
  la base de donnée et donc on ne vérifie pas si on a un pict_id en double après i
- [ ] Remplacer les tableaux statiques dans dedup et image content par des pointeurs?
- [ ] Utiliser des pointeurs pour les struct pict_db?
- [ ] Revoir la doc (rajouter des undocumented in doxygen dans les fichiers .c?)
- [ ] Ajouter des tests automatiques?
- [ ] Ajouter VIPS_INIT et vips_shutdown dans le main? (cf section tests dans donnée de la semaine 7)
