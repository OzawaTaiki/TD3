#include "MyMath.h"

Vector3 GetForwardFromYRotation(const Quaternion& q)
{
    float yAngle = 2.0f * std::atan2(q.y, q.w);

    return Vector3(std::sin(yAngle), 0.0f, std::cos(yAngle));
}
