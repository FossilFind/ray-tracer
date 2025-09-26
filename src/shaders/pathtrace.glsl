#version 430 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D imgOutput;

uniform float camFOV;
uniform float camYaw;
uniform float camPitch;
uniform vec3 camPos;

uniform vec3 triPoint0;
uniform vec3 triPoint1;
uniform vec3 triPoint2;

mat3x3 yaw = mat3x3(cos(-camYaw), -sin(-camYaw), 0, sin(-camYaw), cos(-camYaw), 0, 0, 0, 1);
mat3x3 pitch = mat3x3(cos(-camPitch), 0, sin(-camPitch), 0, 1, 0, -sin(-camPitch), 0, cos(-camPitch));

// vec3 rodriguesRot(vec3 v, vec3 axis, float angle)
// {
// 	return (v * cos(angle)) + (cross(axis, v) * sin(angle)) + (axis * dot(axis, v) * (1 - cos(angle)));
// }

vec3 findRayPoint()
{
	float focalLength = 1 / (2 * tan(camFOV / 2));
	vec3 camDir = vec3(-1, 0, 0) * pitch * yaw;
	return camPos - (camDir * focalLength);
}

vec3 findRayDir(vec3 rayPoint)
{
	vec3 pixelPos = camPos + vec3(0.0, (float(gl_GlobalInvocationID.x)/(gl_NumWorkGroups.x)) * 2 - 1, (float(gl_GlobalInvocationID.y)/(gl_NumWorkGroups.y)) * 2 - 1) * pitch * yaw;
	return pixelPos - rayPoint;
}

vec3 triangleNormal(vec3 p1, vec3 p2, vec3 p3)
{
	vec3 v1 = p2 - p1;
	vec3 v2 = p3 - p1;

	return cross(v1, v2);
}

float distanceFromPlane(vec3 planeNormal, vec3 planePoint, vec3 rayDirection, vec3 rayPoint)
{
	return dot(planeNormal, planePoint + rayPoint) / -dot(planeNormal, rayDirection);
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

	vec3 rayPoint = findRayPoint();
	vec3 rayDir = findRayDir(rayPoint);
	value *= (dot(vec3(0, 0, 1), rayDir) + 3) / 4;

	vec3 triNorm = triangleNormal(triPoint0, triPoint1, triPoint2);

	float dis = distanceFromPlane(triNorm, triPoint0, rayDir, rayPoint);

	if(dis > 0)
	{
		vec3 hitPoint = rayHitPoint(rayDir, rayPoint, dis);

		if(leftTest(hitPoint, triPoint0, triPoint1, triNorm)
			&& leftTest(hitPoint, triPoint1, triPoint2, triNorm)
			&& leftTest(hitPoint, triPoint2, triPoint0, triNorm))
		{
			value = vec4(1.0, 1.0, 1.0, 1.0);
		}
	}

	imageStore(imgOutput, texelCoord, value);
}