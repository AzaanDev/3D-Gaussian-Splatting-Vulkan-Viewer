#pragma once

#include <glm/glm.hpp>

struct UniformBufferObject 
{
	glm::mat4 view;
	glm::mat4 proj;	
	glm::vec3 cam_pos;
	float focal_x;
	float focal_y;
	float tan_fovx;
	float tan_fovy;
	int sh_dim;
	int render_mode;
};
