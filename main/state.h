#pragma once
#pragma once

enum class State
{
	MovingLeft,
	MovingRight,
	StoppingLeft,
	StoppingRight,
	Stopped
};

enum class SpeedState
{
	Normal,
	Rapid
};

enum class UIState
{
	MovingLeft,
	MovingRight,
	Stopping,
	Stopped
};