#pragma once

// Engine
#include <Features/Sprite/Sprite.h>
#include <Features/Sprite/SpriteManager.h>

class Fade
{
public:
	enum class Status {
		None,
		FadeIn,
		FadeOut,
	};

public:
	void Initialize();
	void Update();
	void Draw();

	void Start(Status status, float duration);
	void Stop();

	bool IsFinished() const;

private:
	std::unique_ptr<Sprite> sprite_;

	Status status_ = Status::None;
	float duration_ = 0.0f;
	float counter_ = 0.0f;
};

