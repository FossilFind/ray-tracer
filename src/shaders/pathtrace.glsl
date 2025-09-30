#version 430 core
#extension GL_ARB_compute_shader: enable
#extension GL_ARB_shader_storage_buffer_object: enable

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D imgOutput;

layout(std140, binding = 1) buffer P0
{
	vec3 p0[];
};
layout(std140, binding = 2) buffer P1
{
	vec3 p1[];
};
layout(std140, binding = 3) buffer P2
{
	vec3 p2[];
};

uniform float camFOV;
uniform float camYaw;
uniform float camPitch;
uniform vec3 camPos;

mat3x3 yaw = mat3x3(cos(camYaw), -sin(camYaw), 0, sin(camYaw), cos(camYaw), 0, 0, 0, 1);
mat3x3 pitch = mat3x3(cos(camPitch), 0, sin(camPitch), 0, 1, 0, -sin(camPitch), 0, cos(camPitch));

vec3 findRayDir()
{
	float focalLength = 1 / (2 * tan(camFOV / 2));
	vec3 camDir = vec3(-1, 0, 0) * pitch * yaw;
	vec3 focalPoint = camDir * focalLength;

	vec3 rayDir = focalPoint - (vec3(0.0, (float(gl_GlobalInvocationID.x)/(gl_NumWorkGroups.x)) * 2 - 1, (float(gl_GlobalInvocationID.y)/(gl_NumWorkGroups.x)) * 2 - (float(gl_NumWorkGroups.y)/gl_NumWorkGroups.x)) * -1 * pitch * yaw);
	return normalize(rayDir);
}

vec3 triangleNormal(vec3 p1, vec3 p2, vec3 p3)
{
	vec3 v1 = p2 - p1;
	vec3 v2 = p3 - p1;

	return normalize(cross(v1, v2));
}

float distanceFromPlane(vec3 planeNormal, vec3 planePoint, vec3 rayDirection, vec3 rayPoint)
{
	return -(dot(planeNormal, rayPoint) - dot(planeNormal, planePoint)) / dot(planeNormal, rayDirection);
}

vec3 rayHitPoint(vec3 rayDirection, vec3 rayPoint, float dis)
{
	return rayPoint + dis * rayDirection;
}

bool leftTest(vec3 point, vec3 lineStart, vec3 lineEnd, vec3 normal)
{
	vec3 pointVec = point - lineStart;
	vec3 lineVec = lineEnd - lineStart;

	vec3 dir = cross(lineVec, pointVec);

	return dot(normal, dir) > 0;
}

void main()
{
	vec4 value = vec4(0.7, 0.8, 0.9, 1.0);
	ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

	vec3 rayPoint = camPos;
	vec3 rayDir = findRayDir();
	value *= (dot(vec3(0, 0, 1), rayDir) + 3) / 4;

	float minDis = 1000000000;

	for(int i = 0; i < p0.length(); i++)
	{
		vec3 triNorm = triangleNormal(p0[i], p1[i], p2[i]);
		float dis = distanceFromPlane(triNorm, p0[i], rayDir, rayPoint);

		if(dis > 0 && dis < minDis)
		{
			vec3 hitPoint = rayHitPoint(rayDir, rayPoint, dis);

			if(leftTest(hitPoint, p0[i], p1[i], triNorm)
				&& leftTest(hitPoint, p1[i], p2[i], triNorm)
				&& leftTest(hitPoint, p2[i], p0[i], triNorm))
			{
				minDis = dis;
				value = vec4(vec3(float(i) / p0.length()), 1.0);
			}
		}
	}

	imageStore(imgOutput, texelCoord, value);
}