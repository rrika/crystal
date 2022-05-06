#pragma once
#include "IRenderDrawable.h"

namespace cdc {

class PCDX11RenderTarget;
class PCDX11DepthBuffer;

class PCDX11SetRTDrawable :
	public IRenderDrawable
{
public:
	PCDX11RenderTarget *renderTarget; // 0C
	PCDX11DepthBuffer *depthBuffer; // 10

	PCDX11SetRTDrawable(PCDX11RenderTarget *renderTarget, PCDX11DepthBuffer *depthBuffer)
		: renderTarget(renderTarget)
		, depthBuffer(depthBuffer)
	{}

	virtual void renderDrawable0();
	virtual uint32_t renderDrawable4();
};

}