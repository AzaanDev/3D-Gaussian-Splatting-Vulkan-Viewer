#pragma once

#include <glm/glm.hpp>

struct UniformBufferObject 
{
	glm::mat4 view;
	glm::mat4 proj;	
	glm::vec3 cam_pos;
	float tan_fovy;
	float tan_fovx;
	float focal_y;
	float focal_x;
	int sh_dim;
	int render_mode;
};
