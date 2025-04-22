#include "SimpleEasing.h"

// C++
#include <chrono>
#include <thread>

void SimpleEasing::Animate(float& value, float start, float end, EasingFunction easingFunction, float duration) {
	std::thread([&, start, end, duration, easingFunction]() {
		auto startTime = std::chrono::high_resolution_clock::now();
		// アニメーション終了時刻の計算
		auto endTime = startTime + std::chrono::milliseconds(static_cast<int>(duration * 1000));

		// 現在時刻がアニメーション終了時刻になるまでループ
		while (std::chrono::high_resolution_clock::now() < endTime) {
			auto now = std::chrono::high_resolution_clock::now();
			auto elapsedTime = std::chrono::duration<float>(now - startTime).count();
			float t = elapsedTime / duration;
			// tの値をクランプ
			if (t > 1.0f) 
				t = 1.0f;

			// 遷移途中のvalueを計算
			value = start + (end - start) * easingFunction(t);

			std::this_thread::sleep_for(std::chrono::milliseconds(16)); // 約60fpsで更新
		}

		// valueを最終値に設定
		value = end;
	}).detach(); // スレッドを切り離して非同期処理
}

void SimpleEasing::Animate(Vector3& value, Vector3 start, Vector3 end, EasingFunction easingFunction, float duration) {
	std::thread([&, start, end, duration, easingFunction]() {
		auto startTime = std::chrono::high_resolution_clock::now();
		// アニメーション終了時刻の計算
		auto endTime = startTime + std::chrono::milliseconds(static_cast<int>(duration * 1000));

		// 現在時刻がアニメーション終了時刻になるまでループ
		while (std::chrono::high_resolution_clock::now() < endTime) {
			auto now = std::chrono::high_resolution_clock::now();
			auto elapsedTime = std::chrono::duration<float>(now - startTime).count();
			float t = elapsedTime / duration;
			// tの値をクランプ
			if (t > 1.0f)
				t = 1.0f;

			// 遷移途中のvalueを計算
			value = start + (end - start) * easingFunction(t);

			std::this_thread::sleep_for(std::chrono::milliseconds(16)); // 約60fpsで更新
		}

		// valueを最終値に設定
		value = end;
	}).detach(); // スレッドを切り離して非同期処理
}