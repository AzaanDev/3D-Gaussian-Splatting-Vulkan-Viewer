#include <gtest/gtest.h>
#include <glm/glm.hpp>

#include <vector>
#include <fstream>
#include <iostream>

#include "gaussian.h"

/*
* Copyright (C) 2023, Inria
* https://gitlab.inria.fr/sibr/sibr_core/-/blob/gaussian_code_release_union/src/projects/gaussianviewer/renderer/GaussianView.cpp
*/

template<int D>
struct SHs
{
	float shs[(D + 1) * (D + 1) * 3];
};
struct Scale
{
	float scale[3];
};
struct Rot
{
	float rot[4];
};

template<int D>
struct RichPoint
{
	glm::vec3 pos;
	float n[3];
	SHs<D> shs;
	float opacity;
	Scale scale;
	Rot rot;
};

inline float sigmoid(const float m1)
{
	return 1.0f / (1.0f + exp(-m1));
}

template<int D>
std::vector<RichPoint<D>> loadPly(const char* filename,
	std::vector<glm::vec3>& pos,
	std::vector<SHs<3>>& shs,
	std::vector<float>& opacities,
	std::vector<Scale>& scales,
	std::vector<Rot>& rot,
	glm::vec3& minn,
	glm::vec3& maxx)
{
	std::ifstream infile(filename, std::ios_base::binary);

	if (!infile.good())
		std::cout << "Unable to find model's PLY file, attempted:\n" << filename << std::endl;

	// "Parse" header (it has to be a specific format anyway)
	std::string buff;
	std::getline(infile, buff);
	std::getline(infile, buff);

	std::string dummy;
	std::getline(infile, buff);
	std::stringstream ss(buff);
	int count;
	ss >> dummy >> dummy >> count;

	// Output number of Gaussians contained
	std::cout << "Loading " << count << " Gaussian splats" << std::endl;

	while (std::getline(infile, buff))
		if (buff.compare("end_header") == 0)
			break;

	// Read all Gaussians at once (AoS)
	std::vector<RichPoint<D>> points(count);
	infile.read((char*)points.data(), count * sizeof(RichPoint<D>));

	// Resize our SoA data
	pos.resize(count);
	shs.resize(count);
	scales.resize(count);
	rot.resize(count);
	opacities.resize(count);
	infile.close();
	// Gaussians are done training, they won't move anymore. Arrange
	// them according to 3D Morton order. This means better cache
	// behavior for reading Gaussians that end up in the same tile 
	// (close in 3D --> close in 2D).
	/*
	minn = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
	maxx = -minn;
	for (int i = 0; i < count; i++)
	{
		cwiseMax(maxx, points[i].pos);
		cwiseMin(minn, points[i].pos);
	}

	std::vector<std::pair<uint64_t, int>> mapp(count);
	for (int i = 0; i < count; i++)
	{
		sibr::Vector3f rel = (points[i].pos - minn).array() / (maxx - minn).array();
		sibr::Vector3f scaled = ((float((1 << 21) - 1)) * rel);
		sibr::Vector3i xyz = scaled.cast<int>();

		uint64_t code = 0;
		for (int i = 0; i < 21; i++) {
			code |= ((uint64_t(xyz.x() & (1 << i))) << (2 * i + 0));
			code |= ((uint64_t(xyz.y() & (1 << i))) << (2 * i + 1));
			code |= ((uint64_t(xyz.z() & (1 << i))) << (2 * i + 2));
		}

		mapp[i].first = code;
		mapp[i].second = i;
	}
	auto sorter = [](const std::pair < uint64_t, int>& a, const std::pair < uint64_t, int>& b) {
		return a.first < b.first;
		};
	std::sort(mapp.begin(), mapp.end(), sorter);
	*/
	// Move data from AoS to SoA
	int SH_N = (D + 1) * (D + 1);
	for (int k = 0; k < count; k++)
	{
		int i = k;
		pos[k] = points[k].pos;

		// Normalize quaternion
		float length2 = 0;
		for (int j = 0; j < 4; j++)
			length2 += points[i].rot.rot[j] * points[i].rot.rot[j];
		float length = sqrt(length2);
		for (int j = 0; j < 4; j++)
			rot[k].rot[j] = points[i].rot.rot[j] / length;

		// Exponentiate scale
		for (int j = 0; j < 3; j++)
			scales[k].scale[j] = exp(points[i].scale.scale[j]);

		// Activate alpha
		opacities[k] = sigmoid(points[i].opacity);

		shs[k].shs[0] = points[i].shs.shs[0];
		shs[k].shs[1] = points[i].shs.shs[1];
		shs[k].shs[2] = points[i].shs.shs[2];
		for (int j = 1; j < SH_N; j++)
		{
			shs[k].shs[j * 3 + 0] = points[i].shs.shs[(j - 1) + 3];
			shs[k].shs[j * 3 + 1] = points[i].shs.shs[(j - 1) + SH_N + 2];
			shs[k].shs[j * 3 + 2] = points[i].shs.shs[(j - 1) + 2 * SH_N + 1];
		}
	}
	return points;
}

bool operator==(const Gaussian& lhs, const RichPoint<3>& rhs) {
	bool sh = true;
	for (int i = 0; i < 48; i++) {
		if (lhs.sh[i] != rhs.shs.shs[i]) {
			sh = false;
		}
	}

	return lhs.position == rhs.pos &&
		lhs.normal.x == rhs.n[0] &&
		lhs.normal.y == rhs.n[1] &&
		lhs.normal.z == rhs.n[2] &&
		sh &&
		lhs.opacity == rhs.opacity &&
		lhs.scale.x == rhs.scale.scale[0] &&
		lhs.scale.y == rhs.scale.scale[1] &&
		lhs.scale.z == rhs.scale.scale[2] &&


		lhs.rotation.x == rhs.rot.rot[0];
		lhs.rotation.y == rhs.rot.rot[1];
		lhs.rotation.z == rhs.rot.rot[2];
		lhs.rotation.w == rhs.rot.rot[3];
}

TEST(LoadingPLY, TEST_LOAD_PLY) {
	const std::string file = "C:/Users/kovip/Desktop/3D/gaussian_splatting/tests/static/point_cloud.ply";
	std::vector<glm::vec3> pos;
	std::vector<SHs<3>> shs;
	std::vector<float> opacities;
	std::vector<Scale> scales;
	std::vector<Rot> rot;
	glm::vec3 minn;
	glm::vec3 maxx;

	auto absolute = loadPly<3>(file.c_str(), pos, shs, opacities, scales, rot, minn, maxx);
	auto test = LoadPly(file);
	ASSERT_EQ(absolute.size(), test.positions.size());

	/*
	bool IsEqual = std::equal(absolute.begin(), absolute.end(), test.begin(),
		[](const RichPoint<3>& rp, const Gaussian& g) {
			return g == rp;
		});
	*/
	bool IsEqualPosition = std::equal(pos.begin(), pos.end(), test.positions.begin(), [](const glm::vec3& AP, const glm::vec3& TP) {
			return AP == TP;
		});

	bool IsEqualRotation = std::equal(rot.begin(), rot.end(), test.rotations.begin(), [](const Rot& AR, const glm::vec4& TR) {
		return 
			AR.rot[0] == TR.x &&
			AR.rot[1] == TR.y &&
			AR.rot[2] == TR.z &&
			AR.rot[3] == TR.w;
		});

	bool IsEqualOpcaity= std::equal(opacities.begin(), opacities.end(), test.opacities.begin(), [](const float& AR, const float& TR) {
		return	AR == TR;
		});

	bool IsEqualScale = std::equal(scales.begin(), scales.end(), test.scales.begin(), [](const Scale& AR, const glm::vec3& TR) {
		return
			AR.scale[0] == TR.x &&
			AR.scale[1] == TR.y &&
			AR.scale[2] == TR.z;
		});

	bool IsEqualSHs = std::equal(shs.begin(), shs.end(), test.shs.begin(), [](const SHs<3>& AR, const std::array<float, 48>& TR) {
		for (int i = 0; i < 48; i += 3) {
				if (AR.shs[i] != TR[i]) {
					return false;
				}
			}
		return true;
		});


	ASSERT_TRUE(IsEqualPosition);
	ASSERT_TRUE(IsEqualRotation);
	ASSERT_TRUE(IsEqualOpcaity);
	ASSERT_TRUE(IsEqualScale);
	ASSERT_TRUE(IsEqualSHs);
}