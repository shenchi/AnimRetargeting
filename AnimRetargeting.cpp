#include "AnimRetargeting.h"

#include <assimp/Importer.hpp>
#include <assimp/Exporter.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <imgui.h>
#include <glm/gtc/quaternion.hpp>
using namespace glm;

#ifndef _DEBUG

#ifdef assert
#undef assert
#endif

#define assert(x) if (!(x)) abort();

#endif

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

	ResetCamera();

	rbuttonDown = 0;
	lbuttonDown = 0;
	lastX = -1;
	lastY = -1;

	uint32_t id = 0;
	//OpenModel("assets\\Soldier.fbx");
	//openedModels[id++].model->LoadAvatar("assets\\Soldier.json");

	OpenModel("assets\\archer_running.fbx");
	openedModels[id++].model->LoadAvatar("assets\\archer.json");

	OpenModel("assets\\KB_Kicks.fbx");
	openedModels[id++].model->LoadAvatar("assets\\KB_Jumping.json");

	return 0;
}

int32_t AnimRetargeting::OnRelease()
{
	for (uint32_t i = 0; i < openedModels.size(); i++)
	{
		ModelContext& ctx = openedModels[i];

		delete ctx.model;
		ctx.vb->Release();
		ctx.ib->Release();
		if (nullptr != ctx.boneBuffer) ctx.boneBuffer->Release();
	}

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

	if (selectedModelIdx < openedModels.size())
	{
		matModel = transpose(scale(mat4(1.0f), 
			vec3(openedModels[selectedModelIdx].model->scale)
		));
	}
	else
	{
		matModel = transpose(scale(mat4(1.0f), vec3(modelScale)));
	}

	{
		D3D11_MAPPED_SUBRESOURCE res = {};
		context->Map(frameConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
		memcpy(res.pData, &matProj, sizeof(mat4) * 4);
		context->Unmap(frameConstants, 0);
	}

	gizmoBuffer.clear();

	if (selectedModelIdx < openedModels.size())
	{
		ModelContext& ctx = openedModels[selectedModelIdx];

		const Model* animModel = nullptr;
		const Animation* anim = nullptr;

		if (selectedAnimModelIdx < openedModels.size())
		{
			animModel = openedModels[selectedAnimModelIdx].model;
			if (selectedAnimIdx < animModel->animations.size())
			{
				anim = &(animModel->animations[selectedAnimIdx]);
			}
		}

		if (nullptr != anim)
		{
			if (isAnimPlaying)
			{
				animPlaybackTime += deltaTime * animPlaybackSpeed;
				animPlaybackTime = fmod(animPlaybackTime, anim->length);
			}

			if (!ctx.model->humanBoneBindings.empty() &&
				!animModel->humanBoneBindings.empty())
			{
				UpdateBoneMatrices(*ctx.model, ctx.boneMatrices, *animModel, selectedAnimIdx, animPlaybackTime);
			}
			else if (selectedModelIdx == selectedAnimModelIdx)
			{
				UpdateBoneMatrices(*ctx.model, ctx.boneMatrices, selectedAnimIdx, animPlaybackTime);
			}
		}
		else
		{
			UpdateHumanBoneTPose(*ctx.model, ctx.boneMatrices);
		}

		{
			D3D11_MAPPED_SUBRESOURCE res = {};
			context->Map(ctx.boneBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
			memcpy(res.pData, &(ctx.boneMatrices[0]), sizeof(mat4) * ctx.boneMatrices.size());
			context->Unmap(ctx.boneBuffer, 0);
		}

		if (showBones)
		{
			DrawSkeletal(*ctx.model, ctx.boneMatrices);
		}
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

	if (selectedModelIdx < openedModels.size())
	{
		ModelContext& ctx = openedModels[selectedModelIdx];

		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(layout);

		ID3D11Buffer* vbs[] = { ctx.vb, ctx.vb, ctx.vb, ctx.vb, ctx.vb };
		UINT strides[] = { sizeof(vec3), sizeof(vec3) , sizeof(vec2), sizeof(ivec4), sizeof(vec4) };
		UINT offsets[] = {
			0,
			sizeof(vec3) * ctx.model->vertexCount,
			sizeof(vec3) * 2 * ctx.model->vertexCount,
			(sizeof(vec3) * 2 + sizeof(vec2)) * ctx.model->vertexCount,
			(sizeof(vec3) * 2 + sizeof(vec2) + sizeof(ivec4)) * ctx.model->vertexCount
		};
		context->IASetVertexBuffers(0, 5, vbs, strides, offsets);
		context->IASetIndexBuffer(ctx.ib, DXGI_FORMAT_R16_UINT, 0);

		context->VSSetShader(vs, nullptr, 0);
		context->PSSetShader(ps, nullptr, 0);

		context->VSSetConstantBuffers(1, 1, &(ctx.boneBuffer));

		context->OMSetDepthStencilState(nullptr, 0);

		for (uint32_t i = 0; i < ctx.model->meshIndexBases.size(); i++)
		{
			context->DrawIndexed(ctx.model->meshIndexCounts[i],
				ctx.model->meshIndexBases[i],
				ctx.model->meshVertexBases[i]);
		}
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

		context->Draw(uint32_t(gizmoBuffer.size()) / 6u, 0);
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
	dist = clamp(dist, 0.5f, 3.5f);
}

void AnimRetargeting::GUI()
{
	static bool showDemo = true;
	ImGui::ShowDemoWindow(&showDemo);

	GUI_Models();

	GUI_Animations();
}

void AnimRetargeting::GUI_Models()
{
	if (ImGui::Begin("Models"))
	{
		if (ImGui::Button("Reset Camera"))
		{
			ResetCamera();
		}

		ImGui::SameLine();

		if (ImGui::Button("Open Model"))
		{
			wchar_t initialDir[1024] = {};
			GetCurrentDirectory(1024, initialDir);

			wchar_t filename[1024] = {};
			OPENFILENAME ofn = {};
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFilter = L"*.fbx\0*.fbx\0";
			ofn.lpstrFile = filename;
			ofn.nMaxFile = 1024;
			ofn.lpstrInitialDir = initialDir;

			if (GetOpenFileName(&ofn))
			{
				std::wstring wstr(filename);
				std::string str(wstr.begin(), wstr.end());

				OpenModel(str.c_str());
			}
		}

		if (selectedModelIdx < openedModels.size())
		{
			ImGui::SameLine();

			if (ImGui::Button("Open Avatar"))
			{
				wchar_t initialDir[1024] = {};
				GetCurrentDirectory(1024, initialDir);

				wchar_t filename[1024] = {};
				OPENFILENAME ofn = {};
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFilter = L"*.json\0*.json\0";
				ofn.lpstrFile = filename;
				ofn.nMaxFile = 1024;
				ofn.lpstrInitialDir = initialDir;

				if (GetOpenFileName(&ofn))
				{
					std::wstring wstr(filename);
					std::string str(wstr.begin(), wstr.end());

					openedModels[selectedModelIdx].model->LoadAvatar(str.c_str());
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Close Model"))
			{
				CloseModel(openedModels[selectedModelIdx].name.c_str());
			}
		}

		selectedModelIdx = clamp(selectedModelIdx, 0u, uint32_t(openedModels.size() - 1));

		const char* previewd = "(select models)";
		if (selectedModelIdx < openedModels.size())
			previewd = openedModels[selectedModelIdx].name.c_str();

		if (ImGui::BeginCombo("Models", previewd))
		{
			for (uint32_t i = 0; i < openedModels.size(); i++)
			{
				bool is_selected = (selectedModelIdx == i);
				if (ImGui::Selectable(openedModels[i].name.c_str(), is_selected))
					selectedModelIdx = i;
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		if (selectedModelIdx < openedModels.size())
		{
			ImGui::InputFloat("Model Scale", &(openedModels[selectedModelIdx].model->scale));
		}
		else
		{
			ImGui::InputFloat("Model Scale", &modelScale);
		}

		ImGui::Checkbox("Draw Bones", &showBones);
		if (showBones)
		{
			ImGui::SameLine();
			ImGui::Checkbox("Draw bone axis", &drawBoneAxis);
			ImGui::SameLine();
			ImGui::Checkbox("Show human bones only", &showHumanBonesOnly);
			ImGui::InputFloat("Bone axis length", &boneAxisLength);
			boneAxisLength = max(boneAxisLength, 0.0f);
		}

		if (selectedModelIdx < openedModels.size() && openedModels[selectedModelIdx].model->bones.size() > 0)
		{
			ImGui::Separator();
			GUI_BoneTree(*(openedModels[selectedModelIdx].model), 0);
		}

		ImGui::End();
	}
}

void AnimRetargeting::GUI_Animations()
{
	if (ImGui::Begin("Animations"))
	{
		selectedAnimModelIdx = clamp(selectedAnimModelIdx, 0u, uint32_t(openedModels.size()));

		const Model* selectedModel = nullptr;
		if (selectedAnimModelIdx < openedModels.size())
			selectedModel = openedModels[selectedAnimModelIdx].model;

		if (nullptr != selectedModel)
		{
			selectedAnimIdx = clamp(selectedAnimIdx, 0u,
				uint32_t(selectedModel->animations.size()));
		}

		const Animation* selectedAnim = nullptr;

		const char* previewed = "(select animation)";

		if (nullptr != selectedModel &&
			selectedAnimIdx < selectedModel->animations.size())
		{
			selectedAnim = &(selectedModel->animations[selectedAnimIdx]);
			previewed = selectedAnim->name.c_str();
		}

		if (ImGui::BeginCombo("Animations", previewed))
		{
			for (uint32_t i = 0; i < openedModels.size(); i++)
			{ 
				const Model* model = openedModels[i].model;
				for (uint32_t j = 0; j < model->animations.size(); j++)
				{
					bool is_selected = (selectedAnimModelIdx == i && selectedAnimIdx == j);

					std::string label = model->animations[j].name + " - " + openedModels[i].name;
					if (ImGui::Selectable(label.c_str(), is_selected))
					{
						selectedAnimModelIdx = i;
						selectedAnimIdx = j;
					}

					if (is_selected)
						ImGui::SetItemDefaultFocus();
				}
			}

			ImGui::EndCombo();
		}

		if (!isAnimPlaying)
		{
			if (ImGui::Button("Play"))
			{
				isAnimPlaying = true;
			}
		}
		else
		{
			if (ImGui::Button("Pause"))
			{
				isAnimPlaying = false;
			}
		}

		if (nullptr != selectedAnim &&
			!selectedModel->humanBoneBindings.empty() &&
			selectedModelIdx < openedModels.size() &&
			!openedModels[selectedModelIdx].model->humanBoneBindings.empty())
		{
			ImGui::SameLine();

			if (ImGui::Button("Export"))
			{

				wchar_t initialDir[1024] = {};
				GetCurrentDirectory(1024, initialDir);

				wchar_t filename[1024] = {};
				OPENFILENAME ofn = {};
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFilter = L"*.x\0*.x\0";
				ofn.lpstrFile = filename;
				ofn.nMaxFile = 1024;
				ofn.lpstrInitialDir = initialDir;

				if (GetSaveFileName(&ofn))
				{
					std::wstring wstr(filename);
					std::string str(wstr.begin(), wstr.end());

					ExportAnimation(str.c_str(), openedModels[selectedModelIdx],
						openedModels[selectedAnimModelIdx], selectedAnimIdx);
				}
			}
		}

		ImGui::InputFloat("Playback Speed", &animPlaybackSpeed);
		animPlaybackSpeed = clamp(animPlaybackSpeed, 0.01f, 20.0f);

		float playbackTime = animPlaybackTime;
		ImGui::SliderFloat("Timeline", &playbackTime, 0.0f, selectedAnim ? selectedAnim->length : 1.0f, "%.2fs");
		animPlaybackTime = playbackTime;

		ImGui::End();
	}
}

void AnimRetargeting::GUI_BoneTree(const Model& model, uint32_t boneId)
{
	bool isLeaf = model.boneTree[boneId].empty();

	ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

	if (isLeaf) flags |= ImGuiTreeNodeFlags_Leaf;
	else flags |= ImGuiTreeNodeFlags_DefaultOpen;

	const Bone& bone = model.bones[boneId];

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
		for (uint32_t i = 0; i < model.boneTree[boneId].size(); i++)
		{
			GUI_BoneTree(model, model.boneTree[boneId][i]);
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

glm::vec3 AnimRetargeting::SampleVec3Sequence(const std::vector<Vec3Frame>& frames, float time, const glm::vec3& defaultValue)
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

	return defaultValue;
}

glm::quat AnimRetargeting::SampleQuatSequence(const std::vector<QuatFrame>& frames, float time, const glm::quat& defaultValue)
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

	return defaultValue;
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
		float ticks = time * anim.ticksPerSecond;
		ticks = std::fmodf(ticks, anim.lengthInTicks);

		for (uint32_t i = 0; i < anim.channels.size(); i++)
		{
			const Channel& chnl = anim.channels[i];
			auto iter = model.boneTable.find(chnl.name);
			assert(iter != model.boneTable.end());
			uint32_t boneId = iter->second;

			vec3 t = SampleVec3Sequence(chnl.translations, ticks);
			quat r = SampleQuatSequence(chnl.rotations, ticks);
			vec3 s = SampleVec3Sequence(chnl.scalings, ticks, vec3(1.0f));

			matrices[boneId] = transpose(compose(t, r, s));

			/*mat4 parentMat(1.0f);
			uint32_t p = model.bones[boneId].parent;
			if (p != UINT32_MAX)
			{
				parentMat = model.bones[p].offsetMatrix;
			}
			mat4 bindpose = inverse(model.bones[boneId].offsetMatrix) * parentMat;

			matrices[boneId] = transpose(compose(vec3(0.0f), r, s)) * bindpose;*/
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
		float ticks = time * anim.ticksPerSecond;
		ticks = std::fmodf(ticks, anim.lengthInTicks);

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

			vec3 t = SampleVec3Sequence(chnl.translations, ticks);
			quat r = SampleQuatSequence(chnl.rotations, ticks);
			vec3 s = SampleVec3Sequence(chnl.scalings, ticks, vec3(1.0f));
			//
			//decompose(transpose(animModel.bones[srcBoneId].transform), t, r, s);

			if (!CorrectHumanBoneRotation(model, animModel, r, humanBoneId))
				continue;

			//uint32_t parentId = animModel.bones[srcBoneId].parent;
			//uint32_t parentHumanBoneId = animModel.bones[parentId].humanBoneId;
			//if (parentHumanBoneId != UINT32_MAX)
			//{
			//	quat TPoseLocalR = animModel.humanBoneCorrectionLocalR[humanBoneId] *
			//		animModel.humanBoneLocalR[humanBoneId];

			//	//quat deltaR = inverse(TPoseLocalR) * r;
			//	quat deltaR = r * inverse(TPoseLocalR);

			//	quat srcParentWorldR = animModel.humanBoneWorldR[parentHumanBoneId];
			//	r = srcParentWorldR * deltaR * inverse(srcParentWorldR);
			//	//r = inverse(srcParentWorldR) * deltaR * srcParentWorldR;

			//	quat dstParentWorldR = model.humanBoneWorldR[parentHumanBoneId];
			//	r = inverse(dstParentWorldR) * r * dstParentWorldR;
			//	//r = dstParentWorldR * r * inverse(dstParentWorldR);


			//	quat modelStdTPoseR = model.humanBoneCorrectionLocalR[humanBoneId] *
			//		model.humanBoneLocalR[humanBoneId];

			//	//r = model.humanBoneLocalR[humanBoneId] * r;
			//	r = r * modelStdTPoseR;

			//}

			vec3 tt;
			quat rr;
			if (humanBoneId == HumanBone::Root)
			{
				decompose(transpose(matrices[dstBoneId]), tt, rr, s);

				t = ((t - animModel.rootOffset) * animModel.scale) / model.scale + model.rootOffset;
			}
			else if (humanBoneId == HumanBone::Hips)
			{
				decompose(transpose(matrices[dstBoneId]), tt, rr, s);

				t.x = t.x * animModel.scale / model.scale;
				t.y = (t.y - animModel.hipHeight) * animModel.scale / model.scale + model.hipHeight;
				t.z = t.z * animModel.scale / model.scale;
			}
			else
			{
				decompose(transpose(matrices[dstBoneId]), t, rr, s);
			}

			matrices[dstBoneId] = transpose(compose(t, r, s));

		}

	}

	for (uint32_t i = 1; i < model.bones.size(); i++)
	{
		uint32_t p = model.bones[i].parent;
		matrices[i] = matrices[i] * matrices[p];

		/*uint32_t hId = model.bones[i].humanBoneId;
		if (hId != UINT32_MAX)
		{
			matrices[i] *= mat4_cast(model.humanBoneCorrectionR[hId]);
		}*/
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

	if (!model.humanBoneBindings.empty())
	{
		for (uint32_t i = 0; i < HumanBone::NumHumanBones; i++)
		{
			uint32_t boneId = model.humanBoneBindings[i];
			if (boneId == UINT32_MAX || !model.bones[boneId].hasOffsetMatrix) continue;

			matrices[boneId] = inverse(model.bones[boneId].offsetMatrix);
		}
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

void AnimRetargeting::ResetCamera()
{
	cameraPos = vec3(0, 1.0, -2.0f);
	pitch = 0.0f;
	yaw = 0.0f;
	dist = 2.0f;
	focusPoint = vec3(0, 1, 0);
}

int32_t AnimRetargeting::OpenModel(const char * filename)
{
	auto iter = openedModelTable.find(filename);
	if (iter != openedModelTable.end())
	{
		return 0;
	}

	ModelContext ctx = {};
	ctx.model = new Model();
	int32_t err = ctx.model->Load(filename);
	if (err) { return err; }

	ctx.name = filename;
	Model& model = *ctx.model;

	{
		uint32_t vertexCount = model.positions.size();
		uint32_t vbSize = vertexCount * sizeof(float) * 16;
		uint8_t* base = (uint8*)malloc(vbSize), *ptr = base;

		memcpy(ptr, &(model.positions[0]), sizeof(vec3) * vertexCount);
		ptr += sizeof(vec3) * vertexCount;
		memcpy(ptr, &(model.normals[0]), sizeof(vec3) * vertexCount);
		ptr += sizeof(vec3) * vertexCount;
		memcpy(ptr, &(model.texcoords[0]), sizeof(vec2) * vertexCount);
		ptr += sizeof(vec2) * vertexCount;
		memcpy(ptr, &(model.boneIds[0]), sizeof(ivec4) * vertexCount);
		ptr += sizeof(ivec4) * vertexCount;
		memcpy(ptr, &(model.boneWeights[0]), sizeof(vec4) * vertexCount);

		CD3D11_BUFFER_DESC desc(vbSize, D3D11_BIND_VERTEX_BUFFER);
		D3D11_SUBRESOURCE_DATA data = { base, 0, 0 };
		if (FAILED(device->CreateBuffer(&desc, &data, &(ctx.vb))))
		{
			return __LINE__;
		}

		free(base);
	}

	{
		uint32_t ibSize = model.indices.size() * sizeof(uint16_t);
		CD3D11_BUFFER_DESC desc(ibSize, D3D11_BIND_INDEX_BUFFER);
		D3D11_SUBRESOURCE_DATA data = { &(model.indices[0]), 0, 0 };
		if (FAILED(device->CreateBuffer(&desc, &data, &(ctx.ib))))
		{
			return __LINE__;
		}
	}

	if (!model.bones.empty())
	{
		ctx.boneMatrices.resize(model.bones.size());
		UINT bufferSize = 256 * sizeof(mat4);

		CD3D11_BUFFER_DESC desc(bufferSize, D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		if (FAILED(device->CreateBuffer(&desc, nullptr, &(ctx.boneBuffer))))
		{
			return __LINE__;
		}
	}

	uint32_t idx = openedModels.size();
	openedModels.push_back(ctx);
	openedModelTable.insert(std::pair<std::string, uint32_t>(filename, idx));

	return 0;
}

int32_t AnimRetargeting::CloseModel(const char * filename)
{
	auto iter = openedModelTable.find(filename);
	if (iter == openedModelTable.end())
	{
		return 0;
	}

	{
		ModelContext& ctx = openedModels[iter->second];
		delete ctx.model;
		ctx.vb->Release();
		ctx.ib->Release();
		if (nullptr != ctx.boneBuffer) ctx.boneBuffer->Release();
	}

	openedModels.erase(openedModels.begin() + iter->second);
	openedModelTable.erase(iter);

	return 0;
}

int32_t AnimRetargeting::ExportAnimation(const char * filename, const ModelContext & meshModel, const ModelContext & animModel, uint32_t animId)
{
	if (animId >= animModel.model->animations.size())
	{
		return __LINE__;
	}

	const Model& srcModel = *animModel.model;
	const Model& dstModel = *meshModel.model;
	const Animation& srcAnim = animModel.model->animations[animId];

	uint32_t flags = //aiProcess_Triangulate |
		//aiProcess_ConvertToLeftHanded |
		aiProcess_LimitBoneWeights;
	Assimp::Importer importer;

	aiScene* scene = const_cast<aiScene*>(importer.ReadFile(meshModel.name, flags));
	
	if (nullptr == scene)
	{
		MessageBoxA(hWnd, importer.GetErrorString(), "Error", MB_OK);
		return __LINE__;
	}

	uint32_t backupNumAnimations = scene->mNumAnimations;
	aiAnimation** backupAnimations = scene->mAnimations;

	aiAnimation targetAnim = aiAnimation();
	aiAnimation* anims[1] = { &targetAnim };

	scene->mNumAnimations = 1;
	scene->mAnimations = anims;

	targetAnim.mName = srcAnim.name;
	targetAnim.mDuration = srcAnim.lengthInTicks;
	targetAnim.mTicksPerSecond = srcAnim.ticksPerSecond;

	std::vector<uint32_t> channelIds;

	for (size_t iChannel = 0; iChannel < srcAnim.channels.size(); iChannel++)
	{
		const Channel& srcChnl = srcAnim.channels[iChannel];
		//aiNodeAnim& targetChnl = channels[iChannel];

		auto iter = srcModel.boneTable.find(srcChnl.name);
		assert(iter != srcModel.boneTable.end());

		uint32_t srcBoneId = iter->second;

		uint32_t humanBoneId = srcModel.bones[srcBoneId].humanBoneId;
		if (humanBoneId == UINT32_MAX) continue;

		uint32_t dstBoneId = dstModel.humanBoneBindings[humanBoneId];
		if (dstBoneId == UINT32_MAX) continue;

		channelIds.push_back(iChannel);
	}

	std::vector<aiNodeAnim> channels(channelIds.size());
	std::vector<aiNodeAnim*> channelPtrs(channelIds.size());

	for (size_t i= 0; i < channelIds.size(); i++)
	{
		channelPtrs[i] = &(channels[i]);
	}

	targetAnim.mNumChannels = channelIds.size();
	targetAnim.mChannels = &(channelPtrs[0]);

	std::vector<size_t> transStartIndices(channelIds.size());
	std::vector<size_t> rotStartIndices(channelIds.size());
	std::vector<size_t> scaleStartIndices(channelIds.size());

	size_t totalTranslationKeys = 0;
	size_t totalRotatoinKeys = 0;
	size_t totalScalingKeys = 0;

	for (size_t i = 0; i < channelIds.size(); i++)
	{
		uint32_t chnlId = channelIds[i];

		transStartIndices[i] = totalTranslationKeys;
		totalTranslationKeys += srcAnim.channels[chnlId].translations.size();

		rotStartIndices[i] = totalRotatoinKeys;
		totalRotatoinKeys += srcAnim.channels[chnlId].rotations.size();

		scaleStartIndices[i] = totalScalingKeys;
		totalScalingKeys += srcAnim.channels[chnlId].scalings.size();
	}

	std::vector<aiVectorKey> translations(totalTranslationKeys);
	std::vector<aiQuatKey> rotations(totalRotatoinKeys);
	std::vector<aiVectorKey> scalings(totalScalingKeys);

	for (size_t iChannel = 0; iChannel < channelIds.size(); iChannel++)
	{
		const Channel& srcChnl = srcAnim.channels[channelIds[iChannel]];
		aiNodeAnim& targetChnl = channels[iChannel];

		targetChnl.mNodeName = srcChnl.name;

		auto iter = srcModel.boneTable.find(srcChnl.name);
		assert(iter != srcModel.boneTable.end());

		uint32_t srcBoneId = iter->second;

		uint32_t humanBoneId = srcModel.bones[srcBoneId].humanBoneId;
		if (humanBoneId == UINT32_MAX) continue;

		uint32_t dstBoneId = dstModel.humanBoneBindings[humanBoneId];
		if (dstBoneId == UINT32_MAX) continue;

		if (humanBoneId == HumanBone::Root || humanBoneId == HumanBone::Hips)
		{
			targetChnl.mNumPositionKeys = uint32_t(srcChnl.translations.size());
			targetChnl.mPositionKeys = &translations[transStartIndices[iChannel]];

			for (uint32_t k = 0; k < targetChnl.mNumPositionKeys; k++)
			{
				targetChnl.mPositionKeys[k].mTime = srcChnl.translations[k].time;
				const glm::vec3& v = srcChnl.translations[k].value;
				targetChnl.mPositionKeys[k].mValue = aiVector3D(v.x, v.y, v.z);
			}
		}
		else
		{
			targetChnl.mNumPositionKeys = 1;
			targetChnl.mPositionKeys = &translations[transStartIndices[iChannel]];
			targetChnl.mPositionKeys[0].mTime = 0;
			const glm::vec3 v = srcModel.humanBoneLocalT[humanBoneId];
			targetChnl.mPositionKeys[0].mValue = aiVector3D(v.x, v.y, v.z);
		}

		{
			targetChnl.mNumScalingKeys = 1;
			targetChnl.mScalingKeys = &scalings[scaleStartIndices[iChannel]];
			targetChnl.mScalingKeys[0].mTime = 0;
			targetChnl.mScalingKeys[0].mValue = aiVector3D(1.0f, 1.0f, 1.0f);
		}

		//srcModel.humanbone

		/*if (srcChnl.translations.size() > 0)
		{
			targetChnl.mNumPositionKeys = uint32_t(srcChnl.translations.size());
			targetChnl.mPositionKeys = &translations[transStartIndices[iChannel]];

			for (uint32_t k = 0; k < targetChnl.mNumPositionKeys; k++)
			{
				targetChnl.mPositionKeys[k].mTime = srcChnl.translations[k].time;
				const glm::vec3& v = srcChnl.translations[k].value;
				targetChnl.mPositionKeys[k].mValue = aiVector3D(v.x, v.y, v.z);
			}
		}

		if (srcChnl.scalings.size() > 0)
		{
			targetChnl.mNumScalingKeys = uint32_t(srcChnl.scalings.size());
			targetChnl.mScalingKeys = &scalings[scaleStartIndices[iChannel]];

			for (uint32_t k = 0; k < targetChnl.mNumScalingKeys; k++)
			{
				targetChnl.mScalingKeys[k].mTime = srcChnl.scalings[k].time;
				const glm::vec3& v = srcChnl.scalings[k].value;
				targetChnl.mScalingKeys[k].mValue = aiVector3D(v.x, v.y, v.z);
			}
		}*/

		if (srcChnl.rotations.size() > 0)
		{
			targetChnl.mNumRotationKeys = uint32_t(srcChnl.rotations.size());
			targetChnl.mRotationKeys = &rotations[rotStartIndices[iChannel]];

			for (uint32_t k = 0; k < targetChnl.mNumRotationKeys; k++)
			{
				targetChnl.mRotationKeys[k].mTime = srcChnl.rotations[k].time;
				glm::quat r = srcChnl.rotations[k].value;
								
				CorrectHumanBoneRotation(dstModel, srcModel, r, humanBoneId);

				targetChnl.mRotationKeys[k].mValue = aiQuaternion(r.w, r.x, r.y, r.z);
			}
		}
	}

	// maybe importer need it to release resources
	scene->mNumAnimations = backupNumAnimations;
	scene->mAnimations = backupAnimations;

	aiReturn ret = aiExportScene(scene, "x", filename, 0);


	return 0;
}
