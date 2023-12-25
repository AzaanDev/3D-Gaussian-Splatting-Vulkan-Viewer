#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

#include <vector>
#include <cmath>

#include "gaussian.h"
#include "camera.h"


glm::vec3 ComputeCov2D(glm::vec3 mean, float focal_x, float focal_y, float tan_fovx, float tan_fovy, const std::array<float, 6>& cov3D, const glm::mat4 view_matrix) {
    glm::vec3 t = view_matrix * glm::vec4(mean, 1.0f);
    const float limx = 1.3f * tan_fovx;
    const float limy = 1.3f * tan_fovy;
    const float txtz = t.x / t.z;
    const float tytz = t.y / t.z;
    t.x = std::min(limx, std::max(-limx, txtz)) * t.z;
    t.y = std::min(limy, std::max(-limy, tytz)) * t.z;

    glm::mat3 J = glm::mat3(
        focal_x / t.z, 0.0f, -(focal_x * t.x) / (t.z * t.z),
        0.0f, focal_y / t.z, -(focal_y * t.y) / (t.z * t.z),
        0, 0, 0);

    glm::mat3 W = glm::mat3(
        view_matrix[0][0], view_matrix[1][0], view_matrix[2][0],
        view_matrix[0][1], view_matrix[1][1], view_matrix[2][1],
        view_matrix[0][2], view_matrix[1][2], view_matrix[2][2]
    );

    glm::mat3 T = W * J;

    glm::mat3 Vrk = glm::mat3(
        cov3D[0], cov3D[1], cov3D[2],
        cov3D[1], cov3D[3], cov3D[4],
        cov3D[2], cov3D[4], cov3D[5]);

    glm::mat3 cov = transpose(T) * transpose(Vrk) * T;

    cov[0][0] += .3f;
    cov[1][1] += .3f;
    return glm::vec3(cov[0][0], cov[0][1], cov[1][1]);
}


TEST(TEST_PROJECTION, TEST_CONIC) {
    const std::vector <glm::vec2> vertices = {
    {-1.f, 1.f},
    {1.0f, 1.f},
    {1.0f, -1.f},
    {-1.0f, -1.0f},
    };

	GaussianList gaussians = GenerateTestGaussians();
	Camera camera;
    auto view = camera.GetView();
    auto proj = camera.GetProjection();
    auto cam_pos = camera.GetPosition();
    auto tan_fovy = tan(camera.GetFovy() * 0.5);
    auto tan_fovx = tan_fovy * WIDTH / HEIGHT;
    auto focal_y = HEIGHT / (2.0f * tan_fovy);
    auto focal_x = WIDTH / (2.0f * tan_fovx);

	std::vector<glm::vec3> cov2ds;
    for (int i = 0; i < 4; i++) {
       auto cov =  ComputeCov2D(gaussians.positions[i], focal_x, focal_y, tan_fovx, tan_fovy, gaussians.cov3ds[i], view);
      
       float det = (cov.x * cov.z - cov.y * cov.y);
       if (det == 0.0f)
           return;
       float det_inv = 1.f / det;
       glm::vec3 conic = { cov.z * det_inv, -cov.y * det_inv, cov.x * det_inv };
     
       /*for (int j = 0; j < 6; j++) {
           std::cout << gaussians.cov3ds[i][j] << " ";
       }
       std::cout << std::endl;
       */
       std::cout << to_string(conic) << std::endl;
   }

	ASSERT_TRUE(true);
}
