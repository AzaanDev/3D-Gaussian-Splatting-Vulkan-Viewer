#include "gaussian.h"

#include <iostream>

GaussianList LoadPly(const std::string& file_name) {
	std::vector<Gaussian> gaussians;
	GaussianList list;
	FILE* file = fopen(file_name.c_str(), "rb");

	if (file == NULL) {
		std::cout << "Error opening file" << std::endl;
		exit(1);
	}

	for (int i = 0; i < 66; ++i) {
		int c;
		while ((c = fgetc(file)) != EOF && c != '\n') {
		}
	}

	long start_pos = ftell(file);
	fseek(file, 0, SEEK_END);
	long size = ftell(file) - start_pos;
	fseek(file, start_pos, SEEK_SET);

	long count = size / sizeof(Gaussian);
	gaussians.resize(count);
	fread(gaussians.data(), sizeof(Gaussian), count, file);
	fclose(file);

	list.rotations.resize(count);
	list.positions.resize(count);
	list.shs.resize(count);
	list.opacities.resize(count);
	list.scales.resize(count);

	for (int i = 0; i < count; i++) {
		list.positions[i] = (gaussians[i].position);

		float length2 = 0;
		for (int j = 0; j < 4; j++)
			length2 += gaussians[i].rotation[j] * gaussians[i].rotation[j];
		float length = sqrt(length2);
		for (int j = 0; j < 4; j++)
			list.rotations[i][j] = gaussians[i].rotation[j] / length;
		
		list.opacities[i] = 1.0f / (1.0f + exp(-gaussians[i].opacity));

		list.scales[i].x = exp(gaussians[i].scale.x);
		list.scales[i].y = exp(gaussians[i].scale.y);
		list.scales[i].z = exp(gaussians[i].scale.z);


		list.shs[i][0] = gaussians[i].sh[0];
		list.shs[i][1] = gaussians[i].sh[1];
		list.shs[i][2] = gaussians[i].sh[2];
		for (int j = 1; j < 16; j++)
		{
			list.shs[i][j * 3 + 0] = gaussians[i].sh[(j - 1) + 3];
			list.shs[i][j * 3 + 1] = gaussians[i].sh[(j - 1) + 16 + 2];
			list.shs[i][j * 3 + 2] = gaussians[i].sh[(j - 1) + 2 * 16 + 1];
		}

	}

	list.cov3ds = ComputeCov3D(list.scales, list.rotations);
	return list;
}

GaussianList GenerateTestGaussians()
{
	GaussianList gaussians;
	gaussians.rotations = { {1.f,0.f,0.f, 0.f}, {1.f,0.f,0.f, 0.f}, {1.f,0.f,0.f, 0.f}, {1.f, 0.f, 0.f, 0.f} };
	gaussians.positions = { glm::vec3({0.f, 0.f, 0.f}), glm::vec3({1.f, 0.f, 0.f}), glm::vec3({0.f, 1.f, 0.f}), glm::vec3({0.f, 0.f, 1.f}) };
	gaussians.scales = { {0.03f, 0.03f, 0.03f}, {0.2f, 0.03f, 0.03f}, {0.03f, 0.2f, 0.03f}, {0.03f, 0.03f, 0.2f} };
	gaussians.shs = { {1.f, 0.f, 1.f}, {1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f} };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 3; j++) {
			gaussians.shs[i][j] = (gaussians.shs[i][j] - 0.5) / 0.28209;
		}
	}
	gaussians.opacities = { 1, 1, 1, 1 };
	gaussians.cov3ds = ComputeCov3D(gaussians.scales, gaussians.rotations);
	return gaussians;
}

/*
Move to GPU for dynamic resizing of splats
*/
std::vector<std::array<float, 6>> ComputeCov3D(const std::vector<glm::vec3>& scales, const std::vector<glm::vec4>& rotations, float scale_modifier)
{
	std::vector<std::array<float, 6>> cov3ds;
	cov3ds.reserve(scales.size());

	for (int i = 0; i < scales.size(); i++) {
		std::array<float, 6> cov;
		glm::mat3 S = glm::mat3(1.0f);
		S[0][0] = scale_modifier * scales[i].x;
		S[1][1] = scale_modifier * scales[i].y;
		S[2][2] = scale_modifier * scales[i].z;

		float r = rotations[i].x;
		float x = rotations[i].y;
		float y = rotations[i].z;
		float z = rotations[i].w;

		glm::mat3 R = glm::mat3(
			1.f - 2.f * (y * y + z * z), 2.f * (x * y - r * z), 2.f * (x * z + r * y),
			2.f * (x * y + r * z), 1.f - 2.f * (x * x + z * z), 2.f * (y * z - r * x),
			2.f * (x * z - r * y), 2.f * (y * z + r * x), 1.f - 2.f * (x * x + y * y)
		);

		glm::mat3 M = S * R;
		glm::mat3 sigma = transpose(M) * M;
		cov[0] = sigma[0][0];
		cov[1] = sigma[0][1];
		cov[2] = sigma[0][2];
		cov[3] = sigma[1][1];
		cov[4] = sigma[1][2];
		cov[5] = sigma[2][2];

		cov3ds.push_back(cov);
	}
	return cov3ds;
}


std::array<float, 6> ComputeCov3D(glm::vec3 scale, glm::vec4 rotation, float scale_modifier)
{
	glm::mat3 S = glm::mat3(1.0f);
	std::array<float, 6> cov;
	S[0][0] = scale_modifier * scale.x;
	S[1][1] = scale_modifier * scale.y;
	S[2][2] = scale_modifier * scale.z;

	float r = rotation.x;
	float x = rotation.y;
	float y = rotation.z;
	float z = rotation.w;

	glm::mat3 R = glm::mat3(
		1.f - 2.f * (y * y + z * z), 2.f * (x * y - r * z), 2.f * (x * z + r * y),
		2.f * (x * y + r * z), 1.f - 2.f * (x * x + z * z), 2.f * (y * z - r * x),
		2.f * (x * z - r * y), 2.f * (y * z + r * x), 1.f - 2.f * (x * x + y * y)
	);

	glm::mat3 M = S * R;
	glm::mat3 sigma = transpose(M) * M;

	cov[0] = sigma[0][0];
	cov[1] = sigma[0][1];
	cov[2] = sigma[0][2];
	cov[3] = sigma[1][1];
	cov[4] = sigma[1][2];
	cov[5] = sigma[2][2];

	return cov;
}
