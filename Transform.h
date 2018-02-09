#pragma once

#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>

struct Transform
{
	glm::vec3				translation;
	glm::quat				rotation;
	glm::vec3				scaling;

	Transform()
		:
		translation(),
		rotation(),
		scaling(1.0f)
	{}

	Transform(const glm::vec3& t, const glm::quat& r, const glm::vec3& s)
		:
		translation(t),
		rotation(r),
		scaling(s)
	{}

	Transform(const glm::mat4 m)
	{
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(m, scaling, rotation, translation, skew, perspective);
	}
};
