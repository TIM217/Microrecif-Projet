# Format des fichiers de simulation

Un fichier décrit successivement les algues, les coraux et les scavengers. Chaque
section commence par son nombre d'entités. Les coordonnées sont exprimées dans
un monde de 256 × 256 unités ; les angles sont en radians.

## Exemple

```text
# Algues : x y age
1
128 128 1

# Coraux : x y age id statut direction developpement nombre_segments
1
100 200 1 99 1 0 0 1
    0 12

# Scavengers : x y age rayon statut [id_cible]
1
200 100 1 3 0
```

## Algues

Chaque algue est définie sur une ligne :

```text
x y age
```

## Coraux

L'en-tête d'un corail est suivi d'une ligne `angle longueur` par segment. Chaque
segment commence à l'extrémité du précédent ; le premier commence en `(x, y)`.

| Champ | Valeurs |
| --- | --- |
| `id` | Entier positif ou nul, unique parmi les coraux |
| `statut` | `0` : mort ; `1` : vivant |
| `direction` | `0` : sens trigonométrique ; `1` : sens inverse |
| `developpement` | `0` : ajout d'un segment ; `1` : reproduction |
| `nombre_segments` | Entier strictement positif |
| `angle` | Entre −π et π |
| `longueur` | De 12 inclus à 40 exclu pour un corail vivant |

Les segments morts de longueur 4 ou 8 sont également acceptés : ces états
transitoires apparaissent pendant leur consommation. Ils permettent de recharger
une sauvegarde prise au cours d'un repas.

## Scavengers

```text
x y age rayon statut [id_cible]
```

Le rayon doit être compris entre 3 inclus et 10 exclu. Le statut vaut `0` pour un
scavenger libre et `1` pour un scavenger qui mange. Dans le second cas, l'identifiant
de sa cible doit être présent sur la même ligne et désigner un corail existant.

## Lecture et validation

- Les âges doivent être strictement positifs.
- Les positions explicites doivent appartenir à `[1, 255]` sur les deux axes.
- Les extrémités des segments doivent rester à l'intérieur du monde.
- Les collisions et superpositions interdites sont rejetées.
- Les lignes vides et les commentaires commençant par `#` sont ignorés.
- Les données supplémentaires en fin de ligne sont ignorées.
- Les fins de ligne Unix, Windows et Mac sont acceptées.

Les coordonnées et les longueurs utilisent un point comme séparateur décimal.
Un monde vide contient simplement trois lignes avec la valeur `0`.
