#version 310 es

layout (location = 0) in vec3 inPosition;

layout (location = 0) out vec4 outColor;

void main() 
{
	gl_Position = vec4 (inPosition, 1.0f);
	#if VULKAN_SHADER
	gl_Position.y = -gl_Position.y;
	#endif
	outColor = gl_Position;
}
