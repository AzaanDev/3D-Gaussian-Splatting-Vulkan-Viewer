#define _USE_MATH_DEFINES

#include "camera.h"

#include <math.h>

Camera::Camera()
{
	position = glm::vec3(0.0f, -1.0f, 3.0f);
	target = glm::vec3(0.0f, 1.0f, 0.0f);
	up = glm::vec3(0.0f, 1.f, 0.0f);
	yaw, pitch = 0.0;
	znear = 0.01;
	zfar = 100;
	fovy = M_PI / 2;
	view = glm::lookAt(position, target, up);

	aspect = (float)1280 / 720;
	SetProjection();
}
