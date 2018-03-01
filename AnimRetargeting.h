#pragma once
#include "Application.h"

#include "Model.h"

class AnimRetargeting : public Application
{
private:
	struct ModelContext
	{
		std::string		name;
		std::vector<glm::mat4>	boneMatrices;
		Model*			model;
		ID3D11Buffer*	vb;
		ID3D11Buffer*	ib;
		ID3D11Buffer*	boneBuffer;
	};

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

	void GUI_Models();
	
	void GUI_Animations();

	void GUI_BoneTree(const Model& model, uint32_t boneId);

	void AddLine(const glm::vec3& from, const glm::vec3& to, const glm::vec3& color = glm::vec3(1.0f));

	glm::vec3 SampleVec3Sequence(const std::vector<Vec3Frame>& frames, float time, const glm::vec3 defaultValue = glm::vec3(0.0f));

	glm::quat SampleQuatSequence(const std::vector<QuatFrame>& frames, float time, const glm::quat defaultValue = glm::quat(1, 0, 0, 0));

	void UpdateBoneMatrices(const Model& model, std::vector<glm::mat4>& matrices, uint32_t animId, float time);

	void UpdateBoneMatrices(const Model& model, std::vector<glm::mat4>& matrices, const Model& animModel, uint32_t animId, float time);

	void UpdateHumanBoneTPose(const Model& model, std::vector<glm::mat4>& matrices);

	void DrawSkeletal(const Model& model);

	void DrawSkeletal(const Model & model, std::vector<glm::mat4>& matrices);

	void ResetCamera();

	int32_t OpenModel(const char* filename);

	int32_t CloseModel(const char* filename);

	int32_t ExportAnimation(const char* filename, const ModelContext& meshModel, const ModelContext& animModel, uint32_t animId);

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

	std::vector<ModelContext> openedModels;
	std::unordered_map<std::string, uint32_t> openedModelTable;

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

	std::vector<float>	gizmoBuffer;
	ID3D11Buffer*		gizmoVB;

	uint32_t			selectedModelIdx;
	uint32_t			selectedAnimModelIdx;
	uint32_t			selectedAnimIdx;

	float				modelScale = 0.01f;

	bool				isAnimPlaying = false;
	float				animPlaybackTime = 0.0f;
	float				animPlaybackSpeed = 1.0f;

	bool				showBones = false;
	bool				showHumanBonesOnly = false;
	bool				drawBoneAxis = false;
	float				boneAxisLength = 1.0f;
};
