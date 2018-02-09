#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "Transform.h"



struct Bone
{
	std::string				name;
	uint32_t				parent;
	//Transform				transform;
	glm::mat4				transform;
	glm::mat4				offsetMatrix;
};

struct Model
{
	std::vector<glm::vec3>	positions;
	std::vector<glm::vec3>	normals;
	std::vector<glm::vec2>	texcoords;
	std::vector<glm::ivec4>	boneIds;
	std::vector<glm::vec4>	boneWeights;

	std::vector<uint16_t>	indices;

	std::vector<uint16_t>	meshVertexBases;
	std::vector<uint16_t>	meshIndexBases;
	std::vector<uint16_t>	meshIndexCounts;

	std::vector<Bone>							bones;
	std::unordered_map<std::string, uint32_t>	boneTable;

	int32_t Load(const char* filename);

private:
	int32_t LoadBones(const struct aiNode* node);

	int32_t LoadMeshes(const struct aiScene* scene);

};
