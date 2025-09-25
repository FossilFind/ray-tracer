#version 430 core

layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

layout(rgba32f, binding = 0) uniform image2D imgOutput;

uniform vec3 triPoint0;
uniform vec3 triPoint1;
uniform vec3 triPoint2;

vec3 triangleNormal(vec3 p1, vec3 p2, vec3 p3)
{
	vec3 v1 = p2 - p1;
	vec3 v2 = p3 - p1;

	return cross(v1, v2);
}

vec3 rayHitPoint(vec3 planeNormal, vec3 planePoint, vec3 rayDirection, vec3 rayPoint)
{
	float t = dot(planeNormal, planePoint + rayPoint) / -dot(planeNormal, rayDirection);
	return rayPoint + t * rayDirection;
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
	vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
	ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);

	vec3 rayDir = vec3(-1.0, 0.0, 0.0);
	vec3 rayPoint = vec3(0.0, (float(texelCoord.x)/(gl_NumWorkGroups.x)) * 2 - 1, (float(texelCoord.y)/(gl_NumWorkGroups.y)) * 2 - 1);
	
	vec3 triNorm = triangleNormal(triPoint0, triPoint1, triPoint2);

	vec3 hitPoint = rayHitPoint(triNorm, triPoint0, rayDir, rayPoint);

	if(leftTest(hitPoint, triPoint0, triPoint1, triNorm)
		&& leftTest(hitPoint, triPoint1, triPoint2, triNorm)
		&& leftTest(hitPoint, triPoint2, triPoint0, triNorm))
	{
		value = vec4(1.0, 1.0, 1.0, 1.0);
	}

	imageStore(imgOutput, texelCoord, value);
}