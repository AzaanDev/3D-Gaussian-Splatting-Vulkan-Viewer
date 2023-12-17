#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

constexpr uint32_t WIDTH = 1280;
constexpr uint32_t HEIGHT = 720;

class Camera {
public:
	Camera();
	~Camera() = default;

	inline void SetProjection();
	inline void SetPosition(glm::vec3 position);
	inline glm::vec3 GetPosition();
	inline glm::mat4 GetView();
	inline glm::mat4 GetProjection();
	inline float GetFovy();
	inline float GetNearClip();
	inline float GetFarClip();
private:
	void UpdateViewMatrix();

	glm::vec3 position;
	glm::vec3 target;
	glm::vec3 up;
	glm::mat4 view;
	glm::mat4 projection;
	float znear, zfar, fovy, yaw, pitch, aspect;
};

// glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget);

inline void Camera::SetProjection()
{
	projection = glm::perspective(fovy, aspect, znear, zfar);
}

inline void Camera::SetPosition(glm::vec3 position)
{
	this->position = position;
}

inline glm::vec3 Camera::GetPosition()
{
	return position;
}

inline glm::mat4 Camera::GetView()
{
	return view;
}

inline glm::mat4 Camera::GetProjection()
{
	return projection;
}

inline float Camera::GetFovy()
{
	return fovy;
}

inline float Camera::GetNearClip()
{
	return znear;
}

inline float Camera::GetFarClip()
{
	return zfar;
}

inline void Camera::UpdateViewMatrix()
{
}

