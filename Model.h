#pragma once

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <string>
#define GLM_FORCE_LEFT_HANDED
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include "Transform.h"

struct HumanBone
{
	enum HumanBoneId
	{
		None = -1,
		Hips,
		Spine,
		Spine1,
		Spine2,
		Neck,
		Head,
		LeftShoulder,
		LeftArm,
		LeftForeArm,
		LeftHand,
		LeftHandThumb1,
		LeftHandThumb2,
		LeftHandThumb3,
		LeftHandIndex1,
		LeftHandIndex2,
		LeftHandIndex3,
		LeftHandMiddle1,
		LeftHandMiddle2,
		LeftHandMiddle3,
		LeftHandRing1,
		LeftHandRing2,
		LeftHandRing3,
		LeftHandPinky1,
		LeftHandPinky2,
		LeftHandPinky3,
		RightShoulder,
		RightArm,
		RightForeArm,
		RightHand,
		RightHandThumb1,
		RightHandThumb2,
		RightHandThumb3,
		RightHandIndex1,
		RightHandIndex2,
		RightHandIndex3,
		RightHandMiddle1,
		RightHandMiddle2,
		RightHandMiddle3,
		RightHandRing1,
		RightHandRing2,
		RightHandRing3,
		RightHandPinky1,
		RightHandPinky2,
		RightHandPinky3,
		LeftUpLeg,
		LeftLeg,
		LeftFoot,
		LeftToeBase,
		RightUpLeg,
		RightLeg,
		RightFoot,
		RightToeBase,
		NumHumanBones
	};

	static uint32_t parent(uint32_t id);
	static const char* name(uint32_t id);
};

struct Bone
{
	std::string				name;
	uint32_t				parent;
	uint32_t				humanBoneId = UINT32_MAX;
	//Transform				transform;
	glm::mat4				transform;
	glm::mat4				offsetMatrix;
};

struct Vec3Frame
{
	glm::vec3				value;
	float					time;
};

struct QuatFrame
{
	glm::quat				value;
	float					time;
};

struct Channel
{
	std::string				name;
	std::vector<Vec3Frame>	translations;
	std::vector<QuatFrame>	rotations;
	std::vector<Vec3Frame>	scalings;
};

struct Animation
{
	std::string				name;
	float					length;
	std::vector<Channel>	channels;
};

struct Model
{
	std::vector<glm::vec3>	positions;
	std::vector<glm::vec3>	normals;
	std::vector<glm::vec2>	texcoords;
	std::vector<glm::ivec4>	boneIds;
	std::vector<glm::vec4>	boneWeights;

	std::vector<uint16_t>	indices;

	uint32_t				vertexCount;
	uint32_t				indexCount;
	std::vector<uint16_t>	meshVertexBases;
	std::vector<uint16_t>	meshIndexBases;
	std::vector<uint16_t>	meshIndexCounts;

	std::vector<Bone>							bones;
	std::unordered_map<std::string, uint32_t>	boneTable;
	std::vector<std::vector<uint32_t>>			boneTree;

	std::vector<Animation>						animations;
	std::unordered_map<std::string, uint32_t>	animTable;

	std::vector<uint32_t>						humanBoneBindings;
	std::vector<glm::quat>						humanBoneWorldR;
	std::vector<glm::quat>						humanBoneLocalR;

	int32_t Load(const char* filename);

	int32_t LoadAvatar(const char* filename);

	int32_t SaveAvatar(const char* filename);

	void GuessHumanBoneBindings();

private:
	int32_t LoadBones(const struct aiNode* node);

	int32_t LoadMeshes(const struct aiScene* scene);

	int32_t LoadAnimations(const struct aiScene* scene);
};
