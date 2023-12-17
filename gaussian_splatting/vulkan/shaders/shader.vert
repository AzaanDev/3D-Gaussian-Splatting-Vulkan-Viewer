#version 450

#define SH_C0 0.28209479177387814f
#define SH_C1 0.4886025119029199f
#define SH_C2_0 1.0925484305920792f
#define SH_C2_1 -1.0925484305920792f
#define SH_C2_2 0.31539156525252005f
#define SH_C2_3 -1.0925484305920792f
#define SH_C2_4 0.5462742152960396f
#define SH_C3_0 -0.5900435899266435f
#define SH_C3_1 2.890611442640554f
#define SH_C3_2 -0.4570457994644658f
#define SH_C3_3 0.3731763325901154f
#define SH_C3_4 -0.4570457994644658f
#define SH_C3_5 1.445305721320277f
#define SH_C3_6 -0.5900435899266435f

#define COV3D_SIZE 6
#define SH_SIZE 3

layout(binding = 0) uniform UniformBufferObject {
	mat4 view;
	mat4 proj;
   	vec3 cam_pos;
	float tan_fovy;
	float tan_fovx;
	float focal_y;
	float focal_x;
	int sh_dim;
    int render_mode;
} ubo;

layout(std140, binding = 1) buffer Positions {
   vec3 positions[];
};

// 6
layout(std140, binding = 2) buffer Cov3Ds {
   float cov3ds[];
};

layout(std140, binding = 3) buffer Alphas {
   float opacity[];
};

// 48
layout(std140, binding = 4) buffer Color {
   float shs[];
};

layout(location = 0) in vec2 inPosition;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outConic;
layout(location = 2) out float outAlpha;
layout(location = 3) out vec2 outCoordxy;


vec3 ComputeCov2D(vec3 mean, float focal_x, float focal_y, float tan_fovx, float tan_fovy, const float* cov3D, mat4 view_matrix) {
    vec3 t = view_matrix * vec4(mean, 1.0f);
    float limx = 1.3f * tan_fovx;
    float limy = 1.3f * tan_fovy;
    float txtz = t.x / t.z;
    float tytz = t.y / t.z;
    t.x = min(limx, max(-limx, txtz)) * t.z;
    t.y = min(limy, max(-limy, tytz)) * t.z;

    mat3 J = mat3(
        focal_x / t.z, 0.0f, -(focal_x * t.x) / (t.z * t.z),
        0.0f, focal_y / t.z, -(focal_y * t.y) / (t.z * t.z),
        0, 0, 0);

    mat3 W = mat3(
        view_matrix[0][0], view_matrix[1][0], view_matrix[2][0],
        view_matrix[0][1], view_matrix[1][1], view_matrix[2][1],
        view_matrix[0][2], view_matrix[1][2], view_matrix[2][2]
    );

    mat3 T = W * J;

    mat3 Vrk = mat3(
        cov3D[0], cov3D[1], cov3D[2],
        cov3D[1], cov3D[3], cov3D[4],
        cov3D[2], cov3D[4], cov3D[5]);

    mat3 cov = transpose(T) * transpose(Vrk) * T;

    cov[0][0] += .3f;
    cov[1][1] += .3f;
    return vec3(cov[0][0], cov[0][1], cov[1][1]);
}

void main() {
    int id = gl_InstanceIndex;
	vec3 cov2D = ComputeCov2D(mean, ubo.focal_x, ubo.focal_y, ubo.tan_fovx, ubo.tan_fovy, cov3D, ubo.view);

}