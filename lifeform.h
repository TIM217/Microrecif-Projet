// Antoine Bernadat, Timothy Biersohn
#ifndef LIFEFORM_H
#define LIFEFORM_H
#include <vector>
#include "shape.h"

class Entite
{
public:
    Entite();
    Entite(double x, double y, unsigned age);

    unsigned getAge() const;
    double getX() const;
    double getY() const;

    void setAge(unsigned age);

    bool verif_age_centre();

protected:
    int nb_;
    S2d base_;
    unsigned age_;
};

class Algue : public Entite
{
public:
    Algue();
    Algue(double x, double y, unsigned age);
    ~Algue();

    void dessin_algue();
};
class Corail : public Entite
{
public:
    Corail();
    Corail(double x, double y, unsigned age, int id, int statut_cor, double dir_rot,
           int statut_dev, int nbseg, std::vector<Segment> segment);
    ~Corail();

    unsigned get_id() const;
    int getStatutCor() const;
    double getDir_Rot() const;
    int getStatut_Dev() const;
    int getNbSeg() const;
    std::vector<Segment> get_segment() const;
    std::vector<Segment> &get_true_segment();

    void setStatutCor(int statut_cor);
    void setDir_Rot(double dir_rot);

    void new_seg();
    void new_cor();
    bool verif_corail();
    void move_corail();
    void move_corail(double phi);
    void grow_corail(bool extand);
    Segment &effecteur(bool last);

    void dessin_corail();

private:
    unsigned id1_;
    int statut_cor_;
    double dir_rot_;
    int statut_dev1_;
    std::vector<Segment> segment_;
    std::vector<int> id_;
};

class Scavengers : public Entite
{
public:
    Scavengers(double x, double y, unsigned age, double rayon, int statut_sca,
               unsigned corail_id_cible);
    Scavengers();
    ~Scavengers();

    int getStatutSca() const;
    unsigned getCorIdCible() const;
    double getRayonSca() const;

    void setStatutScav(int statut);
    void setCorIdCible(unsigned cor_id_cible);
    void change_statut(unsigned id);
    void change_statut();

    bool verif_rayon();
    bool grow_scavenger();
    void move_scavenger(double s_vect_x, double s_vect_y);
    void tp_scavengers(double x, double y);

    void dessin_scavenger();

private:
    double rayon1_;
    int statut_sca1_;
    unsigned corail_id_cible1_;
};

bool verif_superposition(std::vector<Segment> seg, std::vector<int> id_all);
bool verif_colision(std::vector<Segment> segment_all, std::vector<int> id_all);
bool verif_id(std::vector<int> id_);
bool verif_ac(std::vector<Algue> &algues, std::vector<Corail> &corails,
              std::vector<Scavengers> &scavengers);

#endif
