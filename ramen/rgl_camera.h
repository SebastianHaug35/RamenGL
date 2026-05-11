#ifndef RGL_CAMERA_H
#define RGL_CAMERA_H

#include "rgl_math.h"

class Camera
{
  public:
    Camera(const Vec3f& position)
    {
        m_Position     = position;
        m_Forward      = RAMEN_CAMERA_FORWARD;
        m_Up           = RAMEN_WORLD_UP;
        m_Right        = RAMEN_WORLD_RIGHT;
        m_qOrientation = AngleAxis(m_Forward, 0.0f);
    }

    const Vec3f& GetPosition() const
    {
        return m_Position;
    }

    Vec3f& Position()
    {
        return m_Position;
    }

    const Vec3f& GetForward() const
    {
        return m_Forward;
    }

    const Vec3f& GetUp() const
    {
        return m_Up;
    }

    const Vec3f GetRight() const
    {
        return Normalize(Cross(m_Forward, m_Up));
    }

    void Move(const Vec3f& offset)
    {
        m_Position = m_Position + offset;
    }

    void MoveForward(const float& distance)
    {
        Move(m_Forward * distance);
    }

    void MoveRight(const float& distance)
    {
        Move(GetRight() * distance);
    }

    void MoveUp(const float& distance)
    {
        Move(m_Up * distance);
    }

    /* Orient camera so that it looks at target. */
    void Orient(const Vec3f target)
    {
        m_Forward = Normalize(target - m_Position);
        m_Right   = Cross(m_Forward, RAMEN_WORLD_UP);
        m_Up      = Cross(m_Right, m_Forward);
    }

    /* NOTE: Angle is negated as the camera's forward is facing
     * to -z and reverses the expected rotation direction (CCW)
     * in worldspace. So, in order to transform the camera
     * like other objects in the world (via a model matrix)
     * this adjustment is being made to the angle.
     *
     * UPDATE (4/27/2026): This is no longer the case since
     * there was a bug in the lookAt() function where
     * rows and columns were swapped during construction.
     */
    void RotateAroundWorldUp(const float& angle)
    {
        Quat q         = AngleAxis(RAMEN_WORLD_UP, angle);
        m_qOrientation = q * m_qOrientation;
        m_qOrientation.Normalize();
        m_Forward = Rotate(m_qOrientation, RAMEN_CAMERA_FORWARD);
        m_Up      = Rotate(m_qOrientation, RAMEN_WORLD_UP);
        m_Right   = Rotate(m_qOrientation, RAMEN_WORLD_RIGHT);
    }

    void RotateAroundUp(const float& angle)
    {
        Quat q         = AngleAxis(m_Up, angle);
        m_qOrientation = q * m_qOrientation;
        m_qOrientation.Normalize();
        m_Forward = Rotate(m_qOrientation, RAMEN_CAMERA_FORWARD);
        m_Right   = Rotate(m_qOrientation, RAMEN_WORLD_RIGHT);
    }

    void RotateAroundSide(const float& angle)
    {
        Quat q         = AngleAxis(GetRight(), angle);
        m_qOrientation = q * m_qOrientation;
        m_qOrientation.Normalize();
        m_Forward = Rotate(m_qOrientation, RAMEN_CAMERA_FORWARD);
        m_Up      = Rotate(m_qOrientation, RAMEN_WORLD_UP);
    }

    void RotateAroundSide(const Quat& q)
    {
        m_qOrientation = q * m_qOrientation;
        m_qOrientation.Normalize();
        m_Forward = Rotate(m_qOrientation, RAMEN_CAMERA_FORWARD);
        m_Up      = Rotate(m_qOrientation, RAMEN_WORLD_UP);
    }

    void RotateAroundForward(const float& angle)
    {
        Quat q         = AngleAxis(m_Forward, angle);
        m_qOrientation = q * m_qOrientation;
        m_qOrientation.Normalize();
        m_Up    = Rotate(m_qOrientation, RAMEN_WORLD_UP);
        m_Right = Rotate(m_qOrientation, RAMEN_WORLD_RIGHT);
    }

    void Roll(const float& angle)
    {
        RotateAroundForward(angle);
    }

    void Pitch(const float& angle)
    {
        RotateAroundSide(angle);
    }

    void Yaw(const float& angle)
    {
        RotateAroundUp(angle);
    }

    void DollyForward(const float& amount)
    {
        m_Position = m_Position + (amount * m_Forward);
    }

    void DollySide(const float& amount)
    {
        m_Position = m_Position + (amount * m_Right);
    }

  private:
    Vec3f m_Position;
    Vec3f m_Forward;
    Vec3f m_Up;
    Vec3f m_Right;
    Quat  m_qOrientation;
};

#endif
