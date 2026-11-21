// Antoine Bernadat, Timothy Biersohn
#include <iostream>
#include <locale>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <optional>
#include <set>
#include <stdexcept>
#include "message.h"
#include "shape.h"
#include "simulation.h"
#include "constantes.h"

using namespace std;

bool verif_id_scav(vector<Scavengers> &sim_scavenger, vector<Corail> &sim_corail);
bool id_scav(const Scavengers &scav, const vector<Corail> &cor);

namespace {
// Accepte les fins de lignes Unix, Windows et les anciens fichiers Mac.
bool read_line(ifstream& file, string& line)
{
    line.clear();
    char c;
    while (file.get(c))
    {
        if (c=='\r' || c=='\n') return true;
        line += c;
    }
    return !line.empty();
}
// Chaque enregistrement ignore lignes vides, commentaires et données supplémentaires.
template<class... Values>
void read_record(ifstream& file, Values&... values)
{
    string line;
    while (read_line(file, line))
    {
        istringstream data(line);
        data.imbue(locale::classic());
        data >> ws;
        if (data.eof() || data.peek() == '#') continue;
        if (!(data >> ... >> values)) throw runtime_error("ligne incomplète ou invalide");
        return;
    }
    throw runtime_error("fin de fichier prématurée");
}
void require(bool condition)
{
    if (!condition) throw runtime_error("valeur ou nombre d'entités invalide");
}
bool valid_age(long long age)
{
    return age >= 0 && age <= numeric_limits<unsigned>::max();
}
}

default_random_engine Simulation::e;

bool Simulation::lecture(string file)
{
    destroy();
    e.seed(1);
    if (file.empty()) return true;
    if (!decode_line(file) || !verif_erreur())
    {
        destroy();
        return false;
    }
    return true;
}

bool Simulation::decode_line(string filename)
{
    ifstream file(filename);
    file.imbue(locale::classic());
    if (!file)
    {
        cerr << "Impossible d'ouvrir : " << filename << '\n';
        return false;
    }
    try
    {
        decodage_alg(file);
        decodage_cor(file);
        decodage_scav(file);
        return true;
    }
    catch (const runtime_error& error)
    {
        cerr << "Fichier invalide : " << error.what() << '\n';
        return false;
    }
}

void Simulation::decodage_alg(ifstream& file)
{
    int count;
    read_record(file, count);
    require(count >= 0);
    for (int i=0; i<count; ++i)
    {
        double x, y;
        long long age;
        read_record(file, x, y, age);
        require(isfinite(x) && isfinite(y) && valid_age(age));
        sim_algue.emplace_back(x, y, static_cast<unsigned>(age));
    }
}

void Simulation::decodage_cor(ifstream& file)
{
    int count;
    read_record(file, count);
    require(count >= 0);
    for (int i=0; i<count; ++i)
    {
        double x, y;
        long long age;
        int id, status, direction, development, n;
        read_record(file, x, y, age, id, status, direction, development, n);
        require(isfinite(x) && isfinite(y) && valid_age(age) && id >= 0 && n > 0);
        require((status==DEAD || status==ALIVE) && (direction==TRIGO || direction==INVTRIGO)
                && (development==EXTEND || development==REPRO));
        vector<Segment> parts;
        S2d base{x,y};
        for (int j=0; j<n; ++j)
        {
            double angle, length;
            read_record(file, angle, length);
            require(isfinite(angle) && isfinite(length));
            parts.emplace_back(base.x, base.y, angle, length);
            base = parts.back().get_extr();
        }
        id_.push_back(id);
        sim_corail.emplace_back(x,y,static_cast<unsigned>(age),id,status,direction,
                               development,n,parts);
    }
    update_segment_all();
}

void Simulation::decodage_scav(ifstream& file)
{
    int count;
    read_record(file, count);
    require(count >= 0);
    for (int i=0; i<count; ++i)
    {
        // L'identifiant optionnel doit être lu sur la même ligne.
        string line;
        do {
            if (!read_line(file, line)) throw runtime_error("scavenger manquant");
            auto start = line.find_first_not_of(" \t\r");
            if (start != string::npos && line[start] != '#') break;
        } while (true);
        istringstream data(line);
        data.imbue(locale::classic());
        double x, y, radius;
        long long age, target=0;
        int status;
        require(bool(data >> x >> y >> age >> radius >> status));
        require(isfinite(x) && isfinite(y) && isfinite(radius) && valid_age(age));
        require(status==FREE || status==EATING);
        if (status==EATING) require(bool(data >> target) && valid_age(target));
        sim_scavenger.emplace_back(x,y,static_cast<unsigned>(age),radius,status,
                                  static_cast<unsigned>(target));
    }
}

bool Simulation::verif_erreur()
{
    if (!verif_ac(sim_algue, sim_corail, sim_scavenger)) return false;
    for (auto& coral : sim_corail)
        if (!coral.verif_corail()) return false;
    for (auto& scav : sim_scavenger)
        if (!scav.verif_rayon()) return false;
    if (!verif_id(id_)) return false;
    if (!verif_superposition(segment_all, id_all)) return false;
    if (!verif_colision(segment_all, id_all)) return false;
    if (!verif_id_scav(sim_scavenger, sim_corail)) return false;
    cout << message::success();
    return true;
}

size_t Simulation::get_nb_algue() const
{
    return sim_algue.size();
}

size_t Simulation::get_nb_corail() const
{
    return sim_corail.size();
}

size_t Simulation::get_nb_scavenger() const
{
    return sim_scavenger.size();
}

bool Simulation::save(string save_file)
{
    ofstream fichier(save_file);

    if (!fichier) { cerr << "Impossible de sauvegarder : " << save_file << '\n'; return false; }
    fichier.imbue(locale::classic());
    fichier << setprecision(numeric_limits<double>::max_digits10);

    save_algues(fichier);
    save_corails(fichier);
    save_scavengers(fichier);

    fichier.close();
    if (!fichier) cerr << "Échec de la sauvegarde : " << save_file << '\n';
    return bool(fichier);
}

void Simulation::save_algues(ofstream &fichier)
{
    fichier << sim_algue.size() << endl;

    for (const auto &algue : sim_algue)
    {
        fichier << "\t" << algue.getX() << " " << algue.getY() << " "

                << algue.getAge() << endl;
    }
}

void Simulation::save_corails(ofstream &fichier)
{
    fichier << "\n"
            << sim_corail.size() << endl;

    for (const auto &corail : sim_corail)
    {
        fichier << "\t" << corail.getX() << " " << corail.getY() << " "

                << corail.getAge() << " " << corail.get_id() << " "

                << corail.getStatutCor() << " " << corail.getDir_Rot() << " "

                << corail.getStatut_Dev() << " " << corail.getNbSeg() << endl;

        for (const auto &segment : corail.get_segment())
        {
            fichier << "\t"
                    << "\t"

                    << segment.get_angle() << " " << segment.get_length() << endl;
        }
    }
}

void Simulation::save_scavengers(ofstream &fichier)
{
    fichier << "\n"
            << sim_scavenger.size() << endl;

    for (const auto &scavenger : sim_scavenger)
    {
        fichier << "\t" << scavenger.getX() << " " << scavenger.getY() << " "

                << scavenger.getAge() << " " << scavenger.getRayonSca() << " "

                << scavenger.getStatutSca() << " ";

        if (scavenger.getStatutSca() == EATING)
            fichier << scavenger.getCorIdCible() << endl;
        else
            fichier << endl;
    }
}

void Simulation::dessin_simu()
{
    for (auto &algue : sim_algue)
    {
        algue.dessin_algue();
    }

    for (auto &corail : sim_corail)
    {
        corail.dessin_corail();
    }

    for (auto &scavenger : sim_scavenger)
    {
        scavenger.dessin_scavenger();
    }
}

void Simulation::destroy()
{
    sim_algue.clear();
    sim_corail.clear();
    sim_scavenger.clear();
    segment_all.clear();
    id_.clear();
    id_all.clear();
    all_new_cor.clear();
    nbSim = 0;
}

bool verif_id_scav(vector<Scavengers> &sim_scavenger, vector<Corail> &sim_corail)
{
    for (const auto &elem : sim_scavenger)
    {
        if (!id_scav(elem, sim_corail))
        {
            cout << message::lifeform_invalid_id(elem.getCorIdCible());
            return false;
        }
    }
    return true;
}

bool id_scav(const Scavengers &scav, const vector<Corail> &cor)
{
    if (scav.getStatutSca() == EATING)
    {
        unsigned id_m = scav.getCorIdCible();
        bool id_found = false;
        for (const auto &elem2 : cor)
        {
            if (id_m == elem2.get_id())
            {
                id_found = true;
                break;
            }
        }
        if (!id_found)
            return false;
    }

    return true;
}

// Met à jour les populations dans l’ordre algues, coraux, scavengers.
void Simulation::mise_a_jour(int, bool naissance_active)
{
    ++nbSim;
    for (auto& algue : sim_algue) algue.setAge(algue.getAge()+1);
    sim_algue.erase(remove_if(sim_algue.begin(), sim_algue.end(), [](const Algue& a) {
        return a.getAge() >= max_life_alg;
    }), sim_algue.end());
    if (naissance_active && bernoulli_distribution(alg_birth_rate)(e))
    {
        uniform_int_distribution<unsigned> position(1, dmax-1);
        const double x=position(e), y=position(e);
        sim_algue.emplace_back(x,y,1);
    }
    for (auto& corail : sim_corail)
    {
        corail.setAge(corail.getAge()+1);
        if (corail.getAge() >= max_life_cor) corail.setStatutCor(DEAD);
    }
    mv_corail();
    add_cor();
    for (auto& scav : sim_scavenger) scav.setAge(scav.getAge()+1);
    sim_scavenger.erase(remove_if(sim_scavenger.begin(), sim_scavenger.end(),
        [](const Scavengers& s) { return s.getAge() >= max_life_sca; }), sim_scavenger.end());
    simu_scav();
}

void Simulation::mv_corail()
{
    for (auto& corail : sim_corail)
    {
        if (corail.getStatutCor() == DEAD) continue;
        const Segment before = corail.effecteur(true);
        const bool eats = found_algue(corail);
        corail.move_corail(eats ? pose_algue : delta_rot);
        if (eats) corail.grow_corail(true);
        const S2d end = corail.effecteur(true).get_extr();
        bool blocked = end.x <= epsil_zero || end.x >= dmax-epsil_zero ||
                       end.y <= epsil_zero || end.y >= dmax-epsil_zero ||
                       colide_ext(corail.effecteur(true),corail.get_id()) || colide_int(corail);
        if (corail.getNbSeg() > 1)
        {
            Segment old = before;
            double a = old.angle_diff(corail.effecteur(false));
            double b = corail.effecteur(true).angle_diff(corail.effecteur(false));
            blocked = blocked || (abs(a) <= delta_rot+1e-10 && a*b <= 0);
        }
        if (blocked)
        {
            corail.effecteur(true) = before;
            corail.setDir_Rot(corail.getDir_Rot()==TRIGO ? INVTRIGO : TRIGO);
            continue;
        }
        if (eats) sim_algue.erase(sim_algue.begin()+number_algue);
        new_seg_cor(corail);
    }
}

void Simulation::new_seg_cor(Corail &corail) 
{
    if (corail.effecteur(1).get_length() >= l_repro)
    {
        if (corail.getStatut_Dev() == EXTEND)
        {
            corail.effecteur(1).setLength(l_seg_interne);
            corail.new_seg();
        }
        else
        {
            corail.effecteur(1).setLength(l_seg_interne);
            Segment first_seg(corail.effecteur(1).get_extr().x,
                              corail.effecteur(1).get_extr().y,
                              corail.effecteur(1).get_angle(),
                              l_repro - l_seg_interne);
            vector<Segment> new_seg;
            new_seg.push_back(first_seg);
            Corail nv_cor(corail.effecteur(1).get_extr().x,
                          corail.effecteur(1).get_extr().y, 1, new_id(),
                          ALIVE, corail.getDir_Rot(), EXTEND, 1, new_seg);
            all_new_cor.push_back(nv_cor);
            corail.new_cor();
        }
    }
}

void Simulation::update_segment_all()
{
    segment_all.clear();
    id_all.clear();
    for (auto const &corail : sim_corail)
    {
        for (int i(0); i < corail.getNbSeg(); ++i)
        {
            segment_all.push_back(corail.get_segment()[i]);
            id_all.push_back(corail.get_id());
        }
    }
}
// Sélectionne la première algue rencontrée dans le sens de rotation.
bool Simulation::found_algue(Corail& corail)
{
    number_algue = -1;
    pose_algue = delta_rot + 1;
    const Segment& tip = corail.effecteur(true);
    for (size_t i=0; i<sim_algue.size(); ++i)
    {
        const double x = sim_algue[i].getX()-tip.get_base().x;
        const double y = sim_algue[i].getY()-tip.get_base().y;
        if (hypot(x,y) > tip.get_length()+1e-10) continue;
        double angle = atan2(y,x)-tip.get_angle();
        if (corail.getDir_Rot()==INVTRIGO) angle = -angle;
        angle = fmod(angle+2*M_PI,2*M_PI);
        if (angle <= delta_rot+1e-10 && angle < pose_algue)
        {
            pose_algue = angle;
            number_algue = static_cast<int>(i);
        }
    }
    return number_algue >= 0;
}

// Ajoute les coraux nés pendant le pas courant.
void Simulation::add_cor()
{
    for (auto &new_corail : all_new_cor)
    {
        sim_corail.push_back(new_corail);
    }
    all_new_cor.clear();
}

namespace {
// Un seul scavenger en route par corail mort.
// Les couples les plus proches sont attribués d'abord, sans double réservation.
vector<optional<unsigned>> assign_scavengers(const vector<Corail>& corals,
                                            const vector<Scavengers>& scavengers)
{
    struct Candidate { double distance; unsigned coral; size_t scavenger; };
    vector<Candidate> candidates;
    vector<optional<unsigned>> targets(scavengers.size());
    set<unsigned> reserved;
    for (const auto& scav : scavengers)
        if (scav.getStatutSca()==EATING) reserved.insert(scav.getCorIdCible());
    for (const auto& coral : corals)
    {
        if (coral.getStatutCor()!=DEAD || reserved.count(coral.get_id())) continue;
        const S2d end = coral.get_segment().back().get_extr();
        for (size_t i=0; i<scavengers.size(); ++i)
            if (scavengers[i].getStatutSca()==FREE)
                candidates.push_back({hypot(end.x-scavengers[i].getX(),
                                            end.y-scavengers[i].getY()), coral.get_id(), i});
    }
    sort(candidates.begin(), candidates.end(), [](const Candidate& a, const Candidate& b) {
        if (a.distance != b.distance) return a.distance < b.distance;
        if (a.coral != b.coral) return a.coral < b.coral;
        return a.scavenger < b.scavenger;
    });
    for (const auto& candidate : candidates)
    {
        if (targets[candidate.scavenger] || reserved.count(candidate.coral)) continue;
        targets[candidate.scavenger] = candidate.coral;
        reserved.insert(candidate.coral);
    }
    return targets;
}
}

// Les affectations sont recalculées au début du pas, avant tout déplacement.

// Les naissances sont différées pour ne pas invalider les références du vecteur.
void Simulation::simu_scav()
{
    vector<Scavengers> births;
    const auto targets = assign_scavengers(sim_corail,sim_scavenger);
    for (size_t i=0; i<sim_scavenger.size(); ++i)
    {
        auto& scav = sim_scavenger[i];
        auto target = sim_corail.end();
        if (scav.getStatutSca()==FREE)
        {
            if (!targets[i]) continue;
            target = find_if(sim_corail.begin(),sim_corail.end(),[&](const Corail& coral) {
                return coral.get_id()==*targets[i];
            });
            if (target==sim_corail.end()) continue;
            const S2d end = target->effecteur(true).get_extr();
            const double closest = hypot(end.x-scav.getX(),end.y-scav.getY());
            if (closest <= delta_l)
            {
                scav.tp_scavengers(end.x,end.y);
                scav.change_statut(target->get_id());
            }
            else scav.move_scavenger(delta_l*(end.x-scav.getX())/closest,
                                      delta_l*(end.y-scav.getY())/closest);
            continue; // L'alimentation commence à la mise à jour suivante.
        }
        target = find_if(sim_corail.begin(),sim_corail.end(),[&](const Corail& coral) {
            return coral.get_id()==scav.getCorIdCible() && coral.getStatutCor()==DEAD;
        });
        if (target==sim_corail.end()) { scav.change_statut(); continue; }
        Segment& tip = target->effecteur(true);
        const double angle = tip.get_angle();
        const double eaten = min(double(delta_l),tip.get_length());
        tip.setLength(tip.get_length()-eaten);
        const S2d end = tip.get_extr();
        scav.tp_scavengers(end.x,end.y);
        if (scav.grow_scavenger())
            births.emplace_back(end.x+delta_l*cos(angle),end.y+delta_l*sin(angle),
                                1,r_sca,FREE,0);
        if (tip.get_length() <= 1e-10) target->get_true_segment().pop_back();
        if (target->getNbSeg()==0)
        {
            sim_corail.erase(target);
            scav.change_statut();
        }
    }
    sim_scavenger.insert(sim_scavenger.end(),births.begin(),births.end());
}

unsigned Simulation::new_id()
{
    unsigned id=1;
    auto exists = [&](const vector<Corail>& corals) {
        return any_of(corals.begin(),corals.end(),[&](const Corail& c) {return c.get_id()==id;});
    };
    while (exists(sim_corail) || exists(all_new_cor)) ++id;
    return id;
}

// Teste les collisions avec les autres coraux.
bool Simulation::colide_ext(Segment effecteur, unsigned id)
{
    for (auto const &corail : sim_corail)
    {
        for (auto const &seg : corail.get_segment())
        {
            if (doIntersect(seg, effecteur, true) && corail.get_id() != id)
            {
                return true;
            }
        }
    }
    for (const auto& coral : all_new_cor)
        for (const auto& seg : coral.get_segment())
            if (doIntersect(seg,effecteur,true)) return true;
    return false;
}

// Teste les collisions internes, hors articulation du dernier segment.
bool Simulation::colide_int(Corail corail)
{
    const auto parts = corail.get_segment();
    // Le segment lui-même et son voisin articulé ne sont pas des obstacles.
    for (size_t i=0; i+2<parts.size(); ++i)
        if (doIntersect(parts[i],parts.back(),true)) return true;
    return parts.size()>1 && superposition(parts[parts.size()-2],parts.back());
}
