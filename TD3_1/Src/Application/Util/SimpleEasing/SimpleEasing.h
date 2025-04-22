#pragma once

// Engine
#include <Math/Easing.h>
#include <Math/Vector/Vector3.h>

// C++
#include <functional>

class SimpleEasing {
public:
	using EasingFunction = std::function<float(float)>;

	/// <summary>
	/// 値を変化させる汎用関数
	/// </summary>
	/// <param name="value">変化させる値</param>
	/// <param name="start">初期値</param>
	/// <param name="end">最終値</param>
	/// <param name="easingFunction">イージング関数</param>
	/// <param name="duration">時間（秒）</param>
	static void Animate(float& value, float start, float end, EasingFunction easingFunction, float duration);

	/// <summary>
	/// 値を変化させる汎用関数
	/// </summary>
	/// <param name="value">変化させる値</param>
	/// <param name="start">初期値</param>
	/// <param name="end">最終値</param>
	/// <param name="easingFunction">イージング関数</param>
	/// <param name="duration">時間（秒）</param>
	static void Animate(Vector3& value, Vector3 start, Vector3 end, EasingFunction easingFunction, float duration);
};