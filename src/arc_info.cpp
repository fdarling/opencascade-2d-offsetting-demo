#include "arc_info.h"

#include <cmath>

void get_arc_info(const gp_Pnt & center, const gp_Pnt & f, const gp_Pnt & m, const gp_Pnt & l, double& angle_start, double& angle_end)
{
    angle_start = atan2(f.Y() - center.Y(), f.X() - center.X());
    double angle_mid   = atan2(m.Y() - center.Y(), m.X() - center.X());
    angle_end   = atan2(l.Y() - center.Y(), l.X() - center.X());
#ifdef DEV_LOG
    printf("angle_start: %lf angle_mid: %lf angle_end: %lf diff: %lf\n", angle_start, angle_mid, angle_end, abs(angle_start - angle_end));
#endif
    double v = abs(angle_start - angle_end);
    double t = 0.00001;
    if ((v > t) && (v < 2 * M_PI - t))
    {
        //puts("CASE A");
        if (angle_start < 0)
        {
            angle_start += 2 * M_PI;
        }
        if (angle_end < 0)
        {
            angle_end += 2 * M_PI;
        }
        if (angle_start > angle_end)
        {
            std::swap(angle_start, angle_end);
        }
        if (angle_mid < 0)
        {
            angle_mid += 2 * M_PI;
        }

        if (angle_mid < angle_start || angle_mid > angle_end)
        {
            std::swap(angle_start, angle_end);
        }
    }
    else
    {
        //puts("CASE B");
        angle_start = 0;
        angle_end = 2 * M_PI;
    }
}
