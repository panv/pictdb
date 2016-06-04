# TODO

- [ ] Gérer le n° de version de la database dans le garbage collecting (réécrire le header à la fin)
- [x] Vérifier les conversions de types size_t -> uint32 dans image_content notamment
- [x] GROS BUG : essayer de read une image non encore resizée fait crasher le programme;
      problème de free avec vips, probablement dans lazily resize.
- [x] Réparer read
- [x] Réparer get_resolution: le free (commenté) fait crasher le programme
- [x] Réparer dedup: insérer la même image 2 fois insère 2 images au lieu de dédupliquer
- [x] Vérifier les coordonnées x/y-width/height : l'affichage avec list donne l'impression que les
  coordonnées sont inversées
- [x] Dans dedup.c, si on trouve un hash identique à l'index i, on ne parcourt pas toute
  la base de donnée et donc on ne vérifie pas si on a un pict_id en double après i
- [x] rajouter la déduplication dans read + déplacer hshcmp dans db_utils
- [x] Remplacer les tableaux statiques dans dedup et image content par des pointeurs? Non. Malloc+Pointers seulement pour allocation dynamique.
- [x] Utiliser des pointeurs pour les struct pict_db?
- [ ] Revoir la doc (rajouter des undocumented in doxygen dans les fichiers .c?)
- [ ] Ajouter des tests automatiques?
- [x] Ajouter VIPS_INIT et vips_shutdown dans le main? (cf section tests dans donnée de la semaine 7)
- [x] Possible bonus? http://moodle.epfl.ch/mod/forum/discuss.php?d=6645 (déjà fait, voir point 7)
