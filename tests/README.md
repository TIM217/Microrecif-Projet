# Tests

## Exécution

Depuis la racine du projet :

```sh
make test
make test-sanitize
```

Un compilateur C++17 et Python 3 suffisent. Aucun affichage ni bibliothèque GTK
n'est nécessaire pour tester le modèle.

## Organisation

| Chemin | Contenu |
| --- | --- |
| `t00.txt` | État initial simple et valide |
| `t01.txt` à `t21.txt` | Fichiers invalides et diagnostics |
| `t22.txt` à `t46.txt` | Mondes valides et scénarios d'évolution |
| `public/out01.txt` à `public/out15.txt` | Messages de référence |
| `runner/model.cc` | Pilote du modèle sans affichage |
| `runner/regression.py` | Assertions et scénarios complémentaires |

Les fichiers numérotés sont conservés comme données de référence. Les cas
complémentaires sont créés dans un répertoire temporaire, supprimé en fin de test.
Les exécutables du pilote sont générés par Make et ne sont pas versionnés.

## Couverture

- Lecture des 47 scénarios et comparaison des 15 messages de référence.
- Rotation, consommation des algues, croissance et reproduction des coraux.
- Collisions, inversion du sens de rotation et passage des angles entre −π et π.
- Vieillissement et disparition de plusieurs organismes au même pas.
- Attribution exclusive des cibles, égalités de distance et réattribution.
- Déplacement, alimentation et reproduction des scavengers.
- Fichiers incomplets, commentaires et fins de ligne.
- Chargements successifs, remise à zéro du modèle et du générateur aléatoire.
- Sauvegarde et relecture sans perte après 10, 100 et 2 000 pas.

Les tests du modèle ne vérifient pas les interactions GTK. Pour contrôler
l'interface, lancer `make demo`, puis essayer Start/Stop, Step, Open, Save et le
redimensionnement de la fenêtre.
