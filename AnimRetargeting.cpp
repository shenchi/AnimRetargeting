#include "AnimRetargeting.h"

#include <imgui.h>
#include <glm/gtc/quaternion.hpp>
using namespace glm;

namespace
{
	int32_t LoadFile(const char* filename, void** ptr, size_t* size)
	{
		FILE* fp = fopen(filename, "rb");
		if (nullptr == fp) return __LINE__;

		fseek(fp, 0, SEEK_END);
		*size = ftell(fp);
		fseek(fp, 0, SEEK_SET);

		*ptr = malloc(*size);
		if (1 != fread(*ptr, *size, 1, fp)) return __LINE__;
		fclose(fp);

		return 0;
	}

	mat4 transform(const vec3& t, const quat& r, const vec3& s)
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

		return mat4{
			vec4{ s.x * (a_sqr + b_sqr - c_sqr - d_sqr), s.y * (b_c_2 - a_d_2), s.z * (a_c_2 + b_d_2), t.x },
			vec4{ s.x * (a_d_2 + b_c_2), s.y * (a_sqr - b_sqr + c_sqr - d_sqr), s.z * (c_d_2 - a_b_2), t.y },
			vec4{ s.x * (b_d_2 - a_c_2), s.y * (a_b_2 + c_d_2), s.z * (a_sqr - b_sqr - c_sqr + d_sqr), t.z },
			vec4{ 0.0f, 0.0f, 0.0f, 1.0f }
		};
	}

	void decompose(const mat4& m, vec3& t, quat& r, vec3& s)
	{
		vec3 skew; vec4 persp;
		glm::decompose(m, s, r, t, skew, persp);
		r = glm::conjugate(r);
	}
}

int32_t AnimRetargeting::OnResize()
{
	int err = Application::OnResize();
	if (err) return err;

	matProj = transpose(
		perspectiveFov(
			half_pi<float>(),
			(float)bufferWidth,
			(float)bufferHeight,
			0.1f, 20.0f));

	return 0;
}

int32_t AnimRetargeting::OnInit()
{
	{
		if (LoadFile("assets/VertexShader.cso", &vsSrc, &vsSrcSize)) return __LINE__;
		if (FAILED(device->CreateVertexShader(vsSrc, vsSrcSize, nullptr, &vs)))
		{
			return __LINE__;
		}
	}

	{
		if (LoadFile("assets/PixelShader.cso", &psSrc, &psSrcSize)) return __LINE__;
		if (FAILED(device->CreatePixelShader(psSrc, psSrcSize, nullptr, &ps)))
		{
			return __LINE__;
		}
	}

	{
		D3D11_INPUT_ELEMENT_DESC descs[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 2, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 3, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 4, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		if (FAILED(device->CreateInputLayout(
			descs,
			sizeof(descs) / sizeof(D3D11_INPUT_ELEMENT_DESC),
			vsSrc, vsSrcSize, &layout)))
		{
			return __LINE__;
		}
	}

	{
		if (LoadFile("assets/VertexShader1.cso", &gizmoVsSrc, &gizmoVsSrcSize)) return __LINE__;
		if (FAILED(device->CreateVertexShader(gizmoVsSrc, gizmoVsSrcSize, nullptr, &gizmoVs)))
		{
			return __LINE__;
		}
	}

	{
		if (LoadFile("assets/PixelShader1.cso", &gizmoPsSrc, &gizmoPsSrcSize)) return __LINE__;
		if (FAILED(device->CreatePixelShader(gizmoPsSrc, gizmoPsSrcSize, nullptr, &gizmoPs)))
		{
			return __LINE__;
		}
	}

	{
		D3D11_INPUT_ELEMENT_DESC descs[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, sizeof(vec3), D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		if (FAILED(device->CreateInputLayout(
			descs,
			sizeof(descs) / sizeof(D3D11_INPUT_ELEMENT_DESC),
			gizmoVsSrc, gizmoVsSrcSize, &gizmoLayout)))
		{
			return __LINE__;
		}
	}

	{
		CD3D11_DEPTH_STENCIL_DESC desc(D3D11_DEFAULT);
		desc.DepthEnable = FALSE;

		if (FAILED(device->CreateDepthStencilState(&desc, &gizmoDSS)))
		{
			return __LINE__;
		}
	}

	//model1.Load("assets/archer_walking.fbx");
	//model1.LoadAvatar("assets/archer.json");

	model1.Load("assets/KB.fbx");
	model1.LoadAvatar("assets/KB_Movement.json");

	//model1.Load("assets/KB_Movement.fbx");
	//model1.LoadAvatar("assets/KB_Movement.json");

	model2.Load("assets/KB_Movement.fbx");
	model2.LoadAvatar("assets/KB_Movement.json");

	{
		CD3D11_BUFFER_DESC desc(
			sizeof(mat4) * 4,
			D3D11_BIND_CONSTANT_BUFFER,
			D3D11_USAGE_DYNAMIC,
			D3D11_CPU_ACCESS_WRITE);
		if (FAILED(device->CreateBuffer(&desc, nullptr, &frameConstants)))
		{
			return __LINE__;
		}
	}

	{
		uint32_t vertexCount = model1.positions.size();
		uint32_t vbSize = vertexCount * sizeof(float) * 16;
		uint8_t* base = (uint8*)malloc(vbSize), *ptr = base;

		memcpy(ptr, &(model1.positions[0]), sizeof(vec3) * vertexCount);
		ptr += sizeof(vec3) * vertexCount;
		memcpy(ptr, &(model1.normals[0]), sizeof(vec3) * vertexCount);
		ptr += sizeof(vec3) * vertexCount;
		memcpy(ptr, &(model1.texcoords[0]), sizeof(vec2) * vertexCount);
		ptr += sizeof(vec2) * vertexCount;
		memcpy(ptr, &(model1.boneIds[0]), sizeof(ivec4) * vertexCount);
		ptr += sizeof(ivec4) * vertexCount;
		memcpy(ptr, &(model1.boneWeights[0]), sizeof(vec4) * vertexCount);

		CD3D11_BUFFER_DESC desc(vbSize, D3D11_BIND_VERTEX_BUFFER);
		D3D11_SUBRESOURCE_DATA data = { base, 0, 0 };
		if (FAILED(device->CreateBuffer(&desc, &data, &model1VB)))
		{
			return __LINE__;
		}

		free(base);
	}

	{
		uint32_t ibSize = model1.indices.size() * sizeof(uint16_t);
		CD3D11_BUFFER_DESC desc(ibSize, D3D11_BIND_INDEX_BUFFER);
		D3D11_SUBRESOURCE_DATA data = { &(model1.indices[0]), 0, 0 };
		if (FAILED(device->CreateBuffer(&desc, &data, &model1IB)))
		{
			return __LINE__;
		}
	}

	{
		model1BoneMatrices.resize(model1.bones.size());
		UINT bufferSize = 128 * sizeof(mat4);

		CD3D11_BUFFER_DESC desc(bufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		if (FAILED(device->CreateBuffer(&desc, nullptr, &model1BoneBuffer)))
		{
			return __LINE__;
		}
	}

	{
		CD3D11_BUFFER_DESC desc(
			sizeof(float) * 6 * 2048,
			D3D11_BIND_VERTEX_BUFFER,
			D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		//D3D11_SUBRESOURCE_DATA data = { base, 0, 0 };
		if (FAILED(device->CreateBuffer(&desc, nullptr, &gizmoVB)))
		{
			return __LINE__;
		}
	}


	matModel = transpose(scale(mat4(1.0f), vec3(0.01f)));

	cameraPos = vec3(0, 1.0, -2.0f);
	pitch = 0.0f;
	yaw = 0.0f;
	dist = 2.0f;
	focusPoint = vec3(0, 1, 0);

	rbuttonDown = 0;
	lbuttonDown = 0;
	lastX = -1;
	lastY = -1;

	animPlayTime = 0.0f;

	return 0;
}

int32_t AnimRetargeting::OnRelease()
{
	model1BoneBuffer->Release();
	model1VB->Release();
	model1IB->Release();
	frameConstants->Release();
	layout->Release();
	vs->Release();
	ps->Release();

	gizmoDSS->Release();
	gizmoVB->Release();
	gizmoLayout->Release();
	gizmoVs->Release();
	gizmoPs->Release();

	free(vsSrc);
	free(psSrc);

	free(gizmoVsSrc);
	free(gizmoPsSrc);

	return 0;
}

int32_t AnimRetargeting::OnUpdate()
{
	GUI();

	vec3 cameraDir = quat(vec3(pitch, yaw, 0.0f)) * vec3(0, 0, 1);
	cameraPos = focusPoint - cameraDir * dist;
	matView = transpose(lookAt(cameraPos, cameraPos + cameraDir, vec3(0, 1, 0)));
	//matView = transpose(lookAt(quat(vec3(pitch, yaw, 0.0f)) * cameraPos, vec3(0, 1, 0), vec3(0, 1, 0)));

	{
		D3D11_MAPPED_SUBRESOURCE res = {};
		context->Map(frameConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
		memcpy(res.pData, &matProj, sizeof(mat4) * 4);
		context->Unmap(frameConstants, 0);
	}

	animPlayTime += deltaTime;

	if (animId < model2.animations.size())
	{

		UpdateBoneMatrices(model1, model1BoneMatrices, model2, animId, animPlayTime);
	}
	else
	{
		UpdateHumanBoneTPose(model1, model1BoneMatrices);
	}

	gizmoBuffer.clear();
	DrawSkeletal(model1, model1BoneMatrices);

	{
		D3D11_MAPPED_SUBRESOURCE res = {};
		context->Map(model1BoneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
		memcpy(res.pData, &(model1BoneMatrices[0]), sizeof(mat4) * model1BoneMatrices.size());
		context->Unmap(model1BoneBuffer, 0);
	}

	if (gizmoBuffer.size() > 0)
	{
		D3D11_MAPPED_SUBRESOURCE res = {};
		context->Map(gizmoVB, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
		memcpy(res.pData, &(gizmoBuffer[0]), sizeof(float) * gizmoBuffer.size());
		context->Unmap(gizmoVB, 0);
	}

	//float clearColor[] = { 0.7f, 0.7f, 0.7f, 1.0f };
	float clearColor[] = { 0.2f, 0.2f, 0.2f, 1.0f };
	context->ClearRenderTargetView(rtv, clearColor);
	context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);

	context->VSSetConstantBuffers(0, 1, &frameConstants);
	context->RSSetViewports(1, &CD3D11_VIEWPORT(0.0f, 0.0f, (float)bufferWidth, (float)bufferHeight));
	context->OMSetRenderTargets(1, &rtv, dsv);

	{
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(layout);
		ID3D11Buffer* vbs[] = { model1VB, model1VB, model1VB, model1VB, model1VB };
		UINT strides[] = { sizeof(vec3), sizeof(vec3) , sizeof(vec2), sizeof(ivec4), sizeof(vec4) };
		UINT offsets[] = {
			0,
			sizeof(vec3) * model1.vertexCount,
			sizeof(vec3) * 2 * model1.vertexCount,
			(sizeof(vec3) * 2 + sizeof(vec2)) * model1.vertexCount,
			(sizeof(vec3) * 2 + sizeof(vec2) + sizeof(ivec4)) * model1.vertexCount
		};
		context->IASetVertexBuffers(0, 5, vbs, strides, offsets);
		context->IASetIndexBuffer(model1IB, DXGI_FORMAT_R16_UINT, 0);

		context->VSSetShader(vs, nullptr, 0);
		context->PSSetShader(ps, nullptr, 0);

		context->VSSetConstantBuffers(1, 1, &model1BoneBuffer);

		context->OMSetDepthStencilState(nullptr, 0);

		context->DrawIndexed(model1.indexCount, 0, 0);
	}

	if (gizmoBuffer.size() > 0)
	{
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
		context->IASetInputLayout(gizmoLayout);
		ID3D11Buffer* vbs[] = { gizmoVB };
		UINT strides[] = { sizeof(vec3) * 2 };
		UINT offsets[] = { 0 };
		context->IASetVertexBuffers(0, 1, vbs, strides, offsets);

		context->VSSetShader(gizmoVs, nullptr, 0);
		context->PSSetShader(gizmoPs, nullptr, 0);

		context->OMSetDepthStencilState(gizmoDSS, 0);

		context->Draw(gizmoBuffer.size() / 6, 0);
	}

	return 0;
}

void AnimRetargeting::OnMouseDown(int button)
{
	if (button == 0)
	{
		lbuttonDown = 1;
	}
	else if (button == 1)
	{
		rbuttonDown = 1;
	}
}

void AnimRetargeting::OnMouseUp(int button)
{
	lbuttonDown = 0;
	rbuttonDown = 0;
	lastX = -1;
	lastY = -1;
}

void AnimRetargeting::OnMouseMove(int x, int y)
{
	if (lbuttonDown)
	{
		if (lastX == -1 && lastY == -1)
		{
			lastX = x;
			lastY = y;
			return;
		}

		yaw += (x - lastX) * 0.01f;
		pitch += (y - lastY) * 0.01f;

		pitch = clamp(pitch, -half_pi<float>(), half_pi<float>());

		lastX = x;
		lastY = y;
	}
	else if (rbuttonDown)
	{
		if (lastX == -1 && lastY == -1)
		{
			lastX = x;
			lastY = y;
			return;
		}

		vec3 cameraDir = quat(vec3(pitch, yaw, 0.0f)) * vec3(0, 0, 1);

		vec3 right = normalize(cross(glm::vec3(0, 1, 0), cameraDir));
		vec3 up = normalize(cross(cameraDir, right));

		focusPoint += right * -((x - lastX) * 0.01f)
			+ up * ((y - lastY) * 0.01f);

		lastX = x;
		lastY = y;
	}
}

void AnimRetargeting::OnMouseWheel(int delta)
{
	dist += delta * 0.0005f;
	dist = clamp(dist, 1.5f, 3.5f);
}

void AnimRetargeting::GUI()
{
	static bool showDemo = true;
	ImGui::ShowDemoWindow(&showDemo);

	ImGui::Text("Debug");

	ImGui::Button("Load Model");
	ImGui::SameLine();
	ImGui::Button("Load Animation");

	if (ImGui::Button("Select Animation"))
		ImGui::OpenPopup("SelectAnimation");

	if (animId < model2.animations.size())
	{
		ImGui::SameLine();
		ImGui::Text(model2.animations[animId].name.c_str());
	}

	if (ImGui::BeginPopup("SelectAnimation"))
	{
		if (ImGui::Selectable("(none)"))
			animId = UINT32_MAX;
		for (uint32_t i = 0; i < model2.animations.size(); i++)
		{
			if (ImGui::Selectable(model2.animations[i].name.c_str()))
				animId = i;
		}
		ImGui::EndPopup();
	}

	ImGui::Checkbox("Draw bone axis", &drawBoneAxis);
	ImGui::SameLine();
	ImGui::Checkbox("Show human bones only", &showHumanBonesOnly);
	ImGui::InputFloat("Bone axis length", &boneAxisLength);
	boneAxisLength = max(boneAxisLength, 0.0f);

	if (model1.bones.size() > 0)
	{
		ImGui::Separator();
		GUI_BoneTree(0);
	}

}

void AnimRetargeting::GUI_BoneTree(uint32_t boneId)
{
	bool isLeaf = model1.boneTree[boneId].empty();

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	if (isLeaf) flags |= ImGuiTreeNodeFlags_Leaf;
	else flags |= ImGuiTreeNodeFlags_DefaultOpen;

	const Bone& bone = model1.bones[boneId];

	bool opened = false;
	if (bone.humanBoneId == UINT32_MAX)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));
		opened = ImGui::TreeNodeEx(
			(void*)(intptr_t)boneId,
			flags,
			bone.name.c_str());
		ImGui::PopStyleColor();
	}
	else
	{
		opened = ImGui::TreeNodeEx(
			(void*)(intptr_t)boneId,
			flags,
			"%s[%s]",
			bone.name.c_str(),
			HumanBone::name(bone.humanBoneId)
		);
	}

	if (opened)
	{
		for (uint32_t i = 0; i < model1.boneTree[boneId].size(); i++)
		{
			GUI_BoneTree(model1.boneTree[boneId][i]);
		}

		ImGui::TreePop();
	}
}

void AnimRetargeting::AddLine(const glm::vec3 & from, const glm::vec3 & to, const glm::vec3 & color)
{
	gizmoBuffer.push_back(from.x);
	gizmoBuffer.push_back(from.y);
	gizmoBuffer.push_back(from.z);

	gizmoBuffer.push_back(color.x);
	gizmoBuffer.push_back(color.y);
	gizmoBuffer.push_back(color.z);

	gizmoBuffer.push_back(to.x);
	gizmoBuffer.push_back(to.y);
	gizmoBuffer.push_back(to.z);

	gizmoBuffer.push_back(color.x);
	gizmoBuffer.push_back(color.y);
	gizmoBuffer.push_back(color.z);
}

glm::vec3 AnimRetargeting::SampleVec3Sequence(const std::vector<Vec3Frame>& frames, float time)
{
	if (frames.size() == 1)
	{
		return frames[0].value;
	}
	else if (frames.size() > 1)
	{
		for (uint32_t i = 0; i < frames.size(); i++)
		{
			if (time < frames[i].time)
			{
				float t = (time - frames[i - 1].time) / (frames[i].time - frames[i - 1].time);
				return mix(frames[i - 1].value, frames[i].value, t);
			}
		}
	}
	assert(false);
	return glm::vec3();
}

glm::quat AnimRetargeting::SampleQuatSequence(const std::vector<QuatFrame>& frames, float time)
{
	if (frames.size() == 1)
	{
		return frames[0].value;
	}
	else if (frames.size() > 1)
	{
		for (uint32_t i = 0; i < frames.size(); i++)
		{
			if (time < frames[i].time)
			{
				float t = (time - frames[i - 1].time) / (frames[i].time - frames[i - 1].time);
				return slerp(frames[i - 1].value, frames[i].value, t);
			}
		}
	}
	assert(false);
	return glm::quat();
}

void AnimRetargeting::UpdateBoneMatrices(const Model & model, std::vector<glm::mat4>& matrices, uint32_t animId, float time)
{
	for (uint32_t i = 0; i < model.bones.size(); i++)
	{
		matrices[i] = model.bones[i].transform;
	}

	if (animId < model.animations.size())
	{

		const Animation& anim = model.animations[animId];
		time = std::fmodf(time, anim.length);

		for (uint32_t i = 0; i < anim.channels.size(); i++)
		{
			const Channel& chnl = anim.channels[i];
			auto iter = model.boneTable.find(chnl.name);
			assert(iter != model.boneTable.end());
			uint32_t boneId = iter->second;

			vec3 t = SampleVec3Sequence(chnl.translations, time);
			quat r = SampleQuatSequence(chnl.rotations, time);
			vec3 s = SampleVec3Sequence(chnl.scalings, time);

			matrices[boneId] = transform(t, r, s);
		}

	}

	for (uint32_t i = 1; i < model.bones.size(); i++)
	{
		uint32_t p = model.bones[i].parent;
		matrices[i] = matrices[i] * matrices[p];
	}

	for (uint32_t i = 0; i < model.bones.size(); i++)
	{
		matrices[i] = model.bones[i].offsetMatrix * matrices[i];
	}
}

void AnimRetargeting::UpdateBoneMatrices(const Model & model, std::vector<glm::mat4>& matrices, const Model& animModel, uint32_t animId, float time)
{
	for (uint32_t i = 0; i < model.bones.size(); i++)
	{
		matrices[i] = model.bones[i].transform;
	}

	if (animId < animModel.animations.size())
	{

		const Animation& anim = animModel.animations[animId];
		time = std::fmodf(time, anim.length);

		for (uint32_t i = 0; i < anim.channels.size(); i++)
		{
			const Channel& chnl = anim.channels[i];
			auto iter = animModel.boneTable.find(chnl.name);
			assert(iter != animModel.boneTable.end());
			uint32_t srcBoneId = iter->second;

			uint32_t humanBoneId = animModel.bones[srcBoneId].humanBoneId;
			if (humanBoneId == UINT32_MAX) continue;

			uint32_t dstBoneId = model.humanBoneBindings[humanBoneId];
			if (dstBoneId == UINT32_MAX) continue;

			vec3 t = SampleVec3Sequence(chnl.translations, time);
			quat r = SampleQuatSequence(chnl.rotations, time);
			vec3 s = SampleVec3Sequence(chnl.scalings, time);
			//
			//decompose(transpose(animModel.bones[srcBoneId].transform), t, r, s);

			uint32_t parentId = animModel.bones[srcBoneId].parent;
			uint32_t parentHumanBoneId = animModel.bones[parentId].humanBoneId;
			if (parentHumanBoneId != UINT32_MAX)
			{
				quat TPoseLocalR = animModel.humanBoneLocalR[humanBoneId];

				//quat deltaR = inverse(TPoseLocalR) * r;
				quat deltaR = r * inverse(TPoseLocalR);

				quat srcParentWorldR = animModel.humanBoneWorldR[parentHumanBoneId];
				r = srcParentWorldR * deltaR * inverse(srcParentWorldR);
				//r = inverse(srcParentWorldR) * deltaR * srcParentWorldR;

				quat dstParentWorldR = model.humanBoneWorldR[parentHumanBoneId];
				r = inverse(dstParentWorldR) * r * dstParentWorldR;
				//r = dstParentWorldR * r * inverse(dstParentWorldR);

				//r = model.humanBoneLocalR[humanBoneId] * r;
				r = r * model.humanBoneLocalR[humanBoneId];
			}

			vec3 tt;
			quat rr;
			if (humanBoneId == HumanBone::Hips || humanBoneId == HumanBone::Root)
			{
				decompose(transpose(matrices[dstBoneId]), tt, rr, s);
			}
			else
			{
				decompose(transpose(matrices[dstBoneId]), t, rr, s);
			}

			matrices[dstBoneId] = transform(t, r, s);

		}

	}

	for (uint32_t i = 1; i < model.bones.size(); i++)
	{
		uint32_t p = model.bones[i].parent;
		matrices[i] = matrices[i] * matrices[p];

		uint32_t hId = model.bones[i].humanBoneId;
		if (hId != UINT32_MAX)
		{
			matrices[i] *= mat4_cast(model.humanBoneCorrectionR[hId]);
		}
	}

	for (uint32_t i = 0; i < model.bones.size(); i++)
	{
		matrices[i] = model.bones[i].offsetMatrix * matrices[i];
	}
}

void AnimRetargeting::UpdateHumanBoneTPose(const Model & model, std::vector<glm::mat4>& matrices)
{
	for (uint32_t i = 0; i < model.bones.size(); i++)
	{
		matrices[i] = model.bones[i].transform;
	}

	for (uint32_t i = 1; i < model.bones.size(); i++)
	{
		uint32_t p = model.bones[i].parent;
		matrices[i] = matrices[i] * matrices[p];
	}

	//uint32_t hips = model.humanBoneBindings[HumanBone::Hips];
	//uint32_t spine = model.humanBoneBindings[HumanBone::Spine];
	//uint32_t leftUpLet = model.humanBoneBindings[HumanBone::LeftUpLeg];
	//uint32_t rightUpLet = model.humanBoneBindings[HumanBone::RightUpLeg];

	///*{
	//	vec3 t1, s1; quat r1;
	//	vec3 t2, s2; quat r2;
	//	decompose(transpose(matrices[id1]), t1, r1, s1);
	//	decompose(transpose(matrices[id2]), t2, r2, s2);

	//	vec3 v1 = vec4(0, 0, 0, 1) * matrices[id1];
	//	vec3 v2 = vec4(0, 0, 0, 1) * matrices[id2];

	//	vec3 actualVec = normalize(v2 - v1);
	//	vec3 proposedVec = vec3(0, 1, 0);
	//}*/

	for (uint32_t i = 0; i < HumanBone::NumHumanBones; i++)
	{
		uint32_t boneId = model.humanBoneBindings[i];
		if (boneId == UINT32_MAX) continue;

		matrices[boneId] = inverse(model.bones[boneId].offsetMatrix);
	}

	for (uint32_t i = 0; i < model.bones.size(); i++)
	{
		matrices[i] = model.bones[i].offsetMatrix * matrices[i];
	}
}

void AnimRetargeting::DrawSkeletal(const Model & model)
{
	if (model.bones.empty())
	{
		return;
	}

	std::vector<mat4> matrices;
	matrices.reserve(model.bones.size());
	matrices.push_back(model.bones[0].transform);

	for (uint32_t i = 1; i < model.bones.size(); i++)
	{
		if (showHumanBonesOnly && model.bones[i].humanBoneId == UINT32_MAX) continue;

		uint32_t p = model.bones[i].parent;

		matrices.push_back(model.bones[i].transform * matrices[p]);

		vec3 startPoint = vec4(0, 0, 0, 1) * matrices[p];
		vec3 endPoint = vec4(0, 0, 0, 1) * matrices[i];

		AddLine(startPoint, endPoint, vec3(0, 0, 1));

		if (drawBoneAxis)
		{
			vec3 x = vec4(1, 0, 0, 1) * matrices[p];
			x = startPoint + normalize(x - startPoint) * boneAxisLength;

			vec3 y = vec4(0, 1, 0, 1) * matrices[p];
			y = startPoint + normalize(y - startPoint) * boneAxisLength;

			vec3 z = vec4(0, 0, 1, 1) * matrices[p];
			z = startPoint + normalize(z - startPoint) * boneAxisLength;

			AddLine(startPoint, x, vec3(1, 0, 0));
			AddLine(startPoint, y, vec3(0, 1, 0));
			AddLine(startPoint, z, vec3(0, 0, 1));
		}
	}
}

void AnimRetargeting::DrawSkeletal(const Model & model, std::vector<glm::mat4>& matrices)
{
	if (matrices.empty())
	{
		return;
	}

	for (uint32_t i = 1; i < matrices.size(); i++)
	{
		if (showHumanBonesOnly && model.bones[i].humanBoneId == UINT32_MAX) continue;

		uint32_t p = model.bones[i].parent;

		mat4 matP = inverse(model.bones[p].offsetMatrix) * matrices[p];
		mat4 matC = inverse(model.bones[i].offsetMatrix) * matrices[i];
		vec3 startPoint = vec4(0, 0, 0, 1) * matP;
		vec3 endPoint = vec4(0, 0, 0, 1) * matC;

		AddLine(startPoint, endPoint, vec3(0, 1, 1));

		if (drawBoneAxis)
		{
			vec3 x = vec4(1, 0, 0, 1) * matP;
			x = startPoint + normalize(x - startPoint) * boneAxisLength;

			vec3 y = vec4(0, 1, 0, 1) * matP;
			y = startPoint + normalize(y - startPoint) * boneAxisLength;

			vec3 z = vec4(0, 0, 1, 1) * matP;
			z = startPoint + normalize(z - startPoint) * boneAxisLength;

			AddLine(startPoint, x, vec3(1, 0, 0));
			AddLine(startPoint, y, vec3(0, 1, 0));
			AddLine(startPoint, z, vec3(0, 0, 1));
		}
	}
}
