#include "Model.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;
using namespace glm;

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

	return 0;
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

	for (uint32_t i = 0; i < node->mNumChildren; i++)
	{
		const aiNode* child = node->mChildren[i];
		uint32_t childId = LoadBones(child);
		bones[childId].parent = boneId;
	}

	return boneId;
}

int32_t Model::LoadMeshes(const aiScene * scene)
{
	const uint32_t meshCount = scene->mNumMeshes;
	uint32_t vertexCount = 0;
	uint32_t indexCount = 0;

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
