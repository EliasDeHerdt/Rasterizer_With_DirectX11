/*=============================================================================*/
// Copyright 2017-2019 Elite Engine
// Authors: Matthieu Delaere
/*=============================================================================*/
// ERenderer.h: class that holds the surface to render too + DirectX initialization.
/*=============================================================================*/
#ifndef ELITE_RAYTRACING_RENDERER
#define	ELITE_RAYTRACING_RENDERER

#include <cstdint>
#include "Mesh.h"
#include "Structs.h"

struct SDL_Window;
struct SDL_Surface;

namespace Elite
{
	class Renderer final
	{
	public:
		Renderer(SDL_Window* pWindow);
		~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) noexcept = delete;
		Renderer& operator=(const Renderer&) = delete;
		Renderer& operator=(Renderer&&) noexcept = delete;

		void Render();
		ID3D11Device* GetDevice();
		void ToggleDepthRendering();
		void PrintDepthRenderingInformation();
		void ToggleEffectRendering();
		void PrintEffectRenderingInformation();

	private:
		SDL_Window* m_pWindow;
		uint32_t m_Width;
		uint32_t m_Height;

		//My Variables
		bool m_RenderEffects = true;

		//DirectX
		ID3D11Device* m_pDevice;
		ID3D11DeviceContext* m_pDeviceContext;
		
		IDXGIFactory* m_pDXGIFactory;
		IDXGISwapChain* m_pSwapChain;
		
		ID3D11Texture2D* m_pDepthStencilBuffer;
		ID3D11DepthStencilView* m_pDepthStencilView;

		ID3D11Texture2D* m_pRenderTargetBuffer;
		ID3D11RenderTargetView* m_pRenderTargetView;

		bool m_IsInitialized;

		//Rasterizer
		SDL_Surface* m_pFrontBuffer = nullptr;
		SDL_Surface* m_pBackBuffer = nullptr;
		uint32_t* m_pBackBufferPixels = nullptr;

		std::vector<float> m_DepthBuffer;
		bool m_DepthRendering = false;
		
		//My Functions
		//DirectX
		long InitializeDirectX();

		//Rasterizer
		bool PixelInTri(float col, float row, const Elite::FPoint4& v0, const Elite::FPoint4& v1, const Elite::FPoint4& v2, float& weight0, float& weight1, float& weight2, BaseEffect::Culling cullMode) const;
		void CalculateDepthBuffer(float& depth, float w0, float w1, float w2, int i0, int i1, int i2, const std::vector<OutputVertex>& vertices) const;
		void CalculateDepthInterpolated(float& depth, float w0, float w1, float w2, int i0, int i1, int i2, const std::vector<OutputVertex>& vertices) const;
		void CalculateUV(Elite::FVector2& finalUV, float depth, float w0, float w1, float w2, int i0, int i1, int i2, const std::vector<OutputVertex>& vertices) const;
		void CalculateNormal(Elite::FVector3& finalNormal, float depth, float w0, float w1, float w2, int i0, int i1, int i2, const std::vector<OutputVertex>& vertices) const;
		void CalculateTangent(Elite::FVector3& finalTangent, float depth, float w0, float w1, float w2, int i0, int i1, int i2, const std::vector<OutputVertex>& vertices) const;
		void CalculateViewDirection(Elite::FVector3& finalViewDirection, float depth, float w0, float w1, float w2, int i0, int i1, int i2, const std::vector<OutputVertex>& vertices) const;
		void CalculateColor(Elite::RGBColor& finalColor, float depth, float w0, float w1, float w2, int i0, int i1, int i2, const std::vector<OutputVertex>& vertices) const;
	};
}

#endif