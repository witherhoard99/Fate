#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace Constants
{
	constexpr float STARTING_YAW            =  0.0f ;
	constexpr float STARTING_PITCH          =  0.0f  ;
	constexpr float SPEED                   =  0.01f ;
	constexpr float SENSITIVITY_X           =  300.f ;
	constexpr float SENSITIVITY_Y           =  300.f ;
	constexpr float SPRINT_SPEED_MULTIPLIER =  5.0f ;

	constexpr float RECOIL_Y_MAX = 0.01;
	constexpr float RECOIL_Y_MIN = 0.001;

	constexpr float RECOIL_X_MAX = 0.03f;
	constexpr float RECOIL_X_MIN = -0.03f;

	constexpr float DEADZONE = 0.0002f;

	constexpr float maxDistanceFromGroundToStillBeOnTheGround = 0.1;
	constexpr bool	benchmark = true;
}

#endif //CONSTANTS_H
