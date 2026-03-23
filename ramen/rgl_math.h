#ifndef RGL_MATH_H
#define RGL_MATH_H

#include <assert.h>

struct Vec4f;

struct Vec3f
{
    float x;
    float y;
    float z;

    Vec3f(float _x, float _y, float _z)
    {
        x = _x;
        y = _y;
        z = _z;
    }

    Vec3f(float s)
    {
        x = s;
        y = s;
        z = s;
    }

    Vec3f()
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
    }

    Vec3f(const Vec4f& v4);

    const float operator[](const int index) const
    {
        assert(index < 3);
        return (&x)[ index ];
    }

    float& operator[](const int index)
    {
        assert(index < 3);
        return (&x)[ index ];
    }

    Vec3f operator-(const Vec3f& right) const
    {
        return Vec3f{ x - right.x, y - right.y, z - right.z };
    }

    const Vec3f operator-() const
    {
        return Vec3f{ -x, -y, -z };
    }

    Vec3f operator+(const Vec3f& right) const
    {
        return Vec3f{ x + right.x, y + right.y, z + right.z };
    }

    Vec3f operator/(float s) const
    {
        return Vec3f{ x / s, y / s, z / s };
    }

    Vec3f operator*(const Vec3f& right) const
    {
        return Vec3f{ x * right.x, y * right.y, z * right.z };
    }

    /* Vector * Scalar */
    Vec3f operator*(const float& s) const
    {
        return Vec3f{ s * x, s * y, s * z };
    }

    Vec3f& operator*=(const float& s)
    {
        x *= s;
        y *= s;
        z *= s;

        return *this;
    }
};

struct Vec4f
{
    float x;
    float y;
    float z;
    float w;

    Vec4f(float _x, float _y, float _z, float _w)
    {
        x = _x;
        y = _y;
        z = _z;
        w = _w;
    }

    Vec4f(float s)
    {
        x = s;
        y = s;
        z = s;
        w = s;
    }

    Vec4f()
    {
        x = 0.0f;
        y = 0.0f;
        z = 0.0f;
        w = 0.0f;
    }

    Vec4f(const Vec3f& v3)
    {
        x = v3.x;
        y = v3.y;
        z = v3.z;
        w = 1.0f;
    }

    Vec4f(const Vec3f& v3, const float& s)
    {
        x = v3.x;
        y = v3.y;
        z = v3.z;
        w = s;
    }

    const float& operator[](const int index) const
    {
        assert(index < 4);
        return (&x)[ index ];
    }

    float& operator[](const int index)
    {
        assert(index < 4);
        return (&x)[ index ];
    }

    Vec4f operator-(const Vec4f& right) const
    {
        return Vec4f{ x - right.x, y - right.y, z - right.z, w - right.w };
    }

    const Vec4f operator-() const
    {
        return Vec4f{ -x, -y, -z, -w };
    }

    Vec4f operator+(const Vec4f& right) const
    {
        return Vec4f{ x + right.x, y + right.y, z + right.z, w + right.w };
    }

    Vec4f operator/(float s) const
    {
        return Vec4f{ x / s, y / s, z / s, w / s };
    }

    Vec4f operator*(const Vec4f& right) const
    {
        return Vec4f{ x * right.x, y * right.y, z * right.z, w * right.w };
    }

    /* Vector * Scalar */
    Vec4f operator*(const float& s) const
    {
        return Vec4f{ s * x, s * y, s * z, s * w };
    }
};

#endif
