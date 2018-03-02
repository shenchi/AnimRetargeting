#include "Model.h"

#include <rapidjson/document.h>
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;
using namespace glm;

namespace
{
	const char* HumanBoneNames[HumanBone::NumHumanBones] =
	{
		"Root",
		"Hips",
		"Spine",
		"Spine1",
		"Spine2",
		"Neck",
		"Head",
		"HeadTop",
		"LeftShoulder",
		"LeftArm",
		"LeftForeArm",
		"LeftHand",
		"LeftHandThumb1",
		"LeftHandThumb2",
		"LeftHandThumb3",
		"LeftHandThumb4",
		"LeftHandIndex1",
		"LeftHandIndex2",
		"LeftHandIndex3",
		"LeftHandIndex4",
		"LeftHandMiddle1",
		"LeftHandMiddle2",
		"LeftHandMiddle3",
		"LeftHandMiddle4",
		"LeftHandRing1",
		"LeftHandRing2",
		"LeftHandRing3",
		"LeftHandRing4",
		"LeftHandPinky1",
		"LeftHandPinky2",
		"LeftHandPinky3",
		"LeftHandPinky4",
		"RightShoulder",
		"RightArm",
		"RightForeArm",
		"RightHand",
		"RightHandThumb1",
		"RightHandThumb2",
		"RightHandThumb3",
		"RightHandThumb4",
		"RightHandIndex1",
		"RightHandIndex2",
		"RightHandIndex3",
		"RightHandIndex4",
		"RightHandMiddle1",
		"RightHandMiddle2",
		"RightHandMiddle3",
		"RightHandMiddle4",
		"RightHandRing1",
		"RightHandRing2",
		"RightHandRing3",
		"RightHandRing4",
		"RightHandPinky1",
		"RightHandPinky2",
		"RightHandPinky3",
		"RightHandPinky4",
		"LeftUpLeg",
		"LeftLeg",
		"LeftFoot",
		"LeftToeBase",
		"LeftToeBaseEnd",
		"RightUpLeg",
		"RightLeg",
		"RightFoot",
		"RightToeBase",
		"RightToeBaseEnd",
	};

	HumanBone::HumanBoneId HumanBoneDirectionTargets[HumanBone::NumHumanBones] =
	{
		HumanBone::Hips, //Root,
		HumanBone::Spine, //Hips,
		HumanBone::Spine1, //Spine,
		HumanBone::Spine2, //Spine1,
		HumanBone::Neck, //Spine2,
		HumanBone::Head, //Neck,
		HumanBone::HeadTop, //Head,
		HumanBone::None, //HeadTop,
		HumanBone::LeftArm, //LeftShoulder,
		HumanBone::LeftForeArm, //LeftArm,
		HumanBone::LeftHand, //LeftForeArm,
		HumanBone::LeftHandMiddle1, //LeftHand,
		HumanBone::LeftHandThumb2, //LeftHandThumb1,
		HumanBone::LeftHandThumb3, //LeftHandThumb2,
		HumanBone::LeftHandThumb4, //LeftHandThumb3,
		HumanBone::None, //LeftHandThumb4,
		HumanBone::LeftHandIndex2, //LeftHandIndex1,
		HumanBone::LeftHandIndex3, //LeftHandIndex2,
		HumanBone::LeftHandIndex4, //LeftHandIndex3,
		HumanBone::None, //LeftHandIndex4,
		HumanBone::LeftHandMiddle2, //LeftHandMiddle1,
		HumanBone::LeftHandMiddle3, //LeftHandMiddle2,
		HumanBone::LeftHandMiddle4, //LeftHandMiddle3,
		HumanBone::None, //LeftHandMiddle4,
		HumanBone::LeftHandRing2, //LeftHandRing1,
		HumanBone::LeftHandRing3, //LeftHandRing2,
		HumanBone::LeftHandRing4, //LeftHandRing3,
		HumanBone::None, //LeftHandRing4,
		HumanBone::LeftHandPinky2, //LeftHandPinky1,
		HumanBone::LeftHandPinky3, //LeftHandPinky2,
		HumanBone::LeftHandPinky4, //LeftHandPinky3,
		HumanBone::None, //LeftHandPinky4,
		HumanBone::RightArm, //RightShoulder,
		HumanBone::RightForeArm, //RightArm,
		HumanBone::RightHand, //RightForeArm,
		HumanBone::RightHandMiddle1, //RightHand,
		HumanBone::RightHandThumb2, //RightHandThumb1,
		HumanBone::RightHandThumb3, //RightHandThumb2,
		HumanBone::RightHandThumb4, //RightHandThumb3,
		HumanBone::None, //RightHandThumb4,
		HumanBone::RightHandIndex2, //RightHandIndex1,
		HumanBone::RightHandIndex3, //RightHandIndex2,
		HumanBone::RightHandIndex4, //RightHandIndex3,
		HumanBone::None, //RightHandIndex4,
		HumanBone::RightHandMiddle2, //RightHandMiddle1,
		HumanBone::RightHandMiddle3, //RightHandMiddle2,
		HumanBone::RightHandMiddle4, //RightHandMiddle3,
		HumanBone::None, //RightHandMiddle4,
		HumanBone::RightHandRing2, //RightHandRing1,
		HumanBone::RightHandRing3, //RightHandRing2,
		HumanBone::RightHandRing4, //RightHandRing3,
		HumanBone::None, //RightHandRing4,
		HumanBone::RightHandPinky2, //RightHandPinky1,
		HumanBone::RightHandPinky3, //RightHandPinky2,
		HumanBone::RightHandPinky4, //RightHandPinky3,
		HumanBone::None, //RightHandPinky4,
		HumanBone::LeftLeg, //LeftUpLeg,
		HumanBone::LeftFoot, //LeftLeg,
		HumanBone::LeftToeBase, //LeftFoot,
		HumanBone::LeftToeBaseEnd, //LeftToeBase,
		HumanBone::None, //LeftToeBaseEnd,
		HumanBone::RightLeg, //RightUpLeg,
		HumanBone::RightFoot, //RightLeg,
		HumanBone::RightToeBase, //RightFoot,
		HumanBone::RightToeBaseEnd, //RightToeBase,
		HumanBone::None, //RightToeBaseEnd,
	};

	vec3 HumanBoneDefaultDirections[HumanBone::NumHumanBones] =
	{
		{}, //Root, (doesn't matter)
		{ 0, 1, 0 }, //Hips,
		{ 0, 1, 0 }, //Spine,
		{ 0, 1, 0 }, //Spine1,
		{ 0, 1, 0 }, //Spine2,
		{ 0, 1, 0 }, //Neck,
		{ 0, 1, 0 }, //Head,
		{}, //HeadTop, (doesn't matter)
		{ 1, 0, 0 }, //LeftShoulder,
		{ 1, 0, 0 }, //LeftArm,
		{ 1, 0, 0 }, //LeftForeArm,
		{ 0.275f, 0.016f, -0.041f }, //LeftHand,
		{ 0.079f, -0.017f, -0.022f }, //LeftHandThumb1,
		{ 1, 0, 0 }, //LeftHandThumb2,
		{ 1, 0, 0 }, //LeftHandThumb3,
		{}, //LeftHandThumb4, (doesn't matter)
		{ 0.132f, 0, -0.005f }, //LeftHandIndex1,
		{ 0.083f, 0, -0.003f }, //LeftHandIndex2,
		{ 0.061f, 0, -0.002f }, //LeftHandIndex3,
		{}, //LeftHandIndex4, (doesn't matter)
		{ 1, 0, 0}, //LeftHandMiddle1,
		{ 1, 0, 0 }, //LeftHandMiddle2,
		{ 1, 0, 0 }, //LeftHandMiddle3,
		{}, //LeftHandMiddle4, (doesn't matter)
		{ 1, 0, 0 }, //LeftHandRing1,
		{ 1, 0, 0 }, //LeftHandRing2,
		{ 1, 0, 0 }, //LeftHandRing3,
		{}, //LeftHandRing4, (doesn't matter)
		{ 1, 0, 0 }, //LeftHandPinky1,
		{ 1, 0, 0 }, //LeftHandPinky2,
		{ 1, 0, 0 }, //LeftHandPinky3,
		{}, //LeftHandPinky4, (doesn't matter)
		{ -1, 0, 0 }, //RightShoulder,
		{ -1, 0, 0 }, //RightArm,
		{ -1, 0, 0 }, //RightForeArm,
		{ -0.275f, 0.016f, -0.041f }, //RightHand,
		{ -0.079f, -0.017f, -0.022f }, //RightHandThumb1,
		{ -1, 0, 0 }, //RightHandThumb2,
		{ -1, 0, 0 }, //RightHandThumb3,
		{}, //RightHandThumb4, (doesn't matter)
		{ -0.132f, 0, -0.005f }, //RightHandIndex1,
		{ -0.083f, 0, -0.003f }, //RightHandIndex2,
		{ -0.061f, 0, -0.002f }, //RightHandIndex3,
		{}, //RightHandIndex4, (doesn't matter)
		{ -1, 0, 0 }, //RightHandMiddle1,
		{ -1, 0, 0 }, //RightHandMiddle2,
		{ -1, 0, 0 }, //RightHandMiddle3,
		{}, //RightHandMiddle4, (doesn't matter)
		{ -1, 0, 0 }, //RightHandRing1,
		{ -1, 0, 0 }, //RightHandRing2,
		{ -1, 0, 0 }, //RightHandRing3,
		{}, //RightHandRing4, (doesn't matter)
		{ -1, 0, 0 }, //RightHandPinky1,
		{ -1, 0, 0 }, //RightHandPinky2,
		{ -1, 0, 0 }, //RightHandPinky3,
		{}, //RightHandPinky4, (doesn't matter)
		{ 0, -1, 0 }, //LeftUpLeg,
		{ 0, -1, 0 }, //LeftLeg,
		{ 0, -0.196f, -0.405f }, //LeftFoot,
		{ 0, 0, -1 }, //LeftToeBase,
		{}, //LeftToeBaseEnd, (doesn't matter)
		{ 0, -1, 0 }, //RightUpLeg,
		{ 0, -1, 0 }, //RightLeg,
		{ 0, -0.196f, -0.405f }, //RightFoot,
		{ 0, 0, -1 }, //RightToeBase,
		{}, //RightToeBaseEnd, (doesn't matter)
	};

	HumanBone::HumanBoneId HumanBoneParents[HumanBone::NumHumanBones] =
	{
		HumanBone::None, //Root,
		HumanBone::Root, //Hips,
		HumanBone::Hips, //Spine,
		HumanBone::Spine, //Spine1,
		HumanBone::Spine1, //Spine2,
		HumanBone::Spine2, //Neck,
		HumanBone::Neck, //Head,
		HumanBone::Head, //HeadTop,
		HumanBone::Spine2, //LeftShoulder,
		HumanBone::LeftShoulder, //LeftArm,
		HumanBone::LeftArm, //LeftForeArm,
		HumanBone::LeftForeArm, //LeftHand,
		HumanBone::LeftHand, //LeftHandThumb1,
		HumanBone::LeftHandThumb1, //LeftHandThumb2,
		HumanBone::LeftHandThumb2, //LeftHandThumb3,
		HumanBone::LeftHandThumb3, //LeftHandThumb4,
		HumanBone::LeftHand, //LeftHandIndex1,
		HumanBone::LeftHandIndex1, //LeftHandIndex2,
		HumanBone::LeftHandIndex2, //LeftHandIndex3,
		HumanBone::LeftHandIndex3, //LeftHandIndex4,
		HumanBone::LeftHand, //LeftHandMiddle1,
		HumanBone::LeftHandMiddle1, //LeftHandMiddle2,
		HumanBone::LeftHandMiddle2, //LeftHandMiddle3,
		HumanBone::LeftHandMiddle3, //LeftHandMiddle4,
		HumanBone::LeftHand, //LeftHandRing1,
		HumanBone::LeftHandRing1, //LeftHandRing2,
		HumanBone::LeftHandRing2, //LeftHandRing3,
		HumanBone::LeftHandRing3, //LeftHandRing4,
		HumanBone::LeftHand, //LeftHandPinky1,
		HumanBone::LeftHandPinky1, //LeftHandPinky2,
		HumanBone::LeftHandPinky2, //LeftHandPinky3,
		HumanBone::LeftHandPinky3, //LeftHandPinky4,
		HumanBone::Spine2, //RightShoulder,
		HumanBone::RightShoulder, //RightArm,
		HumanBone::RightArm, //RightForeArm,
		HumanBone::RightForeArm, //RightHand,
		HumanBone::RightHand, //RightHandThumb1,
		HumanBone::RightHandThumb1, //RightHandThumb2,
		HumanBone::RightHandThumb2, //RightHandThumb3,
		HumanBone::RightHandThumb3, //RightHandThumb4,
		HumanBone::RightHand, //RightHandIndex1,
		HumanBone::RightHandIndex1, //RightHandIndex2,
		HumanBone::RightHandIndex2, //RightHandIndex3,
		HumanBone::RightHandIndex3, //RightHandIndex4,
		HumanBone::RightHand, //RightHandMiddle1,
		HumanBone::RightHandMiddle1, //RightHandMiddle2,
		HumanBone::RightHandMiddle2, //RightHandMiddle3,
		HumanBone::RightHandMiddle3, //RightHandMiddle4,
		HumanBone::RightHand, //RightHandRing1,
		HumanBone::RightHandRing1, //RightHandRing2,
		HumanBone::RightHandRing2, //RightHandRing3,
		HumanBone::RightHandRing3, //RightHandRing4,
		HumanBone::RightHand, //RightHandPinky1,
		HumanBone::RightHandPinky1, //RightHandPinky2,
		HumanBone::RightHandPinky2, //RightHandPinky3,
		HumanBone::RightHandPinky3, //RightHandPinky4,
		HumanBone::Hips, //LeftUpLeg,
		HumanBone::LeftUpLeg, //LeftLeg,
		HumanBone::LeftLeg, //LeftFoot,
		HumanBone::LeftFoot, //LeftToeBase,
		HumanBone::LeftToeBase, //LeftToeBaseEnd,
		HumanBone::Hips, //RightUpLeg,
		HumanBone::RightUpLeg, //RightLeg,
		HumanBone::RightLeg, //RightFoot,
		HumanBone::RightFoot, //RightToeBase
		HumanBone::RightToeBase, //RightToeBaseEnd
	};
}

mat4 compose(const vec3& t, const quat& r, const vec3& s)
{

	float a_sqr = r.w * r.w;
	float b_sqr = r.x * r.x;
	float c_sqr = r.y * r.y;
	float d_sqr = r.z * r.z;

	float a_b_2 = r.w * r.x * 2;
	float a_c_2 = r.w * r.y * 2;
	float a_d_2 = r.w * r.z * 2;

	float b_c_2 = r.x * r.y * 2;
	float b_d_2 = r.x * r.z * 2;

	float c_d_2 = r.y * r.z * 2;

	return transpose(mat4{
		vec4{ s.x * (a_sqr + b_sqr - c_sqr - d_sqr), s.y * (b_c_2 - a_d_2), s.z * (a_c_2 + b_d_2), t.x },
		vec4{ s.x * (a_d_2 + b_c_2), s.y * (a_sqr - b_sqr + c_sqr - d_sqr), s.z * (c_d_2 - a_b_2), t.y },
		vec4{ s.x * (b_d_2 - a_c_2), s.y * (a_b_2 + c_d_2), s.z * (a_sqr - b_sqr - c_sqr + d_sqr), t.z },
		vec4{ 0.0f, 0.0f, 0.0f, 1.0f }
	});
}

void decompose(const mat4& m, vec3& t, quat& r, vec3& s)
{
	vec3 skew; vec4 persp;
	glm::decompose(m, s, r, t, skew, persp);
	r = glm::conjugate(r);
}

bool CorrectHumanBoneRotation(const Model & dstModel, const Model & srcModel, quat& rotation, uint32_t humanBoneId)
{
	uint32_t srcBoneId = srcModel.humanBoneBindings[humanBoneId];
	if (srcBoneId == UINT32_MAX) return false;

	uint32_t dstBoneId = dstModel.humanBoneBindings[humanBoneId];
	if (dstBoneId == UINT32_MAX) return false;

	uint32_t parentId = srcModel.bones[srcBoneId].parent;
	uint32_t parentHumanBoneId = srcModel.bones[parentId].humanBoneId;
	if (parentHumanBoneId != UINT32_MAX)
	{
		bool isHand = (humanBoneId >= HumanBone::LeftHand &&
			humanBoneId < HumanBone::RightShoulder) ||
			(humanBoneId >= HumanBone::RightHand &&
				humanBoneId < HumanBone::LeftUpLeg);

		isHand = false;

		quat TPoseLocalR = srcModel.humanBoneLocalR[humanBoneId];
		if (!isHand)
		{
			TPoseLocalR = srcModel.humanBoneCorrectionLocalR[humanBoneId] * TPoseLocalR;
		}

		//quat deltaR = inverse(TPoseLocalR) * r;
		quat deltaR = rotation *  inverse(TPoseLocalR);

		quat srcParentWorldR = srcModel.humanBoneWorldR[parentHumanBoneId];
		deltaR = srcParentWorldR * deltaR * inverse(srcParentWorldR);
		//r = inverse(srcParentWorldR) * deltaR * srcParentWorldR;

		quat dstParentWorldR = dstModel.humanBoneWorldR[parentHumanBoneId];
		deltaR = inverse(dstParentWorldR) * deltaR * dstParentWorldR;
		//r = dstParentWorldR * r * inverse(dstParentWorldR);

		deltaR = quat(1, 0, 0, 0);

		quat modelStdTPoseR = dstModel.humanBoneLocalR[humanBoneId];

		if (!isHand)
		{
			modelStdTPoseR = dstModel.humanBoneCorrectionLocalR[humanBoneId] * modelStdTPoseR;
		}

		//r = model.humanBoneLocalR[humanBoneId] * r;
		rotation = deltaR * modelStdTPoseR;

	}
	return true;
}

uint32_t HumanBone::parent(uint32_t id)
{
	if (id >= NumHumanBones) return HumanBone::None;
	return HumanBoneParents[id];
}

uint32_t HumanBone::target(uint32_t id)
{
	if (id >= NumHumanBones) return HumanBone::None;
	return HumanBoneDirectionTargets[id];
}

const glm::vec3& HumanBone::direction(uint32_t id)
{
	if (id >= NumHumanBones) return HumanBoneDefaultDirections[HumanBone::Root];
	return HumanBoneDefaultDirections[id];
}

const char * HumanBone::name(uint32_t id)
{
	if (id >= NumHumanBones) return nullptr;
	return HumanBoneNames[id];
}

int32_t Model::Load(const char * filename)
{
	Assimp::Importer importer;

	uint32_t flags = aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded |
		aiProcess_LimitBoneWeights;
	const aiScene* scene = importer.ReadFile(filename, flags);

	if (nullptr == scene)
	{
		return __LINE__;
	}

	LoadBones(scene->mRootNode);

	if (scene->HasMeshes())
	{
		LoadMeshes(scene);
	}

	if (scene->HasAnimations())
	{
		LoadAnimations(scene);
	}

	return 0;
}

//int32_t Model::Save(const char * filename)
//{
//	aiScene scene = aiScene();
//	Assimp::Exporter exporter;
//	
//	// bones
//	vector<aiNode> nodes(bones.size());
//	vector<size_t> nodeTreeIndices(boneTree.size(), 0);
//	vector<size_t> nodeTreeChildrenCounts(boneTree.size(), 0);
//
//	size_t nodeTreeSize = 0;
//	for (uint32_t i = 0; i < boneTree.size(); i++)
//	{
//		nodeTreeIndices[i] = nodeTreeSize;
//		nodeTreeChildrenCounts[i] = boneTree[i].size();
//		nodeTreeSize += boneTree[i].size();
//	}
//
//	vector<aiNode*> nodeTree(nodeTreeSize, nullptr);
//
//	for (uint32_t i = 0; i < boneTree.size(); i++)
//	{
//		for (uint32_t j = 0; j < boneTree[i].size(); j++)
//		{
//			nodeTree[nodeTreeIndices[i] + j] = &nodes[boneTree[i][j]];
//		}
//	}
//
//	scene.mRootNode = &nodes[0];
//
//	for (uint32_t i = 0; i < bones.size(); i++)
//	{
//		uint32_t p = bones[i].parent;
//		if (p != UINT32_MAX)
//		{
//			nodes[i].mParent = &nodes[p];
//		}
//		if (nodeTreeChildrenCounts[i] > 0)
//		{
//			nodes[i].mNumChildren = nodeTreeChildrenCounts[i];
//			nodes[i].mChildren = reinterpret_cast<aiNode**>(nodeTree[nodeTreeIndices[i]]);
//		}
//
//		nodes[i].mName = bones[i].name;
//
//		const mat4& m = bones[i].transform;
//		nodes[i].mTransformation = aiMatrix4x4(
//			m[0][0], m[0][1], m[0][2], m[0][3],
//			m[1][0], m[1][1], m[1][2], m[1][3],
//			m[2][0], m[2][1], m[2][2], m[2][3],
//			m[3][0], m[3][1], m[3][2], m[3][3]
//		);
//	}
//
//	// meshes
//
//	vector<aiMesh> meshes(meshVertexBases.size());
//
//	scene.mMeshes = reinterpret_cast<aiMesh**>(&(meshes[0]));
//	scene.mNumMeshes = meshes.size();
//
//	for (uint32_t i = 0; i < scene.mNumMeshes; i++)
//	{
//		aiMesh& mesh = meshes[i];
//		mesh.
//	}
//
//	// animations
//
//
//	// export
//	return exporter.Export(&scene, "fbx", filename);
//}

int32_t Model::LoadAvatar(const char * filename)
{
	FILE* fp = fopen(filename, "rb");
	if (!fp) return 0;

	fseek(fp, 0, SEEK_END);
	size_t size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char* buffer = (char*)malloc(size + 1);
	fread(buffer, size, 1, fp);
	fclose(fp);
	buffer[size] = 0;

	rapidjson::Document doc;
	doc.Parse(buffer);

	free(buffer);

	assert(!doc.HasParseError());

	// column-majored
	std::vector<mat4> humanBoneWorldMatrices(HumanBone::NumHumanBones, mat4(1.0f));
	std::vector<mat4> humanBoneLocalMatrices(HumanBone::NumHumanBones, mat4(1.0f));

	humanBoneLocalT.resize(HumanBone::NumHumanBones, vec3(0, 0, 0));
	humanBoneWorldR.resize(HumanBone::NumHumanBones, quat(1, 0, 0, 0));
	humanBoneLocalR.resize(HumanBone::NumHumanBones, quat(1, 0, 0, 0));
	humanBoneBindings.resize(HumanBone::NumHumanBones, UINT32_MAX);

	for (uint32_t i = 0; i < HumanBone::NumHumanBones; i++)
	{
		const char* humanBoneName = HumanBone::name(i);

		if (doc.HasMember(humanBoneName))
		{
			const char* boneName = doc[humanBoneName].GetString();

			if (boneName[0] == '*') boneName = humanBoneName;

			auto iter = boneTable.find(boneName);
			if (iter != boneTable.end())
			{
				humanBoneBindings[i] = iter->second;
				bones[iter->second].humanBoneId = i;

				quat rot;
				vec3 scale, trans;

				if (bones[iter->second].hasOffsetMatrix)
				{
					humanBoneWorldMatrices[i] = inverse(transpose(bones[iter->second].offsetMatrix));
				}
				else
				{
					uint32_t p = HumanBone::parent(i);
					if (p == HumanBone::None)
					{
						humanBoneWorldMatrices[i] = mat4(1.0f);
					}
					else
					{
						decompose(transpose(bones[iter->second].transform), trans, rot, scale);

						// column-majored
						humanBoneWorldMatrices[i] = humanBoneWorldMatrices[p] * translate(mat4(1.0f), trans);
					}
				}


				decompose(humanBoneWorldMatrices[i],
					trans, humanBoneWorldR[i], scale);

				continue;
			}
			return __LINE__;
		}
		else
		{
			uint32_t p = HumanBone::parent(i);
			if (p == HumanBone::None)
			{
				humanBoneWorldMatrices[i] = mat4(1.0f);
				humanBoneWorldR[i] = quat(1, 0, 0, 0);
			}
			else
			{
				humanBoneWorldMatrices[i] = humanBoneWorldMatrices[p];
				humanBoneWorldR[i] = humanBoneWorldR[p];
			}
		}
	}

	// get local rotation for each bone in human skeleton
	for (uint32_t i = 0; i < HumanBone::NumHumanBones; i++)
	{
		uint32_t p = HumanBone::parent(i);
		humanBoneLocalR[i] = humanBoneWorldR[i];
		humanBoneLocalMatrices[i] = humanBoneWorldMatrices[i];
		if (p != HumanBone::None)
		{
			humanBoneLocalR[i] = inverse(humanBoneWorldR[p]) * humanBoneLocalR[i];
			humanBoneLocalMatrices[i] = inverse(humanBoneWorldMatrices[p]) * humanBoneLocalMatrices[i];
		}

		{
			vec3 t, s;
			quat r;
			decompose(humanBoneLocalMatrices[i], t, r, s);
			humanBoneLocalT[i] = t;
		}
	}

	// get rootOffset and hipHeight
	{
		vec3 t, s;
		quat r;
		decompose(humanBoneWorldMatrices[HumanBone::Root], rootOffset, r, s);
		decompose(humanBoneLocalMatrices[HumanBone::Hips], t, r, s);
		hipHeight = t.y;
	}

	humanBoneCorrectionLocalR.resize(HumanBone::NumHumanBones, quat(1, 0, 0, 0));

	fp = fopen("debug.log", "w");

	// adjust 
	for (uint32_t i = HumanBone::Hips; i < HumanBone::NumHumanBones; i++)
	{
		uint32_t p = HumanBone::parent(i);
		humanBoneWorldMatrices[i] = humanBoneWorldMatrices[p] * humanBoneLocalMatrices[i];

		uint32_t t = HumanBone::target(i);
		if (t == HumanBone::None)
		{
			continue;
		}

		if (humanBoneBindings[t] == UINT32_MAX) continue;

		// vectors in parent's coordinate system
		vec3 boneStart = humanBoneLocalMatrices[i] * vec4(0, 0, 0, 1);
		vec3 boneEnd = humanBoneLocalMatrices[i] * (humanBoneLocalMatrices[t] * vec4(0, 0, 0, 1));
		vec3 boneDir = normalize(boneEnd - boneStart);

		vec3 stdBoneDir = inverse(mat3(humanBoneWorldMatrices[p])) * normalize(HumanBone::direction(i));

		float cosDelta = dot(boneDir, stdBoneDir);
		if (cosDelta > 0.999f)
		{
			humanBoneCorrectionLocalR[i] = quat(1, 0, 0, 0);
		}
		else
		{
			float delta = acosf(cosDelta);
			vec3 rotateAxis = normalize(cross(boneDir, stdBoneDir));

			humanBoneCorrectionLocalR[i] = angleAxis(delta, rotateAxis);
		}

		/*if (humanBoneBindings[t] == UINT32_MAX)
		{
			boneDir = stdBoneDir;
			humanBoneCorrectionR[i] = quat(1, 0, 0, 0);
		}*/

		quat rot;
		vec3 trans, scale;
		decompose(humanBoneLocalMatrices[i], trans, rot, scale);
		rot = humanBoneCorrectionLocalR[i] * rot;
		humanBoneLocalMatrices[i] = compose(trans, rot, scale);
		humanBoneWorldMatrices[i] = humanBoneWorldMatrices[p] * humanBoneLocalMatrices[i];

		{
			quat q = humanBoneCorrectionLocalR[i];
			vec3 v = degrees(eulerAngles(q));
			fprintf(fp, "%20s: (%10.6f, %10.6f, %10.6f) <> (%10.6f, %10.6f, %10.6f)\n                      (%10.6f, %10.6f, %10.6f, %10.6f) = [%7.2f, %7.2f, %7.2f]\n",
				HumanBone::name(i),
				boneDir.x, boneDir.y, boneDir.z,
				stdBoneDir.x, stdBoneDir.y, stdBoneDir.z,
				q.w, q.x, q.y, q.z,
				v.x, v.y, v.z
			);
		}
	}

	fclose(fp);

	return 0;
}

int32_t Model::SaveAvatar(const char * filename)
{
	rapidjson::Document doc;

	for (uint32_t i = 0; i < HumanBone::NumHumanBones; i++)
	{
		if (humanBoneBindings[i] != UINT32_MAX)
		{
			const char* humanBoneName = HumanBone::name(i);
			const char* boneName = bones[humanBoneBindings[i]].name.c_str();

			rapidjson::Value k;
			k.SetString(humanBoneName, doc.GetAllocator());

			rapidjson::Value v;
			v.SetString(boneName, doc.GetAllocator());

			doc.AddMember(k, v, doc.GetAllocator());
		}
	}

	rapidjson::StringBuffer buffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);

	FILE* fp = fopen(filename, "wb");
	fwrite(buffer.GetString(), buffer.GetSize(), 1, fp);
	fclose(fp);

	return 0;
}

void Model::GuessHumanBoneBindings()
{
	//humanBoneBindings

}

namespace
{
	enum FbxNodeName
	{
		fbxTranslation,
		fbxRotationOffset,
		fbxRotationPivot,
		fbxPreRotation,
		fbxRotation,
		fbxPostRotation,
		fbxPivotInverse,
		fbxScalingOffset,
		fbxScalingPivot,
		fbxScaling,
		fbxScalingPivotInverse,
		fbxNodeMax
	};

	std::unordered_map<std::string, uint32_t> fbxNodeNameTable =
	{
		{ "Translation", fbxTranslation },
		{ "RotationOffset", fbxRotationOffset },
		{ "RotationPivot", fbxRotationPivot },
		{ "PreRotation", fbxPreRotation },
		{ "Rotation", fbxRotation },
		{ "PostRotation", fbxPostRotation },
		{ "PivotInverse", fbxPivotInverse },
		{ "ScalingOffset", fbxScalingOffset },
		{ "ScalingPivot", fbxScalingPivot },
		{ "Scaling", fbxScaling },
		{ "ScalingPivotInverse", fbxScalingPivotInverse },
	};

	struct FbxNode
	{
		mat4 matrices[fbxNodeMax];

		FbxNode()
		{
			for (uint32_t i = 0; i < fbxNodeMax; i++)
				matrices[i] = mat4(1.0f);
		}

		void SetMatrix(FbxNodeName id, const mat4& matrix) { matrices[id] = matrix; }

		void SetMatrix(const char* name, const mat4& matrix)
		{
			auto iter = fbxNodeNameTable.find(name);
			if (iter == fbxNodeNameTable.end())
			{
				assert(false);
				return;
			}

			SetMatrix((FbxNodeName)(iter->second), matrix);
		}

		mat4 GetMatrix() const
		{
			mat4 ret = matrices[0];
			for (uint32_t i = 1; i < fbxNodeMax; i++)
			{
				ret = matrices[i] * ret;
			}
		}
	};
}

int32_t Model::LoadBones(const aiNode* node)
{
	uint32_t boneId = bones.size();

	bones.push_back(Bone());
	{
		Bone& bone = bones[boneId];
		bone.name = string(node->mName.C_Str());

		const aiMatrix4x4& m = node->mTransformation;
		bone.transform = mat4(
			m.a1, m.a2, m.a3, m.a4,
			m.b1, m.b2, m.b3, m.b4,
			m.c1, m.c2, m.c3, m.c4,
			m.d1, m.d2, m.d3, m.d4
		);

		bone.hasOffsetMatrix = 0;
		bone.offsetMatrix = mat4(1.0f);

		//// deal with "$AssimpFbx$" issue
		//{
		//	size_t idx = bone.name.find("$AssimpFbx$");
		//	if (idx != string::npos)
		//	{
		//		string basename = bone.name.substr(0, idx - 1);
		//		//size_t transIdx = idx + 12;
		//		//string transname = bone.name.substr(transIdx);

		//		bone.name = basename;

		//		while (node->mNumChildren > 0)
		//		{
		//			const aiNode* child = node->mChildren[0];

		//			string bonename(child->mName.C_Str());

		//			if (bonename != bone.name)
		//			{
		//				idx = bonename.find("$AssimpFbx$");
		//				if (idx == string::npos) break;

		//				basename = bonename.substr(0, idx - 1);
		//				if (basename != bone.name) break;
		//			}

		//			node = child;

		//			const aiMatrix4x4& m = node->mTransformation;
		//			mat4 mat = mat4(
		//				m.a1, m.a2, m.a3, m.a4,
		//				m.b1, m.b2, m.b3, m.b4,
		//				m.c1, m.c2, m.c3, m.c4,
		//				m.d1, m.d2, m.d3, m.d4
		//			);

		//			bone.transform = mat * bone.transform;
		//		}

		//	}
		//}

		boneTable.insert(pair<string, uint32_t>(bone.name, boneId));
	}

	boneTree.resize(bones.size());

	for (uint32_t i = 0; i < node->mNumChildren; i++)
	{
		const aiNode* child = node->mChildren[i];
		uint32_t childId = LoadBones(child);
		bones[childId].parent = boneId;
		boneTree[boneId].push_back(childId);
	}

	return boneId;
}

int32_t Model::LoadMeshes(const aiScene * scene)
{
	const uint32_t meshCount = scene->mNumMeshes;
	vertexCount = 0;
	indexCount = 0;

	meshVertexBases.resize(meshCount);
	meshIndexBases.resize(meshCount);
	meshIndexCounts.resize(meshCount);

	for (uint32_t i = 0; i < meshCount; i++)
	{
		const aiMesh* mesh = scene->mMeshes[i];

		meshVertexBases[i] = vertexCount;
		meshIndexBases[i] = indexCount;
		meshIndexCounts[i] = mesh->mNumFaces * 3;

		vertexCount += mesh->mNumVertices;
		indexCount += mesh->mNumFaces * 3;
	}

	positions.resize(vertexCount);
	normals.resize(vertexCount);
	texcoords.resize(vertexCount);
	boneIds.resize(vertexCount);
	boneWeights.resize(vertexCount);
	indices.resize(indexCount);

	for (uint32_t iMesh = 0; iMesh < meshCount; iMesh++)
	{
		const aiMesh* mesh = scene->mMeshes[iMesh];
		uint32_t vertexBase = meshVertexBases[iMesh];

		for (uint32_t i = 0; i < mesh->mNumVertices; i++)
		{
			positions[vertexBase + i] = vec3(
				mesh->mVertices[i].x,
				mesh->mVertices[i].y,
				mesh->mVertices[i].z);

			normals[vertexBase + i] = vec3(
				mesh->mNormals[i].x,
				mesh->mNormals[i].y,
				mesh->mNormals[i].z);

			texcoords[vertexBase + i] = vec2(
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y);
		}

		for (uint32_t i = 0; i < mesh->mNumBones; i++)
		{
			const aiBone* bone = mesh->mBones[i];

			auto iter = boneTable.find(bone->mName.C_Str());
			if (iter == boneTable.end())
			{
				return __LINE__;
			}
			uint32_t boneId = iter->second;

			const aiMatrix4x4& m = bone->mOffsetMatrix;
			bones[boneId].hasOffsetMatrix = 1;
			bones[boneId].offsetMatrix = mat4(
				m.a1, m.a2, m.a3, m.a4,
				m.b1, m.b2, m.b3, m.b4,
				m.c1, m.c2, m.c3, m.c4,
				m.d1, m.d2, m.d3, m.d4
			);

			for (uint32_t j = 0; j < bone->mNumWeights; j++)
			{
				float weight = bone->mWeights[j].mWeight;
				uint32_t vertexId = vertexBase + bone->mWeights[j].mVertexId;
				ivec4& ids = boneIds[vertexId];
				vec4& weights = boneWeights[vertexId];

				if (weights.x == 0.0f)
				{
					ids.x = boneId;
					weights.x = weight;
				}
				else if (weights.y == 0.0f)
				{
					ids.y = boneId;
					weights.y = weight;
				}
				else if (weights.z == 0.0f)
				{
					ids.z = boneId;
					weights.z = weight;
				}
				else if (weights.w == 0.0f)
				{
					ids.w = boneId;
					weights.w = weight;
				}
			}
		}

		uint32_t IndexBase = meshIndexBases[iMesh];

		for (uint32_t i = 0; i < mesh->mNumFaces; i++)
		{
			const aiFace& face = mesh->mFaces[i];
			assert(3 == face.mNumIndices);
			indices[IndexBase + i * 3] = face.mIndices[0];
			indices[IndexBase + i * 3 + 1] = face.mIndices[1];
			indices[IndexBase + i * 3 + 2] = face.mIndices[2];
		}
	}

	return 0;
}

int32_t Model::LoadAnimations(const aiScene * scene)
{
	animations.reserve(scene->mNumAnimations);
	for (uint32_t i = 0; i < scene->mNumAnimations; i++)
	{
		const aiAnimation* anim = scene->mAnimations[i];

		Animation clip = {};
		clip.name = string(anim->mName.C_Str());
		clip.lengthInTicks = float(anim->mDuration);
		clip.ticksPerSecond = float(anim->mTicksPerSecond);
		clip.length = float(anim->mDuration / anim->mTicksPerSecond);

		for (uint32_t j = 0; j < anim->mNumChannels; j++)
		{
			const aiNodeAnim* nodeAnim = anim->mChannels[j];
			Channel newChannel = {};
			Channel* channel = &newChannel;
			channel->name = nodeAnim->mNodeName.C_Str();

			bool omitT = false, omitR = false, omitS = false;

			//size_t idx = channel->name.find("$AssimpFbx$");
			//if (idx != string::npos)
			//{
			//	string basename = channel->name.substr(0, idx - 1);
			//	size_t transIdx = idx + 12;
			//	string transname = channel->name.substr(transIdx);

			//	channel->name = basename;

			//	if (transname == "Translation")
			//	{
			//		omitR = true; omitS = true;
			//	}
			//	else if (transname == "Rotation")
			//	{
			//		omitT = true; omitS = true;
			//	}
			//	else if (transname == "Scaling")
			//	{
			//		omitT = true; omitR = true;
			//	}
			//	else
			//	{
			//		return __LINE__;
			//	}

			//	for (uint32_t k = 0; k < clip.channels.size(); k++)
			//	{
			//		if (clip.channels[k].name == basename)
			//		{
			//			channel = &(clip.channels[k]);
			//			break;
			//		}
			//	}
			//}

			if (!omitT)
			{
				channel->translations.reserve(nodeAnim->mNumPositionKeys);
				for (uint32_t k = 0; k < nodeAnim->mNumPositionKeys; k++)
				{
					aiVectorKey& frame = nodeAnim->mPositionKeys[k];
					channel->translations.push_back(Vec3Frame{
						vec3(frame.mValue.x, frame.mValue.y, frame.mValue.z),
						float(frame.mTime)
					});
				}
			}

			if (!omitR)
			{
				channel->rotations.reserve(nodeAnim->mNumRotationKeys);
				for (uint32_t k = 0; k < nodeAnim->mNumRotationKeys; k++)
				{
					aiQuatKey& frame = nodeAnim->mRotationKeys[k];
					channel->rotations.push_back(QuatFrame{
						quat(frame.mValue.w, frame.mValue.x, frame.mValue.y, frame.mValue.z),
						float(frame.mTime)
					});
				}
			}

			if (!omitS)
			{
				channel->scalings.reserve(nodeAnim->mNumScalingKeys);
				for (uint32_t k = 0; k < nodeAnim->mNumScalingKeys; k++)
				{
					aiVectorKey& frame = nodeAnim->mScalingKeys[k];
					channel->scalings.push_back(Vec3Frame{
						vec3(frame.mValue.x, frame.mValue.y, frame.mValue.z),
						float(frame.mTime)
					});
				}
			}

			if (channel == &newChannel)
			{
				clip.channels.push_back(newChannel);
			}
		}

		animations.push_back(clip);
		animTable.insert(pair<string, uint32_t>(clip.name, i));
	}

	return 0;
}

