// Antoine Bernadat, Timothy Biersohn
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#define _USE_MATH_DEFINES
#include "lifeform.h"
#include "shape.h"
#include "message.h"
#include "constantes.h"

using namespace std;


void verif_rayon(unsigned rayon);

Entite::Entite()
    : base_(), age_() {}

Entite::Entite(double x, double y, unsigned age)
    : base_{x, y}, age_(age) {}

unsigned Entite::getAge() const
{
    return age_;
}

double Entite::getX() const
{
    return base_.x;
}

double Entite::getY() const
{
    return base_.y;
}

void Entite::setAge(unsigned age)
{
    age_ = age;
}

bool Entite::verif_age_centre()
{
    if (age_ <= 0)
    {
        cout << message::lifeform_age(age_);
        return false;
    }

    if ((base_.x < 1 || base_.x > (dmax - 1)) || (base_.y < 1 || base_.y > (dmax - 1)))
    {
        cout << message::lifeform_center_outside(base_.x, base_.y);
        return false;
    }
    return true;
}

Algue::Algue(double x, double y, unsigned age)
    : Entite(x, y, age) {}

Algue::Algue()
    : Entite() {}

Algue::~Algue() {}

void Algue::dessin_algue()
{
    draw_shape_circle(base_.x, base_.y, 0);
}

Corail::Corail(double x, double y, unsigned age, int id, int statut_cor,
               double dir_rot, int statut_dev, int, vector<Segment> segment)
    : Entite(x, y, age), id1_(id), statut_cor_(statut_cor), dir_rot_(dir_rot),
      statut_dev1_(statut_dev), segment_(segment) {}

Corail::Corail()
    : Entite(),
      id1_(), statut_cor_(), dir_rot_(), statut_dev1_(), segment_() {}

Corail::~Corail() {}

unsigned Corail::get_id() const
{
    return id1_;
}

int Corail::getStatutCor() const

{
    return statut_cor_;
}

double Corail::getDir_Rot() const

{
    return dir_rot_;
}

int Corail::getStatut_Dev() const

{
    return statut_dev1_;
}

int Corail::getNbSeg() const

{
    return static_cast<int>(segment_.size());
}

vector<Segment> Corail::get_segment() const
{
    return segment_;
}

vector<Segment> &Corail::get_true_segment()
{
    return segment_;
}

void Corail::setStatutCor(int statut_cor)
{
    statut_cor_ = statut_cor;
}

void Corail::setDir_Rot(double dir_rot)
{
    dir_rot_ = dir_rot;
}

bool Corail::verif_corail()
{
    for (const auto &elem : segment_)
    {
        if ((elem.get_length() < (statut_cor_ == DEAD ? delta_l : l_repro - l_seg_interne)) ||
            (elem.get_length() >= l_repro))
        {
            cout << message::segment_length_outside(id1_, elem.get_length());
            return false;
        }
    }
    for (const auto &elem : segment_)
    {
        if ((elem.get_angle() < (-M_PI)) || (elem.get_angle() > M_PI))
        {

            cout << message::segment_angle_outside(id1_, elem.get_angle());
            return false;
        }
    }
    for (const auto &elem : segment_)
    {
        if (((elem.get_extr().x) <= 0) || ((elem.get_extr().y) <= 0) ||
            ((elem.get_extr().x) >= dmax) || ((elem.get_extr().y) >= dmax))
        {
            cout << message::lifeform_computed_outside(id1_, elem.get_extr().x,
                                                       elem.get_extr().y);
            return false;
        }
    }
    return true;
}

Segment &Corail::effecteur(bool last)
{
    if (last)
        return segment_[segment_.size() - 1];
    else
        return segment_[segment_.size() - 2];
}

void Corail::move_corail()
{
    if (statut_cor_)
    {
        if (dir_rot_ == TRIGO)
            effecteur(1).setAngle(effecteur(1).get_angle() + delta_rot);
        else
            effecteur(1).setAngle(effecteur(1).get_angle() - delta_rot);
    }
}

void Corail::move_corail(double phi)
{
    if (statut_cor_)
    {
        if (dir_rot_ == 0)
            effecteur(1).setAngle(effecteur(1).get_angle() + phi);
        else
            effecteur(1).setAngle(effecteur(1).get_angle() - phi);
    }
}

// Allonge ou réduit le dernier segment du corail.
void Corail::grow_corail(bool extand)
{
    if (extand)
        effecteur(1).setLength(effecteur(1).get_length() + delta_l);
    else
        effecteur(1).setLength(effecteur(1).get_length() - delta_l);
}

// Ajoute un segment à la chaîne du corail.
void Corail::new_seg()
{
    Segment new_seg(effecteur(1).get_extr().x, effecteur(1).get_extr().y,
                    effecteur(1).get_angle(), l_repro - l_seg_interne);
    segment_.push_back(new_seg);
    statut_dev1_ = REPRO;
}

void Corail::new_cor()
{
    effecteur(1).setLength(l_repro / 2);
    statut_dev1_ = EXTEND;
}


void Corail::dessin_corail()
{
    draw_shape_square(base_.x, base_.y, statut_cor_);
    for (const auto &elem : segment_)
    {
        draw_shape_rectangle(elem.get_base().x, elem.get_base().y,
                             elem.get_extr().x, elem.get_extr().y, statut_cor_);
    }
}

Scavengers::Scavengers(double x, double y, unsigned age, double rayon,
                       int statut_sca, unsigned corail_id_cible)
    : Entite(x, y, age), rayon1_(rayon), statut_sca1_(statut_sca),
      corail_id_cible1_(corail_id_cible) {}

Scavengers::Scavengers()
    : Entite(),
      rayon1_(), statut_sca1_(), corail_id_cible1_() {}

Scavengers::~Scavengers() {}

int Scavengers::getStatutSca() const
{
    return statut_sca1_;
}

unsigned Scavengers::getCorIdCible() const
{
    return corail_id_cible1_;
}

double Scavengers::getRayonSca() const

{
    return rayon1_;
}

void Scavengers::setStatutScav(int statut)
{
    statut_sca1_ = statut;
}

void Scavengers::setCorIdCible(unsigned cor_id_cible)
{
    corail_id_cible1_ = cor_id_cible;
}

bool Scavengers::verif_rayon()
{
    if ((rayon1_ < r_sca) || (rayon1_ >= r_sca_repro))
    {
        cout << message::scavenger_radius_outside(rayon1_);
        return false;
    }
    return true;
}

void Scavengers::move_scavenger(double s_vect_x, double s_vect_y)
{
    base_.x += s_vect_x;
    base_.y += s_vect_y;
}

void Scavengers::tp_scavengers(double x, double y)
{
    base_.x = x;
    base_.y = y;
}

void Scavengers::change_statut(unsigned id)
{
    statut_sca1_ = EATING;
    corail_id_cible1_ = id;
}

void Scavengers::change_statut()
{
    statut_sca1_ = FREE;
    corail_id_cible1_ = 0;
}

bool Scavengers::grow_scavenger()
{
    rayon1_ += delta_r_sca;

    if (rayon1_ >= r_sca_repro)
    {
        rayon1_ = r_sca;
        return true;
    }
    return false;
}

void Scavengers::dessin_scavenger()
{
    draw_shape_circle(base_.x, base_.y, rayon1_);
}

bool verif_ac(vector<Algue> &algues, vector<Corail> &corails,
              vector<Scavengers> &scavengers)
{
    for (size_t i(0); i < algues.size(); ++i)
    {
        if (!algues[i].verif_age_centre())
            return false;
    }
    for (size_t j(0); j < corails.size(); ++j)
    {
        if (!corails[j].verif_age_centre())
            return false;
    }
    for (size_t z(0); z < scavengers.size(); ++z)
    {
        if (!scavengers[z].verif_age_centre())
            return false;
    }
    return true;
}

// Vérifie l’unicité des identifiants.
bool verif_id(vector<int> id_)
{
    for (size_t i(0); i < id_.size(); ++i)
    {
        for (size_t j(0); j < id_.size(); ++j)
        {
            if ((i != j) && (id_[i] == id_[j]))
            {
                cout << message::lifeform_duplicated_id(id_[i]);
                return false;
            }
        }
    }
    return true;
}

// Vérifie la superposition des segments consécutifs.
bool verif_superposition(vector<Segment> seg, vector<int> id_all)
{
    unsigned local = 0;
    for (size_t i = 0; i + 1 < seg.size(); ++i)
    {
        if (id_all[i] != id_all[i+1]) { local = 0; continue; }
        if (superposition(seg[i], seg[i+1]))
        {
            cout << message::segment_superposition(id_all[i], local, local+1);
            return false;
        }
        ++local;
    }
    return true;
}

bool verif_colision(vector<Segment> segments, vector<int> ids)
{
    vector<unsigned> indices(segments.size());
    for (size_t i=1; i<segments.size(); ++i)
        indices[i] = ids[i] == ids[i-1] ? indices[i-1]+1 : 0;
    for (size_t i=0; i<segments.size(); ++i)
        for (size_t j=i+1; j<segments.size(); ++j)
        {
            if (ids[i] == ids[j] && j == i+1) continue;
            if (doIntersect(segments[i], segments[j], false))
            {
                cout << message::segment_collision(ids[i], indices[i], ids[j], indices[j]);
                return false;
            }
        }
    return true;
}
