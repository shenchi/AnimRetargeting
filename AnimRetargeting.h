#pragma once
#include "Application.h"

#include "Model.h"

class AnimRetargeting : public Application
{
public:
	virtual int32_t OnResize() override;

	virtual int32_t OnInit() override;

	virtual int32_t OnRelease() override;

	virtual int32_t OnUpdate() override;

private:
	glm::mat4			matProj;
	glm::mat4			matView;
	glm::mat4			matModel;
	glm::vec3			cameraPos;
	float				padding[13];

	float				pitch, yaw;

	Model				model1;

	ID3D11Buffer*		frameConstants;
	ID3D11Buffer*		model1VB;
	ID3D11Buffer*		model1IB;
};
