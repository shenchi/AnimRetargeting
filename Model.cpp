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
	const char* HumanBoneName[HumanBone::NumHumanBones] =
	{
		"Hips",
		"Spine",
		"Spine1",
		"Spine2",
		"Neck",
		"Head",
		"LeftShoulder",
		"LeftArm",
		"LeftForeArm",
		"LeftHand",
		"LeftHandThumb1",
		"LeftHandThumb2",
		"LeftHandThumb3",
		"LeftHandIndex1",
		"LeftHandIndex2",
		"LeftHandIndex3",
		"LeftHandMiddle1",
		"LeftHandMiddle2",
		"LeftHandMiddle3",
		"LeftHandRing1",
		"LeftHandRing2",
		"LeftHandRing3",
		"LeftHandPinky1",
		"LeftHandPinky2",
		"LeftHandPinky3",
		"RightShoulder",
		"RightArm",
		"RightForeArm",
		"RightHand",
		"RightHandThumb1",
		"RightHandThumb2",
		"RightHandThumb3",
		"RightHandIndex1",
		"RightHandIndex2",
		"RightHandIndex3",
		"RightHandMiddle1",
		"RightHandMiddle2",
		"RightHandMiddle3",
		"RightHandRing1",
		"RightHandRing2",
		"RightHandRing3",
		"RightHandPinky1",
		"RightHandPinky2",
		"RightHandPinky3",
		"LeftUpLeg",
		"LeftLeg",
		"LeftFoot",
		"LeftToeBase",
		"RightUpLeg",
		"RightLeg",
		"RightFoot",
		"RightToeBase"
	};
}

const char * HumanBone::name(uint32_t id)
{
	if (id > NumHumanBones) return nullptr;
	return HumanBoneName[id];
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

	humanBoneBindings.resize(HumanBone::NumHumanBones, UINT32_MAX);

	return 0;
}

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

	for (uint32_t i = 0; i < HumanBone::NumHumanBones; i++)
	{
		const char* humanBoneName = HumanBone::name(i);
		humanBoneBindings[i] = UINT32_MAX;
		if (doc.HasMember(humanBoneName))
		{
			const char* boneName = doc[humanBoneName].GetString();

			if (boneName[0] == '*') boneName = humanBoneName;

			auto iter = boneTable.find(boneName);
			if (iter != boneTable.end())
			{
				humanBoneBindings[i] = iter->second;
				bones[iter->second].humanBoneId = i;
				continue;
			}
			return __LINE__;
		}
	}
	
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

int32_t Model::LoadBones(const aiNode* node)
{
	uint32_t boneId = bones.size();

	bones.push_back(Bone());
	{
		Bone& bone = bones[boneId];
		bone.name = string(node->mName.C_Str());

		boneTable.insert(pair<string, uint32_t>(bone.name, boneId));
		
		/*aiVector3D t, s;
		aiQuaternion r;
		node->mTransformation.Decompose(s, r, t);
		
		bone.transform = Transform(
			vec3(t.x, t.y, t.z),
			quat(r.w, r.x, r.y, r.z),
			vec3(s.x, s.y, s.z));*/

		const aiMatrix4x4& m = node->mTransformation;
		bone.transform = mat4(
			m.a1, m.a2, m.a3, m.a4,
			m.b1, m.b2, m.b3, m.b4,
			m.c1, m.c2, m.c3, m.c4,
			m.d1, m.d2, m.d3, m.d4
		);
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
		clip.length = float(anim->mDuration / anim->mTicksPerSecond);

		for (uint32_t j = 0; j < anim->mNumChannels; j++)
		{ 
			const aiNodeAnim* nodeAnim = anim->mChannels[j];
			Channel channel = {};
			channel.name = nodeAnim->mNodeName.C_Str();

			channel.translations.reserve(nodeAnim->mNumPositionKeys);
			for (uint32_t k = 0; k < nodeAnim->mNumPositionKeys; k++)
			{
				aiVectorKey& frame = nodeAnim->mPositionKeys[k];
				channel.translations.push_back(Vec3Frame{
					vec3(frame.mValue.x, frame.mValue.y, frame.mValue.z),
					float(frame.mTime / anim->mTicksPerSecond)
				});
			}

			channel.rotations.reserve(nodeAnim->mNumRotationKeys);
			for (uint32_t k = 0; k < nodeAnim->mNumRotationKeys; k++)
			{
				aiQuatKey& frame = nodeAnim->mRotationKeys[k];
				channel.rotations.push_back(QuatFrame{
					quat(frame.mValue.w, frame.mValue.x, frame.mValue.y, frame.mValue.z),
					float(frame.mTime / anim->mTicksPerSecond)
				});
			}

			channel.scalings.reserve(nodeAnim->mNumScalingKeys);
			for (uint32_t k = 0; k < nodeAnim->mNumScalingKeys; k++)
			{
				aiVectorKey& frame = nodeAnim->mScalingKeys[k];
				channel.scalings.push_back(Vec3Frame{
					vec3(frame.mValue.x, frame.mValue.y, frame.mValue.z),
					float(frame.mTime / anim->mTicksPerSecond)
				});
			}

			clip.channels.push_back(channel);
		}

		animations.push_back(clip);
		animTable.insert(pair<string, uint32_t>(clip.name, i));
	}

	return 0;
}

