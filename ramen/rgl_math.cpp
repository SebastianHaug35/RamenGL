#include "rgl_math.h"

Vec3f::Vec3f(const Vec4f& v4)
{
    x = v4.x;
    y = v4.y;
    z = v4.z;
}
