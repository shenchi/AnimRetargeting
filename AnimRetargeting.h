#pragma once
#include "Application.h"

class AnimRetargeting : public Application
{
public:
	virtual int32_t OnResize() override;

	virtual int32_t OnInit() override;

	virtual int32_t OnRelease() override;
};
