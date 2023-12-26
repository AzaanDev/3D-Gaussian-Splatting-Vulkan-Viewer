#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>

#include<vector>
#include<array>

struct Gaussian {
    glm::vec3 position;
    glm::vec3 normal;
    std::array<float, 48> sh;
    float opacity;
    glm::vec3 scale;
    glm::vec4 rotation;
};


struct GaussianList {
    std::vector<glm::vec4> rotations;
    std::vector<glm::vec3> positions;
    std::vector<std::array<float, 48>>  shs;
    std::vector<float> opacities;
    std::vector<glm::vec3> scales;
    std::vector<std::array<float, 6>> cov3ds;
};


GaussianList LoadPly(const std::string& file_name);
GaussianList GenerateTestGaussians();

// Move to Compute Shader
std::vector<std::array<float, 6>> ComputeCov3D(const std::vector<glm::vec3>& scales, const std::vector<glm::vec4>& rotations, float scale_modifier = 1.0f);
std::array<float, 6>  ComputeCov3D(glm::vec3 scale, glm::vec4 rotation, float scale_modifier = 1.0f);
std::vector<int> SortGaussians(const GaussianList& list);
