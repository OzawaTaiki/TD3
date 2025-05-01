#pragma once
#include <Math/Vector/Vector3.h>
#include <Math/Quaternion/Quaternion.h>

// Y軸回転のみでforwardベクトルを求める
Vector3 GetForwardFromYRotation(const Quaternion& q);