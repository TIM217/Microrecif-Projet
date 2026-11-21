// Antoine Bernadat, Timothy Biersohn
#include <iostream>
#include "graphic_gui.h"

enum Couleur {ROUGE, VERT, BLEU, NOIR};

static const Cairo::RefPtr<Cairo::Context> *ptcr(nullptr);

void choix_couleur(Couleur color);

void set_cr(const Cairo::RefPtr<Cairo::Context> &cr)
{
    ptcr = &cr;
    (*ptcr)->set_line_width(1.0);
}

void choix_couleur(Couleur color)
{
    switch (color)
    {
    case ROUGE:
        (*ptcr)->set_source_rgb(1.0, 0.0, 0.0);
        break;
    case VERT:
        (*ptcr)->set_source_rgb(0.0, 1.0, 0.0);
        break;
    case BLEU:
        (*ptcr)->set_source_rgb(0.0, 0.0, 1.0);
        break;
    case NOIR:
        (*ptcr)->set_source_rgb(0.0, 0.0, 0.0);
        break;
    default:
        break;
    }
}

void dessin_cercle(double x, double y, double rayon, bool type)
{
    if(type == 0)   // Scavenger
    {
    choix_couleur(ROUGE);
    (*ptcr)->arc(x, y, rayon, 0.0, 2.0 * M_PI);
    (*ptcr)->move_to(x, y);
    (*ptcr)->stroke();
    }
    if(type == 1)   // Algue
    {
    choix_couleur(VERT);
    (*ptcr)->arc(x, y, 1., 0.0, 2.0 * M_PI);
    (*ptcr)->move_to(x, y);
    (*ptcr)->stroke();
    }
}

void dessin_carre(double x, double y, int state)
{
    choix_couleur(BLEU);

    if(state == 0)
        choix_couleur(NOIR);

    (*ptcr)->rectangle(x, y, 3., 3.);
    (*ptcr)->move_to(x, y);
    (*ptcr)->stroke();
}

void dessin_segment(double x1, double y1, double x2, double y2, int state)
{
    choix_couleur(BLEU);

    if(state == 0){
        choix_couleur(NOIR); }

    (*ptcr)->move_to(x1, y1);
    (*ptcr)->line_to(x2, y2);
    (*ptcr)->stroke();
}
