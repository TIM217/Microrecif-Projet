#!/usr/bin/env python3
"""Régressions fonctionnelles, fichiers publics et simulations longues, sans GTK."""
import math
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
EXE = Path(sys.argv[1]).resolve()
checks = 0


def expect(condition, description):
    global checks
    if not condition:
        raise AssertionError(description)
    checks += 1


def invoke(source, steps=0, destination=None, births=False):
    args = [str(EXE), str(source), str(steps)]
    if destination is not None:
        args.append(str(destination))
        if births:
            args.append('births')
    result = subprocess.run(args, cwd=ROOT, capture_output=True, text=True, timeout=40)
    expect('runtime error:' not in result.stderr and 'AddressSanitizer' not in result.stderr,
           f'{source}: erreur mémoire\n{result.stderr}')
    return result


def read_state(path):
    lines = iter(line.strip() for line in path.read_text().splitlines() if line.strip())
    algae = [list(map(float, next(lines).split())) for _ in range(int(next(lines)))]
    corals = []
    for _ in range(int(next(lines))):
        data = list(map(float, next(lines).split()))
        segments = [list(map(float, next(lines).split())) for _ in range(int(data[7]))]
        corals.append((data, segments))
    scavengers = [list(map(float, next(lines).split())) for _ in range(int(next(lines)))]
    expect(next(lines, None) is None, 'la sauvegarde doit être entièrement consommée')
    for data, segments in corals:
        expect(len(segments) > 0, 'un corail doit avoir au moins un segment')
        expect(all(length > 0 for angle, length in segments), 'longueurs positives')
    values = algae + scavengers + [d for d, _ in corals] + [s for _, ss in corals for s in ss]
    expect(all(math.isfinite(x) for row in values for x in row), 'valeurs finies')
    expect(len({d[3] for d, _ in corals}) == len(corals), 'identifiants uniques')
    return algae, corals, scavengers


with tempfile.TemporaryDirectory(prefix='ba2-regression-') as directory:
    tmp = Path(directory)
    serial = 0

    def state(source, steps=0, births=False, reload=True):
        global serial
        serial += 1
        saved = tmp / f'state-{serial}.txt'
        result = invoke(source, steps, saved, births)
        expect(result.returncode == 0, f'{source}: {result.stdout}\n{result.stderr}')
        data = read_state(saved)
        if reload:
            second = tmp / f'reload-{serial}.txt'
            result = invoke(saved, 0, second)
            expect(result.returncode == 0, f'Relecture {source} après {steps} pas: {result.stdout} {result.stderr}')
            expect(saved.read_text() == second.read_text(), 'sauvegarde/relecture sans perte')
        return data

    def fixture(name, content):
        path = tmp / name
        path.write_text(content)
        return path

    result = subprocess.run([str(EXE), '--sequence', str(tmp)], cwd=ROOT,
                            capture_output=True, text=True, timeout=40)
    expect(result.returncode == 0, 'ouvertures successives : remise à zéro et reprise après erreur')
    expect((tmp/'first.txt').read_text() == (tmp/'second.txt').read_text(),
           'réinitialisation du générateur aléatoire et de tout le modèle')
    for i in range(47):
        source = ROOT / 'tests' / f't{i:02}.txt'
        result = invoke(source)
        expect(result.returncode == (2 if 1 <= i <= 21 else 0), f'lecture {source}: {result.stdout}')
        expected = ROOT / 'tests' / 'public' / f'out{i:02}.txt'
        if expected.exists():
            expect(result.stdout.strip() == expected.read_text().strip(), f'message public {i}')
    print('47 fichiers : acceptation/rejet corrects ; 15 messages publics conformes.', flush=True)

    for i in range(1, 5):
        algae, corals, _ = state(ROOT/'tests/t36.txt', i)
        expect(len(algae) == (1 if i == 1 else 0), 't36 : repas au pas 2')
        expect(corals[0][1][0][1] == (24 if i == 1 else 28), 't36 : croissance de 4')
    algae, corals, _ = state(ROOT/'tests/t40.txt', 2)
    expect(len(algae) == 1 and algae[0][0] == 34, 'première algue selon angle')
    algae, _, _ = state(ROOT/'tests/t40.txt', 3)
    expect(len(algae) == 0, 'deuxième repas au pas 3')
    _, corals, _ = state(ROOT/'tests/t41.txt', 2)
    expect(len(corals) == 2, 't41 : reproduction')
    expect(sorted(c[1][0][1] for c in corals) == [12,20], 'longueurs parent/enfant')
    _, corals, _ = state(ROOT/'tests/t41.txt', 10)
    expect(any(len(parts) > 1 for _, parts in corals), 't41 : création de segment')

    for i in (42, 43):
        before = state(ROOT/f'tests/t{i}.txt')
        after = state(ROOT/f'tests/t{i}.txt', 1)
        expect(len(before[0]) == len(after[0]), 'collision : aucune algue mangée')
        expect(before[1][0][1] == after[1][0][1], 'collision : restauration exacte')
        expect(before[1][0][0][5] != after[1][0][0][5], 'collision : inversion du sens')

    # Une algue plus proche angulairement est placée en deuxième ligne.
    source = fixture('angle.txt', f'2\n{50+20*math.cos(.04)} {50+20*math.sin(.04)} 1\n'
                     f'{50+20*math.cos(.02)} {50+20*math.sin(.02)} 1\n'
                     '1\n50 50 1 1 1 0 0 1\n0 24\n0\n')
    algae, corals, _ = state(source, 1)
    expect(len(algae) == 1 and abs(corals[0][1][0][0]-.02) < 1e-10, 'priorité angulaire indépendante du fichier')
    for direction, angle, target in [(0,3.12,-3.12),(1,-3.12,3.12)]:
        source = fixture('wrap.txt', f'1\n{100+20*math.cos(target)} {100+20*math.sin(target)} 1\n'
                         f'1\n100 100 1 1 1 {direction} 0 1\n{angle} 24\n0\n')
        algae, corals, _ = state(source,1)
        expect(not algae and corals[0][1][0][1] == 28, 'repas au passage -pi/pi')
    source = fixture('dead.txt','1\n60 50 1\n1\n50 50 1500 1 0 0 0 1\n0 24\n0\n')
    algae, corals, _ = state(source,1)
    expect(len(algae)==1 and corals[0][1] == [[0,24]], 'un corail mort ne mange pas et ne tourne pas')
    source = fixture('ages.txt','2\n10 10 499\n20 20 499\n0\n2\n40 40 1999 3 0\n50 50 1999 3 0\n')
    expect(state(source,1) == ([],[],[]), 'disparitions consécutives sans élément sauté')
    source = fixture('scavengers.txt','0\n2\n50 50 1500 1 0 0 0 1\n0 12\n150 150 1500 2 0 0 0 1\n0 12\n2\n70 50 1 3 0\n170 150 1 3 0\n')
    _, _, scav = state(source,1)
    expect([s[0] for s in scav] == [66,166], 'tous les scavengers libres avancent de 4')
    expect(state(source,5)[1] == [], 'consommation complète des deux cibles')
    # Variante : un seul candidat se déplace vers chaque corail mort.
    source = fixture('nearest.txt','0\n1\n50 50 1500 1 0 0 0 1\n0 12\n3\n100 50 1 3 0\n70 50 1 3 0\n90 50 1 3 0\n')
    _, _, scav = state(source,1)
    expect([s[0] for s in scav] == [100,66,90], 'seul le scavenger le plus proche avance')
    _, _, scav = state(source,2)
    expect([s[0] for s in scav] == [100,62,90], 'les autres restent immobiles pendant le trajet')
    source = fixture('tie.txt','0\n1\n50 50 1500 1 0 0 0 1\n0 12\n2\n70 50 1 3 0\n70 50 1 3 0\n')
    _, _, scav = state(source,1)
    expect([s[0] for s in scav] == [66,70], 'égalité de distance : un seul gagnant déterministe')
    source = fixture('occupied.txt','0\n1\n50 50 1500 1 0 0 0 1\n0 12\n2\n63 50 1 3 0\n62 50 1 3 1 1\n')
    _, _, scav = state(source,1)
    expect([s[0] for s in scav] == [63,58], 'un corail déjà mangé ne reçoit aucun autre scavenger')
    source = fixture('closest-pair.txt','0\n2\n50 50 1500 1 0 0 0 1\n0 12\n150 150 1500 2 0 0 0 1\n0 12\n1\n170 150 1 3 0\n')
    _, _, scav = state(source,1)
    expect(scav[0][:2] == [166,150], 'un scavenger ne poursuit qu’une cible, la plus proche')
    source = fixture('replacement.txt','0\n1\n50 50 1500 1 0 0 0 1\n0 12\n2\n70 50 1998 3 0\n90 50 1 3 0\n')
    _, _, scav = state(source,2)
    expect(len(scav)==1 and scav[0][0]==86, 'réattribution après disparition du candidat')
    source = fixture('no-dead.txt','0\n0\n1\n70 50 1 3 0\n')
    expect(state(source,1)[2][0][:2] == [70,50], 'aucune cible : aucune agitation')
    _, corals, scav = state(ROOT/'tests/t44.txt',100)
    expect(not corals and len(scav) >= 3, 't44 : consommation et reproduction')

    # Formats flexibles et erreurs de syntaxe : ni boucle infinie ni état partiel.
    for content in ('', '# seul commentaire\n', '1\n', '-1\n0\n0\n',
                    '1\n0 0 0\n0\n0\n', '0\n1\n10 10 1 1 1 0 0 0\n0\n',
                    '1\nnan 10 1\n0\n0\n', '1\n10 10 -1\n0\n0\n'):
        expect(invoke(fixture('bad.txt',content)).returncode == 2, 'rejet propre du fichier mal formé')
    for newline in ('\n','\r\n','\r'):
        source = fixture('comments.txt', newline.join([' # commentaire','1 extra','10 10 1 # algue','','# coraux','0',' # scavengers','0','']))
        expect(len(state(source)[0]) == 1, 'commentaires, champs supplémentaires et fins de lignes')
    print('Croissance, reproduction, collisions, vieillissement, scavengers et formats : OK.', flush=True)

    # Chaque scénario valide est exercé avec naissances ; vérification de relecture
    # pendant et après l'évolution, y compris les segments partiellement consommés.
    for i in [0] + list(range(22,47)):
        for steps in (10,100,2000):
            state(ROOT/f'tests/t{i:02}.txt',steps,births=True)
    state(ROOT/'examples/recif.txt', 2000, births=True)
    print(f'Simulations longues et sauvegardes : OK ({checks} vérifications).', flush=True)
