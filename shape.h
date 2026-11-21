// Antoine Bernadat, Timothy Biersohn
#ifndef SHAPE_H
#define SHAPE_H

constexpr double epsil_zero(0.5);
struct S2d 
{
    double x = 0.;
    double y = 0.;
};

class Segment 
{
public:
    Segment();
    Segment(double x1, double y1, double angle, double longueur);

    S2d get_base() const;
    double get_angle() const;
    double get_length() const;
    S2d get_extr() const;

    void setAngle(double angle);
    void setLength(double length);

    S2d calculate_extr();

    double angle_diff(const Segment& seg2);

private:
    S2d base_;
    double angle_;
    double length_;
    S2d extr_;
};

void normalisation(double &angle);
double use_epsil(bool enable);

bool superposition(Segment seg1, Segment seg2);

bool doIntersect(const Segment& seg1, const Segment &seg2, bool epsil_use);
bool onSegment(const S2d& p, const S2d& q, const S2d& r, bool epsil_use);
int orientation(const S2d& p, const S2d& q, const S2d& r, bool epsil_use);

void draw_shape_circle(double x, double y, double rayon);
void draw_shape_square(double x, double y, int state);
void draw_shape_rectangle(double x, double y, double extr_x, double extr_y, int state);

#endif