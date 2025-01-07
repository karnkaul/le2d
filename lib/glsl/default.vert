#version 450 core

struct Instance {
	mat4 mat_world;
	vec4 tint;
};

layout (location = 0) in vec2 a_pos;
layout (location = 1) in vec4 a_color;
layout (location = 2) in vec2 a_uv;

layout (location = 0) out vec4 out_tint;
layout (location = 1) out vec2 out_uv;

layout (set = 0, binding = 0) uniform View {
	mat4 mat_vp;
};

layout (set = 1, binding = 0) readonly buffer Instances {
	Instance instances[];
};

void main() {
	const Instance instance = instances[gl_InstanceIndex];

	out_uv = a_uv;
	out_tint = a_color;

	const vec4 v_pos = vec4(a_pos, 0.0, 1.0);
	const vec4 world_pos = instance.mat_world * v_pos;
	gl_Position = mat_vp * world_pos;
}
