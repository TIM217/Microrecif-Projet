// Antoine Bernadat, Timothy Biersohn
#ifndef SIMULATION_H
#define SIMULATION_H

#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <random>
#include "lifeform.h"
class Simulation
{
public:
    size_t get_nb_algue() const;
    size_t get_nb_corail() const;
    size_t get_nb_scavenger() const;

    bool lecture(std::string file);
    void destroy();

    bool decode_line(std::string fichier);
    bool verif_erreur();
    void mise_a_jour(int count, bool naissance_algue);
    bool save(std::string save_file);
    void save_algues(std::ofstream &fichier);
    void save_corails(std::ofstream &fichier);
    void save_scavengers(std::ofstream &fichier);
    void dessin_simu();
    void add_cor();
    void update_segment_all();
    void mv_corail();
    void simu_scav();
    bool found_algue(Corail &corail);
    void new_seg_cor(Corail &corail);
    unsigned new_id();
    bool colide_ext(Segment effecteur, unsigned id);
    bool colide_int(Corail corail);

    void decodage_alg(std::ifstream &fichier);
    void decodage_cor(std::ifstream &fichier);
    void decodage_scav(std::ifstream &fichier);

private:
    std::vector<Algue> sim_algue;
    std::vector<Corail> sim_corail, all_new_cor;
    std::vector<Scavengers> sim_scavenger;
    std::vector<Segment> segment_all;
    std::vector<int> id_all, id_;

    unsigned long long nbSim = 0;
    int number_algue = -1;
    double pose_algue = 0;

    static std::default_random_engine e;
};

#endif
