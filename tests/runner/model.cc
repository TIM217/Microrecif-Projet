// Pilote de vérification du modèle : aucun affichage ni dépendance GTK.
#include "simulation.h"
#include <cstdlib>
#include <iostream>
void dessin_cercle(double,double,double,bool) {}
void dessin_carre(double,double,int) {}
void dessin_segment(double,double,double,double,int) {}
int main(int argc, char** argv)
{
    if (argc < 2) return 1;
    Simulation sim;
    if (std::string(argv[1]) == "--sequence")
    {
        if (argc != 3) return 1;
        for (const char* suffix : {"first", "second"})
        {
            if (!sim.lecture("tests/t41.txt")) return 4;
            for (int step=1; step<=100; ++step) sim.mise_a_jour(step,true);
            if (!sim.save(std::string(argv[2])+"/"+suffix+".txt")) return 4;
            if (sim.lecture("tests/t12.txt")) return 4;
            if (sim.get_nb_algue() || sim.get_nb_corail() || sim.get_nb_scavenger()) return 4;
        }
        if (!sim.lecture("tests/t00.txt") || !sim.lecture("tests/t00.txt")) return 4;
        if (sim.get_nb_algue()!=1 || sim.get_nb_corail()!=1 || sim.get_nb_scavenger()!=1) return 4;
        if (!sim.lecture("")) return 4;
        return (sim.get_nb_algue() || sim.get_nb_corail() || sim.get_nb_scavenger()) ? 4 : 0;
    }
    if (!sim.lecture(argv[1])) return 2;
    int steps = argc > 2 ? std::atoi(argv[2]) : 0;
    bool births = argc > 4 && std::string(argv[4]) == "births";
    for (int i=1; i<=steps; ++i) sim.mise_a_jour(i,births);
    if (argc > 3 && !sim.save(argv[3])) return 3;
    return 0;
}
