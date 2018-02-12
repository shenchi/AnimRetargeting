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

	virtual void OnMouseDown(int button) override;

	virtual void OnMouseUp(int button) override;

	virtual void OnMouseMove(int x, int y) override;

	virtual void OnMouseWheel(int delta) override;
	
private:
	void GUI();

	void GUI_BoneTree(uint32_t boneId);

	void AddLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color = glm::vec3(1.0f));

	glm::vec3 SampleVec3Sequence(const std::vector<Vec3Frame>& frames, float time);

	glm::quat SampleQuatSequence(const std::vector<QuatFrame>& frames, float time);

	void UpdateBoneMatrices(const Model& model, std::vector<glm::mat4>& matrices, uint32_t animId, float time);

	void UpdateHumanBoneTPose(const Model& model, std::vector<glm::mat4>& matrices);

	void DrawSkeletal(const Model& model);

	void DrawSkeletal(const Model & model, std::vector<glm::mat4>& matrices);

private:
	glm::mat4			matProj;
	glm::mat4			matView;
	glm::mat4			matModel;
	glm::vec3			cameraPos;
	float				padding[13];

	int					lastX, lastY;
	int					lbuttonDown, rbuttonDown;
	float				pitch, yaw, dist;
	glm::vec3			focusPoint;

	Model				model1;

	void*				vsSrc;
	size_t				vsSrcSize;
	void*				psSrc;
	size_t				psSrcSize;
	ID3D11VertexShader*	vs;
	ID3D11PixelShader*	ps;
	ID3D11InputLayout*	layout;

	void*				gizmoVsSrc;
	size_t				gizmoVsSrcSize;
	void*				gizmoPsSrc;
	size_t				gizmoPsSrcSize;
	ID3D11VertexShader*	gizmoVs;
	ID3D11PixelShader*	gizmoPs;
	ID3D11InputLayout*	gizmoLayout;

	ID3D11DepthStencilState* gizmoDSS;

	ID3D11Buffer*		frameConstants;

	ID3D11Buffer*		model1VB;
	ID3D11Buffer*		model1IB;

	std::vector<glm::mat4>	model1BoneMatrices;
	ID3D11Buffer*		model1BoneBuffer;

	std::vector<float>	gizmoBuffer;
	ID3D11Buffer*		gizmoVB;

	uint32_t			animId;
	float				animPlayTime;

	bool				showHumanBonesOnly = false;
	bool				drawBoneAxis = false;
	float				boneAxisLength = 1.0f;
};
