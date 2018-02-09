#include "AnimRetargeting.h"

#include <glm/gtc/quaternion.hpp>
using namespace glm;



int32_t AnimRetargeting::OnResize()
{
	int err = Application::OnResize();
	if (err) return err;

	matProj = perspectiveFov(half_pi<float>(), (float)bufferWidth, (float)bufferHeight, 0.1f, 10.0f);

	return 0;
}

int32_t AnimRetargeting::OnInit()
{
	model1.Load("assets/akai_e_espiritu.fbx");

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

	/*{
		CD3D11_BUFFER_DESC desc(
			model1.positions.size() * sizeof(float) * ,
			D3D11_BIND_CONSTANT_BUFFER,
			D3D11_USAGE_DYNAMIC,
			D3D11_CPU_ACCESS_WRITE);
		if (FAILED(device->CreateBuffer(&desc, nullptr, &frameConstants)))
		{
			return __LINE__;
		}
	}*/

	matModel = mat4(1.0f);

	cameraPos = vec3(0, 0, -1);
	pitch = 0.0f;
	yaw = 0.0f;

	return 0;
}

int32_t AnimRetargeting::OnRelease()
{
	return 0;
}

int32_t AnimRetargeting::OnUpdate()
{
	vec3 cameraDir = quat(vec3(pitch, yaw, 0.0f)) * vec3(0, 0, 1);
	matView = lookAt(cameraPos, cameraPos + cameraDir, vec3(0, 1, 0));

	D3D11_MAPPED_SUBRESOURCE res = {};
	context->Map(frameConstants, 0, D3D11_MAP_WRITE_DISCARD, 0, &res);
	memcpy(res.pData, &matProj, sizeof(mat4) * 4);
	context->Unmap(frameConstants, 0);

	float clearColor[] = { 0.7f, 0.7f, 0.7f, 1.0f };
	context->ClearRenderTargetView(rtv, clearColor);
	context->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH, 1.0f, 0);



	swapChain->Present(0, 0);
	return 0;
}
