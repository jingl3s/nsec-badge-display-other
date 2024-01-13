# TODO

- [x] Changer le style pour supprimer les marges
  - Difficulté mais trouvé un contournement
- [x] Bouton fin de match
- [x] Fin de match bip 3 fois
  - buzzer.h boucle sur tous les sont depuis un nouveau tab dedie pour tous les ecouter
  ```c
      if (sound >= Sounds::ListCount || sound < 0) {
        ESP_LOGE(TAG, "Not playing %u, there are only %u sounds", sound, Sounds::ListCount);
        return;
    }
  ```
- IGNORÉ: Fin de match non possible si egalite
- IGNORÉ Fin de match bloque mise a jour score jus'au reinit
- [ ] Change la couleur de fond vers l'équipe qui a gagné
- [x] Chaque equipe a une couleur
- [ ] Fin de match change couleur led vers la couleur equipe gagné
- [x] Affichage des équipes par rapport une liste d'équipes disponibles
- [x] Lors reinit 0-0 alors change les equipes avec random
- [x] Revoir les random pour eviter celui ESP qui active BT et WIFI
- [x] Changer la couleur de la led
- [x] Regler la luminosité écran
- [x] Supprimer le code de mesh


Docs
- LVDS https://docs.lvgl.io/8.2/overview/style.html
  - https://github.com/lvgl/lvgl/tree/master
- Marges et padding https://docs.lvgl.io/8.2/overview/coords.html?highlight=margin
- Luminosité https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/Examples/Basics/4-BacklightControlTest/4-BacklightControlTest.ino
- Demos https://github.com/lvgl/lv_demos
