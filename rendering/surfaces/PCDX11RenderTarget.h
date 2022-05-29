#pragma once
#include "CommonRenderTarget.h"
#include "PCDX11RenderTexture.h"

namespace cdc {

class PCDX11RenderTarget : public CommonRenderTarget {
public:
	virtual uint32_t method_24(uint32_t, uint32_t) = 0;
	virtual uint32_t method_28() = 0;
	virtual uint32_t method_2C() = 0;
	virtual ID3D11Resource *getTextureResource() = 0;
	virtual ID3D11RenderTargetView *getRenderTargetView() = 0;
	virtual void copyFromTexture(void*) = 0;
	virtual bool method_3C() = 0;
	virtual void method_40() = 0;
};

}
