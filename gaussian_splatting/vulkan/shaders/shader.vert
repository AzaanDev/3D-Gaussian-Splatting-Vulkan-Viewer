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

layout(std430, binding = 1) buffer Positions {
   float positions[];
};

// 6
layout(std430, binding = 2) buffer Cov3Ds {
   float cov3ds[];
};

layout(std430, binding = 3) buffer Alphas {
   float opacity[];
};

// 48
layout(std430, binding = 4) buffer Color {
   float shs[];
};

layout(location = 0) in vec2 inPosition;

layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outConic;
layout(location = 2) out float outAlpha;
layout(location = 3) out vec2 outCoordxy;


vec3 ComputeCov2D(vec3 mean, float focal_x, float focal_y, float tan_fovx, float tan_fovy, const float cov3D[6], mat4 view_matrix) {
    vec4 t = view_matrix * vec4(mean, 1.f);
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

float ndc2Pix(float v, float S) {
    return ((v + 1.) * S - 1.) * .5;
}

vec3 get_color(int index)
{
	return vec3(shs[index], shs[index + 1], shs[index + 2]);
}

void main() {
    int id = gl_InstanceIndex;
    vec3 position = vec3(positions[id * 3], positions[id * 3 + 1], positions[id * 3 + 2]);
    vec4 position_view = ubo.view * vec4(position, 1.0f);
    vec4 position_screen = ubo.proj * position_view;
    position_screen.xyz = position_screen.xyz / position_screen.w;
    position_screen.w = 1.f;

    float cov3D[6];
    cov3D[0] = cov3ds[id * 6];
    cov3D[1] = cov3ds[id * 6 + 1];
    cov3D[2] = cov3ds[id * 6 + 2];
    cov3D[3] = cov3ds[id * 6 + 3];
    cov3D[4] = cov3ds[id * 6 + 4];
    cov3D[5] = cov3ds[id * 6 + 5];

    vec3 cov =  ComputeCov2D(position, ubo.focal_x, ubo.focal_y, ubo.tan_fovx, ubo.tan_fovy, cov3D, ubo.view);
      
    float det = (cov.x * cov.z - cov.y * cov.y);
    if (det == 0.0f)
        return;
    float det_inv = 1.f / det;
    vec3 conic = { cov.z * det_inv, -cov.y * det_inv, cov.x * det_inv };
    
    vec3 t = vec3(ubo.tan_fovx, ubo.tan_fovy, ubo.focal_y);
    vec2 wh = 2 * t.xy * t.z;

    vec2 quadwh_scr = vec2(3.f * sqrt(cov.x), 3.f * sqrt(cov.z));  
    vec2 quadwh_ndc = quadwh_scr / wh * 2;  
    position_screen.xy = position_screen.xy + inPosition * quadwh_ndc;
    gl_Position = position_screen;
   
    int color_index = id * 48;
    vec3 dir = position - ubo.cam_pos;
    dir = normalize(dir);
    outColor = SH_C0 * get_color(color_index);

    if (ubo.render_mode > 0 && ubo.sh_dim > 3) 
    {
        float x = dir.x;
        float y = dir.y;
        float z = dir.z;
        outColor = outColor - SH_C1 * y * get_color(color_index + 1 * 3) + SH_C1 * z * get_color(color_index + 2 * 3) - SH_C1 * x * get_color(color_index + 3 * 3);
        
        if (ubo.render_mode > 1 && ubo.sh_dim > 12)
		{
			float xx = x * x, yy = y * y, zz = z * z;
			float xy = x * y, yz = y * z, xz = x * z;
			outColor = outColor +
				SH_C2_0 * xy * get_color(color_index + 4 * 3) +
				SH_C2_1 * yz * get_color(color_index + 5 * 3) +
				SH_C2_2 * (2.0f * zz - xx - yy) * get_color(color_index + 6 * 3) +
				SH_C2_3 * xz * get_color(color_index + 7 * 3) +
				SH_C2_4 * (xx - yy) * get_color(color_index + 8 * 3);

			if (ubo.render_mode > 2 && ubo.sh_dim > 27)
			{
				outColor = outColor +
					SH_C3_0 * y * (3.0f * xx - yy) * get_color(color_index + 9 * 3) +
					SH_C3_1 * xy * z * get_color(color_index + 10 * 3) +
					SH_C3_2 * y * (4.0f * zz - xx - yy) * get_color(color_index + 11 * 3) +
					SH_C3_3 * z * (2.0f * zz - 3.0f * xx - 3.0f * yy) * get_color(color_index + 12 * 3) +
					SH_C3_4 * x * (4.0f * zz - xx - yy) * get_color(color_index + 13 * 3) +
					SH_C3_5 * z * (xx - yy) * get_color(color_index + 14 * 3) +
					SH_C3_6 * x * (xx - 3.0f * yy) * get_color(color_index + 15 * 3);
			}
		}
    }

    outColor += 0.5f;
    outConic = conic;
    outAlpha = opacity[id];
    outCoordxy = inPosition * quadwh_scr;
}
