#include "rgl_math.h"
#include "rgl_defines.h"

Vec3f::Vec3f(const Vec4f& v4)
{
    x = v4.x;
    y = v4.y;
    z = v4.z;
}

Vec3f Cross(const Vec3f& a, const Vec3f& b)
{
    return Vec3f{ a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

float Length(const Vec3f& v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

float Length(const Vec4f& v)
{
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w);
}

Vec3f Normalize(const Vec3f& v)
{
    float len = Length(v);
    return v / len;
}

float Dot(const Vec3f& a, const Vec3f& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

Vec4f Normalize(const Vec4f& v)
{
    float len = Length(v);
    if ( len > RAMEN_EPSILON )
    {
        return v / len;
    }
    return v;
}

float Dot(const Vec4f& a, const Vec4f& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

/* Create Inverse of 4x4 Matrix.
 * Code from 'Foundations of Game Engine programming Vol.1'
 * by Eric Lengyel.
 */
Mat4f Inverse(const Mat4f& m)
{
    const Vec3f& a = reinterpret_cast<const Vec3f&>(m[ 0 ]);
    const Vec3f& b = reinterpret_cast<const Vec3f&>(m[ 1 ]);
    const Vec3f& c = reinterpret_cast<const Vec3f&>(m[ 2 ]);
    const Vec3f& d = reinterpret_cast<const Vec3f&>(m[ 3 ]);

    const float& x = m(3, 0);
    const float& y = m(3, 1);
    const float& z = m(3, 2);
    const float& w = m(3, 3);

    Vec3f s = Cross(a, b);
    Vec3f t = Cross(c, d);
    Vec3f u = a * y - b * x;
    Vec3f v = c * w - d * z;

    float invDet = 1.0f / (Dot(s, v) + Dot(t, u));
    s *= invDet;
    t *= invDet;
    u *= invDet;
    v *= invDet;

    Vec3f r0 = Cross(b, v) + t * y;
    Vec3f r1 = Cross(v, a) - t * x;
    Vec3f r2 = Cross(d, u) + s * w;
    Vec3f r3 = Cross(u, c) - s * z;

    return Mat4f(r0.x,
                 r0.y,
                 r0.z,
                 -Dot(b, t),
                 r1.x,
                 r1.y,
                 r1.z,
                 Dot(a, t),
                 r2.x,
                 r2.y,
                 r2.z,
                 -Dot(d, s),
                 r3.x,
                 r3.y,
                 r3.z,
                 Dot(c, s));
}

/* Builds a versor from given axis and rotation angle (in degrees), given in degrees.
 * A versor is the expression of a rotation in a '4D rotational vector'.
 * Note that this computes a unit-quaternion.
 * Also note that mathematicians don't like it when we call Quaternions Vectors!
 */
Quat AngleAxis(const float& x, const float& y, const float& z, const float& angleDgr)
{
    Quat result{};

    const float& angleRad     = TO_RAD(angleDgr);
    const float& halfTheta    = angleRad * 0.5f;
    const float& sinHalfTheta = sinf(halfTheta);
    result.x                  = sinHalfTheta * x;
    result.y                  = sinHalfTheta * y;
    result.z                  = sinHalfTheta * z;
    result.w                  = cosf(halfTheta);

    return result;
}

Quat AngleAxis(const Vec3f& v, const float& angleDgr)
{
    return AngleAxis(v.x, v.y, v.z, angleDgr);
}

Mat4f ToMat4f(const Quat& qIn)
{
    Quat        q    = qIn;
    const float len2 = q.Length2();
    if ( fabs(len2 - 1.0f) > RAMEN_EPSILON )
    {
        q.Normalize();
    }
    q.Normalize(); // FIX: Sanity check. Remove later.

    const float& x  = q.x;
    const float& y  = q.y;
    const float& z  = q.z;
    const float& w  = q.w;
    const float  x2 = x * x;
    const float  y2 = y * y;
    const float  z2 = z * z;

    Mat4f result{ Vec4f{ 1.0f - 2.0f * y2 - 2.0f * z2, 2.0f * x * y + 2.0f * w * z, 2.0f * x * z - 2.0f * w * y, 0.0f },
                  Vec4f{ 2.0f * x * y - 2.0f * w * z, 1.0f - 2.0f * x2 - 2.0f * z2, 2.0f * y * z + 2.0f * w * x, 0.0f },
                  Vec4f{ 2.0f * x * z + 2.0f * w * y, 2.0f * y * z - 2.0f * w * x, 1.0f - 2.0f * x2 - 2.0f * y2, 0.0f },
                  Vec4f{ 0.0f, 0.0f, 0.0f, 1.0f } };

    return result;
}

Mat4f LookAt(const Vec3f& position, const Vec3f& target, const Vec3f& up)
{
    Vec3f t = -position;
    Vec3f f = Normalize(target - position);
    Vec3f r = Normalize(Cross(f, up));
    Vec3f u = Cross(r, f);
    Mat4f R = Mat4f::Identity();
    R(0, 0) = r.x;
    R(0, 1) = r.y;
    R(0, 2) = r.z;
    R(1, 0) = u.x;
    R(1, 1) = u.y;
    R(1, 2) = u.z;
    /* Note the flipped values of the forward vector.
     * This does *NOT* mean, though, that this matrix
     * is becoming left-handed.
     */
    R(2, 0) = -f.x;
    R(2, 1) = -f.y;
    R(2, 2) = -f.z;

    Mat4f T = Translate(t);

    return R * T;
}

/* Creates a right-handed, y-up, perspective projection matrix. */
Mat4f PerspectiveProjection(const float& fovy, const float& aspect, const float& near, const float& far)
{
    float d = 1.0f / tanf(fovy * 0.5f);
    float s = far - near;
    return Mat4f(d / aspect,
                 0.0f,
                 0.0f,
                 0.0f,
                 0.0f,
                 d,
                 0.0f,
                 0.0f,
                 0.0f,
                 0.0f,
                 -(near + far) / s,
                 -1.0f,
                 0.0f,
                 0.0f,
                 -(2.0f * near * far) / s,
                 0.0f);
}

Mat4f Translate(const Vec3f& v)
{
    Mat4f result = Mat4f::Identity();
    result[ 3 ]  = Vec4f{ v, 1.0f };

    return result;
}

void Translate(Mat4f& M, const Vec3f& v)
{
    const Mat4f T = Translate(v);
    M             = T * M;
}

Mat4f Scale(const Vec3f& v)
{
    Mat4f result = Mat4f::Identity();
    result[ 0 ][ 0 ] *= v.x;
    result[ 1 ][ 1 ] *= v.y;
    result[ 2 ][ 2 ] *= v.z;

    return result;
}

void Scale(Mat4f& M, const Vec3f& v)
{
    const Mat4f S = Scale(v);
    M             = S * M;
}

Mat4f Rotate(const Vec3f& axis, const float& angleDgr)
{
    const Quat q = AngleAxis(axis, angleDgr);
    return ToMat4f(q);
}

void Rotate(Mat4f& M, const Vec3f& axis, const float& angleDgr)
{
    const Mat4f R = Rotate(axis, angleDgr);
    M             = R * M;
}

Mat4f Rotate(const Mat4f& M, const Vec3f& axis, const float& angleDgr)
{
    const Mat4f R = Rotate(axis, angleDgr);
    return R * M;
}

/* TODO: Rotate matrix around all three angles. */
// void Rotate(Mat4f& M, const Vec3f& anglesDgr)
// {
//     const Quat qX   = AngleAxis(RAMEN_WORLD_RIGHT, anglesDgr.x);
//     const Quat qY   = AngleAxis(RAMEN_WORLD_UP, anglesDgr.y);
//     const Quat qZ   = AngleAxis(RAMEN_WORLD_FORWARD, anglesDgr.z);
//     const Quat qRot = qY * qX * qZ;
//     M               = ToMat4f(qRot) * M;
// }

// void Rotate(Mat4f& M, const Mat4f& frame, const Vec3f& anglesDgr)
// {
//
// }

/* Assumes q is noramlized. */
Vec3f Rotate(const Quat& q, const Vec3f& v)
{
    /* pure quaternion from vector */
    const Quat p = Quat{ v.x, v.y, v.z, 0.0f };
    /* conjugate */
    const Quat cq = Quat{ -q.x, -q.y, -q.z, q.w };
    /* pure quaternion rotated. Its components yield rotated v */
    const Quat pp = q * p * cq;
    return Vec3f{ pp.x, pp.y, pp.z };
}

/* Scalar * Vector */
Vec3f operator*(const float& s, const Vec3f& v)
{
    return v * s;
}
