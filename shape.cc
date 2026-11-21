// Antoine Bernadat, Timothy Biersohn
#include <cmath>
#include <iostream>
#include <vector>
#include "shape.h"
#include "graphic.h"

using namespace std;

constexpr double square_center(1.5); // Demi-côté du carré représentant la base du corail

Segment::Segment()
    : base_(), angle_(), length_(), extr_() {}

Segment::Segment(double x1, double y1, double angle, double longueur)
    : base_{x1, y1}, angle_(angle), length_(longueur) 
    {
        extr_ = calculate_extr();
    }

S2d Segment::get_base() const
{
    return base_;
}

S2d Segment::get_extr() const 
{
    return extr_;
}

double Segment::get_angle() const 
{
    return angle_;
}

double Segment::get_length() const 
{
    return length_;
}

S2d Segment::calculate_extr() 
{
    extr_.x = base_.x + (length_ * cos(angle_));
    extr_.y = base_.y + (length_ * sin(angle_));
    return extr_;
}

double Segment::angle_diff(const Segment& seg2)
{
    double phi = M_PI + angle_ - seg2.get_angle();
    normalisation(phi);
    return phi;
}

bool superposition(Segment seg1, Segment seg2)
{
    if (std::abs(seg1.angle_diff(seg2)) <= 1e-10)
    {
        return true;
    }
    else 
    {
        return false;
    }
}

int orientation(const S2d& p, const S2d& q, const S2d& r, bool epsil_use)
{
    const double length = std::hypot(q.x - p.x, q.y - p.y);
    if (length <= 1e-12) return 0;
    const double distance = ((q.x-p.x)*(r.y-p.y)-(q.y-p.y)*(r.x-p.x))/length;
    if (std::abs(distance) <= std::max(1e-10, use_epsil(epsil_use))) return 0;
    return distance > 0 ? 1 : 2;
}

bool onSegment(const S2d& p, const S2d& q, const S2d& r, bool epsil_use)
{
    const double length = std::hypot(r.x-p.x, r.y-p.y);
    const double tolerance = std::max(1e-10, use_epsil(epsil_use));
    if (length <= 1e-12) return std::hypot(q.x-p.x,q.y-p.y) <= tolerance;
    const double projection = ((q.x-p.x)*(r.x-p.x)+(q.y-p.y)*(r.y-p.y))/length;
    return projection >= -tolerance && projection <= length+tolerance;
}

// Teste l’intersection ou la proximité de deux segments.
bool doIntersect(const Segment& seg1, const Segment& seg2, bool epsil_use) 
{
    S2d p1 = seg1.get_base();
    S2d q1 = seg1.get_extr();
    S2d p2 = seg2.get_base();
    S2d q2 = seg2.get_extr();

    int o1 = orientation(p1, q1, p2, epsil_use);
    int o2 = orientation(p1, q1, q2, epsil_use);
    int o3 = orientation(p2, q2, p1, epsil_use);
    int o4 = orientation(p2, q2, q1, epsil_use);

    if (o1 && o2 && o3 && o4 && o1 != o2 && o3 != o4) 
    {
        return true;
    }
    if (o1 == 0 && onSegment(p1, p2, q1, epsil_use)) 
    {
        return true;
    }
    if (o2 == 0 && onSegment(p1, q2, q1, epsil_use)) 
    {
        return true; 
    }
    if (o3 == 0 && onSegment(p2, p1, q2, epsil_use)) 
    {
        return true; 
    }
    if (o4 == 0 && onSegment(p2, q1, q2, epsil_use)) 
    {
        return true; 
    }
    return false; 
}

// Active la marge géométrique pendant la simulation.
double use_epsil(bool enable)
{
    if (enable)
    {
        return epsil_zero;
    }
    return 0.;
}

// Dessine une algue ou un scavenger.
void draw_shape_circle(double x, double y, double rayon)
{
    if (rayon == 0)
    {
        dessin_cercle(x, y, 0, 1);
    }
    else
    {
        dessin_cercle(x, y, rayon, 0);
    }
}

// Dessine la base du corail, centrée sur sa position.
void draw_shape_square(double x, double y, int state)
{
    dessin_carre(x - square_center, y - square_center, state);
}

// Dessine un segment dans les coordonnées du modèle.
void draw_shape_rectangle(double x, double y, double extr_x, double extr_y, int state)
{
    dessin_segment(x, y, extr_x, extr_y, state);
}

void Segment::setAngle(double angle)
{
    normalisation(angle);
    angle_ = angle;
    extr_ = calculate_extr();
}

void Segment::setLength(double length)
{
    length_ = length;
    extr_ = calculate_extr();
}

// Ramène l’angle dans l’intervalle [-pi, pi].
void normalisation(double& angle)
{
    while ((angle < (-M_PI)) || (angle > M_PI))
    {
        if(angle < (-M_PI))
        {
            angle += 2 * (M_PI); 
        }
        if(angle > M_PI) 
        {
            angle -= 2 * M_PI; 
        }
    }
}