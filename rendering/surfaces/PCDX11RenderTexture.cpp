#include "../PCDX11DeviceManager.h"
#include "../PCDX11RenderDevice.h"
#include "PCDX11RenderTexture.h"
#include <d3d11.h>

namespace cdc {

PCDX11RenderTexture::PCDX11RenderTexture(
	uint16_t width, uint16_t height,
	uint32_t flags, uint32_t isDepthBuffer,
	PCDX11RenderDevice *renderDevice, uint32_t unknown2)
:
	PCDX11BaseTexture(width, height, 0),
	flags30(flags),
	shortWidth(width),
	shortHeight(height),
	isDepthBuffer(isDepthBuffer),
	renderDevice(renderDevice)
{

	// TODO
}

void PCDX11RenderTexture::ensureResource() {
	if (!resource) {
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = getWidth();
		desc.Height = getHeight();
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.Format = (DXGI_FORMAT) textureFormat; // from PCDX11BaseTexture
		desc.SampleDesc.Count = sampleCount;
		desc.SampleDesc.Quality = sampleQuality;
		desc.Usage = (D3D11_USAGE) 0;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;
		if (isDepthBuffer == false)
			desc.BindFlags |= D3D11_BIND_RENDER_TARGET | D3D11_BIND_UNORDERED_ACCESS;
		else if (isDepthBuffer == true)
			desc.BindFlags |= D3D11_BIND_DEPTH_STENCIL;

		auto *device = deviceManager->getD3DDevice();
		ID3D11Texture2D *texture;
		device->CreateTexture2D(&desc, nullptr, &texture);
		resource = texture;
		// TODO
	}
}

void PCDX11RenderTexture::ensureRenderTargetView() {
	// TODO
	auto *device = deviceManager->getD3DDevice();
	if (!view) {
		if (isDepthBuffer == false) {
			ID3D11RenderTargetView* frameBufferView;
			device->CreateRenderTargetView(resource, /*TODO*/ nullptr, &frameBufferView);
			view = frameBufferView;
		} else {
			ID3D11DepthStencilView* depthStencilView;
			device->CreateDepthStencilView(resource, /*TODO*/ nullptr, &depthStencilView);
			this->view = depthStencilView;
			this->depthStencilView = depthStencilView;
		}
	}

	if (!shaderResourceView)
		createShaderResourceView_internal(resource, &shaderResourceView);

	// TODO
}

void PCDX11RenderTexture::ensureBuffer() {
	// TODO
	if (!borrowedResource) {
		if (!resource) {
			ensureResource();
		}
	}
	// TODO
	if (!view || !shaderResourceView) {
		// TODO
		ensureRenderTargetView();
	}

	if (!registeredForDeletionAfterFrame && (flags30 & 4) == 0) {
		auto& num = renderDevice->numTemporarySurfaces;
		renderDevice->temporarySurfaces[num++] = originRenderSurface; 
		registeredForDeletionAfterFrame = true;
	}
}

ID3D11View *PCDX11RenderTexture::getView() {
	return view;
}

void PCDX11RenderTexture::initForRenderTarget(IRenderSurface *renderSurface, uint32_t format, ID3D11Texture2D *texture) {
	// TODO
	textureFormat = format;
	originRenderSurface = renderSurface;
	if (texture) {
		resource = texture;
		borrowedResource = true;
	}
}


void PCDX11RenderTexture::resFree() {
	// HACK
	if (!borrowedResource) {
		if (view)
			view->Release();
		if (depthStencilView)
			depthStencilView->Release();
		if (shaderResourceView)
			shaderResourceView->Release();
		resource->Release();
}

	if (borrowedResource) {
		if (view) {
			view->Release();
			view = nullptr;
		}

		if (shaderResourceView) {
			shaderResourceView->Release();
			shaderResourceView = nullptr;
		}

	} else {
		resource = nullptr;
	}

	if (unorderedAccessView) {
		unorderedAccessView->Release();
		unorderedAccessView = nullptr;
	}

	view = nullptr;
	shaderResourceView = nullptr;
	depthStencilView = nullptr;
	registeredForDeletionAfterFrame = false;
}

void PCDX11RenderTexture::resFill(void* src, size_t size, size_t offset) {
	// TODO
}

char *PCDX11RenderTexture::resGetBuffer() {
	// TODO
	return nullptr;
}

void PCDX11RenderTexture::resConstruct() {
	// TODO
}


uint32_t PCDX11RenderTexture::getWidth() {
	return width;
}

uint32_t PCDX11RenderTexture::getHeight() {
	return height;
}


ID3D11Resource *PCDX11RenderTexture::getTextureResource() {
	ensureBuffer();
	return resource;
}

ID3D11ShaderResourceView *PCDX11RenderTexture::createShaderResourceView() {
	ensureBuffer();
	return shaderResourceView;
}

ID3D11UnorderedAccessView *PCDX11RenderTexture::createUnorderedAccessView() {
	ensureBuffer();
	if (!unorderedAccessView) {
		// TODO
	}
	return unorderedAccessView;
}

ID3D11RenderTargetView *PCDX11RenderTexture::createRenderTargetView() {
	ensureBuffer();
	return static_cast<ID3D11RenderTargetView*>(view);
}

ID3D11DepthStencilView *PCDX11RenderTexture::createDepthStencilView() {
	ensureBuffer();
	return depthStencilView;
}

void PCDX11RenderTexture::createShaderResourceView_internal(
	ID3D11Resource *resource, ID3D11ShaderResourceView **pShaderResourceView)
{
	auto *device = deviceManager->getD3DDevice();
	D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = sampleCount > 1
		? D3D11_SRV_DIMENSION_TEXTURE2DMS
		: D3D11_SRV_DIMENSION_TEXTURE2D;
	desc.Format = (DXGI_FORMAT)PCDX11BaseTexture::textureFormat; // TODO
	desc.Texture2D.MipLevels = -1;

	device->CreateShaderResourceView(resource, &desc, pShaderResourceView);
}

}
