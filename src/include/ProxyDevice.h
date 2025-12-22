#pragma once

// c++ includes
#include <iostream>
#include <iomanip>
#include <sstream>
#include <atomic>

// windows headers
#define NOMINMAX // disable min/max macros
#include <Windows.h>

// directx 11 headers
#include <d3d11.h>
#include <dxgi1_2.h>

// incluide our device context proxy wrapper
#include "ProxyDeviceContext.h"
#include "ProxyDXGIDevice.h" // lets us wrap IDXGIDevice

// forward decls for helpers implemented in d3d11.cpp
extern std::string timeStamp();
extern void output();
extern int g_Width;  // target width
extern int g_Height; // target height

// depth texture address
extern ID3D11Texture2D *g_DepthTexture;

// ID3D11Device proxy wrapper
// logs every CreateTexture2D call, forwards everything else unaltered
class ProxyDevice : public ID3D11Device
{
public:
    explicit ProxyDevice(ID3D11Device *real)
        : m_real(real), m_ref(1)
    {
        m_real->AddRef();
    }

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) override
    {
        if (riid == __uuidof(ID3D11Device) || riid == __uuidof(IUnknown))
        {
            *ppv = static_cast<ID3D11Device *>(this);
            AddRef();
            return S_OK;
        }

        // intercept and wrap IDXGIDevice so we can follow the adapter→factory chain
        if (riid == __uuidof(IDXGIDevice) ||
            riid == __uuidof(IDXGIDevice1) ||
            riid == __uuidof(IDXGIDevice2))
        {
            IDXGIDevice *realDX = nullptr;
            if (SUCCEEDED(m_real->QueryInterface(__uuidof(IDXGIDevice), (void **)&realDX)) && realDX)
            {
#if DEBUG
                output();
#endif
#if DEBUG
                std::cout << timeStamp()
                          << "ID3D11Device::QueryInterface → wrapping IDXGIDevice"
                          << std::endl;
#endif

                *ppv = static_cast<IDXGIDevice *>(new ProxyDXGIDevice(realDX));
                realDX->Release();
                return S_OK;
            }
        }

        // everything else
        return m_real->QueryInterface(riid, ppv);
    }

    ULONG STDMETHODCALLTYPE AddRef() override { return static_cast<ULONG>(++m_ref); }
    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG cnt = static_cast<ULONG>(--m_ref);
        if (!cnt)
        {
            m_real->Release();
            delete this;
        }
        return cnt;
    }

    // intercept D3D11Device::CreateTexture2D
    HRESULT STDMETHODCALLTYPE CreateTexture2D(
        const D3D11_TEXTURE2D_DESC *pDesc,
        const D3D11_SUBRESOURCE_DATA *pInitialData,
        ID3D11Texture2D **ppTexture2D) override
    {
#if DEBUG
        output();
#endif
        // log the call if the width, height, and format match
        if (pDesc && pDesc->Height == g_Height && pDesc->Width == g_Width &&
            pDesc->Format == DXGI_FORMAT_R32_TYPELESS)
        {
#if DEBUG
            std::cout << timeStamp() << "CreateTexture2D called!" << std::endl;
#endif
#if DEBUG
            std::cout << timeStamp() << "Width: " << pDesc->Width << std::endl;
#endif
#if DEBUG
            std::cout << timeStamp() << "Height: " << pDesc->Height << std::endl;
#endif
#if DEBUG
            std::cout << timeStamp() << "Format: " << pDesc->Format << std::endl;
#endif
#if DEBUG
            std::cout << timeStamp() << "MipLevels: " << pDesc->MipLevels << std::endl;
#endif
#if DEBUG
            std::cout << timeStamp() << "ArraySize: " << pDesc->ArraySize << std::endl;
#endif
#if DEBUG
            std::cout << timeStamp() << "Sample.Count: " << pDesc->SampleDesc.Count << std::endl;
#endif
#if DEBUG
            std::cout << timeStamp() << "Sample.Quality: " << pDesc->SampleDesc.Quality << std::endl;
#endif
#if DEBUG
            std::cout << timeStamp() << "Usage: " << pDesc->Usage << std::endl;
#endif
#if DEBUG
            std::cout << timeStamp() << "BindFlags: " << pDesc->BindFlags << std::endl;
#endif
#if DEBUG
            std::cout << timeStamp() << "CPUAccessFlags: " << pDesc->CPUAccessFlags << std::endl;
#endif
#if DEBUG
            std::cout << timeStamp() << "MiscFlags: " << pDesc->MiscFlags << std::endl;
#endif

            // calculate the size of the depth texture in bytes
            size_t bytesPerPixel = (pDesc->Format == DXGI_FORMAT_R16_TYPELESS ||
                                    pDesc->Format == DXGI_FORMAT_R16_FLOAT ||
                                    pDesc->Format == DXGI_FORMAT_D16_UNORM)
                                       ? 2
                                       : 4;

            size_t memorySize = static_cast<size_t>(pDesc->Width) * pDesc->Height *
                                pDesc->ArraySize * bytesPerPixel;
#if DEBUG
            std::cout << timeStamp() << "Memory Size: " << memorySize << " bytes" << std::endl;
#endif
        }

        // forward the call to the real device
        HRESULT hr = m_real->CreateTexture2D(pDesc, pInitialData, ppTexture2D);

        // intercept the call & log the address of the target texture
        if (SUCCEEDED(hr) && ppTexture2D && *ppTexture2D &&
            pDesc && pDesc->SampleDesc.Count == 1 &&
            pDesc->Width == g_Width &&
            pDesc->Height == g_Height &&
            pDesc->Format == DXGI_FORMAT_R32_TYPELESS)
        {
            // release previous depth surface (if any)
            if (g_DepthTexture)
                g_DepthTexture->Release();

            g_DepthTexture = *ppTexture2D;
            g_DepthTexture->AddRef(); // avoid premature release
#if DEBUG
            std::cout << timeStamp() << "Depth texture address: " << g_DepthTexture << std::endl;
#endif
        }

        return hr;
    }

    // -------- boiler-plate forwards (full signatures, named parameters) --------
    HRESULT STDMETHODCALLTYPE CreateBuffer(
        const D3D11_BUFFER_DESC *pDesc,
        const D3D11_SUBRESOURCE_DATA *pInitialData,
        ID3D11Buffer **ppBuffer) override
    {
        return m_real->CreateBuffer(pDesc, pInitialData, ppBuffer);
    }

    HRESULT STDMETHODCALLTYPE CreateTexture1D(
        const D3D11_TEXTURE1D_DESC *pDesc,
        const D3D11_SUBRESOURCE_DATA *pInitialData,
        ID3D11Texture1D **ppTexture1D) override
    {
        return m_real->CreateTexture1D(pDesc, pInitialData, ppTexture1D);
    }

    HRESULT STDMETHODCALLTYPE CreateTexture3D(
        const D3D11_TEXTURE3D_DESC *pDesc,
        const D3D11_SUBRESOURCE_DATA *pInitialData,
        ID3D11Texture3D **ppTexture3D) override
    {
        return m_real->CreateTexture3D(pDesc, pInitialData, ppTexture3D);
    }

    HRESULT STDMETHODCALLTYPE CreateShaderResourceView(
        ID3D11Resource *pResource,
        const D3D11_SHADER_RESOURCE_VIEW_DESC *pDesc,
        ID3D11ShaderResourceView **ppSRView) override
    {
        return m_real->CreateShaderResourceView(pResource, pDesc, ppSRView);
    }

    HRESULT STDMETHODCALLTYPE CreateUnorderedAccessView(
        ID3D11Resource *pResource,
        const D3D11_UNORDERED_ACCESS_VIEW_DESC *pDesc,
        ID3D11UnorderedAccessView **ppUAView) override
    {
        return m_real->CreateUnorderedAccessView(pResource, pDesc, ppUAView);
    }

    HRESULT STDMETHODCALLTYPE CreateRenderTargetView(
        ID3D11Resource *pResource,
        const D3D11_RENDER_TARGET_VIEW_DESC *pDesc,
        ID3D11RenderTargetView **ppRTView) override
    {
        return m_real->CreateRenderTargetView(pResource, pDesc, ppRTView);
    }

    HRESULT STDMETHODCALLTYPE CreateDepthStencilView(
        ID3D11Resource *pResource,
        const D3D11_DEPTH_STENCIL_VIEW_DESC *pDesc,
        ID3D11DepthStencilView **ppDSView) override
    {
        return m_real->CreateDepthStencilView(pResource, pDesc, ppDSView);
    }

    HRESULT STDMETHODCALLTYPE CreateInputLayout(
        const D3D11_INPUT_ELEMENT_DESC *pLayout,
        UINT NumElements,
        const void *pShaderBytecodeWithInputSignature,
        SIZE_T BytecodeLength,
        ID3D11InputLayout **ppInputLayout) override
    {
        return m_real->CreateInputLayout(pLayout, NumElements,
                                         pShaderBytecodeWithInputSignature,
                                         BytecodeLength, ppInputLayout);
    }

    HRESULT STDMETHODCALLTYPE CreateVertexShader(
        const void *pShaderBytecode,
        SIZE_T BytecodeLength,
        ID3D11ClassLinkage *pClassLinkage,
        ID3D11VertexShader **ppVertexShader) override
    {
        return m_real->CreateVertexShader(pShaderBytecode, BytecodeLength,
                                          pClassLinkage, ppVertexShader);
    }

    HRESULT STDMETHODCALLTYPE CreateGeometryShader(
        const void *pShaderBytecode,
        SIZE_T BytecodeLength,
        ID3D11ClassLinkage *pClassLinkage,
        ID3D11GeometryShader **ppGeometryShader) override
    {
        return m_real->CreateGeometryShader(pShaderBytecode, BytecodeLength,
                                            pClassLinkage, ppGeometryShader);
    }

    HRESULT STDMETHODCALLTYPE CreateGeometryShaderWithStreamOutput(
        const void *pShaderBytecode,
        SIZE_T BytecodeLength,
        const D3D11_SO_DECLARATION_ENTRY *pSODeclaration,
        UINT NumSOEntries,
        const UINT *pBufferStrides,
        UINT NumStrides,
        UINT RasterizedStream,
        ID3D11ClassLinkage *pClassLinkage,
        ID3D11GeometryShader **ppGeometryShader) override
    {
        return m_real->CreateGeometryShaderWithStreamOutput(pShaderBytecode, BytecodeLength,
                                                            pSODeclaration, NumSOEntries, pBufferStrides, NumStrides, RasterizedStream,
                                                            pClassLinkage, ppGeometryShader);
    }

    HRESULT STDMETHODCALLTYPE CreatePixelShader(
        const void *pShaderBytecode,
        SIZE_T BytecodeLength,
        ID3D11ClassLinkage *pClassLinkage,
        ID3D11PixelShader **ppPixelShader) override
    {
        return m_real->CreatePixelShader(pShaderBytecode, BytecodeLength,
                                         pClassLinkage, ppPixelShader);
    }

    HRESULT STDMETHODCALLTYPE CreateHullShader(
        const void *pShaderBytecode,
        SIZE_T BytecodeLength,
        ID3D11ClassLinkage *pClassLinkage,
        ID3D11HullShader **ppHullShader) override
    {
        return m_real->CreateHullShader(pShaderBytecode, BytecodeLength,
                                        pClassLinkage, ppHullShader);
    }

    HRESULT STDMETHODCALLTYPE CreateDomainShader(
        const void *pShaderBytecode,
        SIZE_T BytecodeLength,
        ID3D11ClassLinkage *pClassLinkage,
        ID3D11DomainShader **ppDomainShader) override
    {
        return m_real->CreateDomainShader(pShaderBytecode, BytecodeLength,
                                          pClassLinkage, ppDomainShader);
    }

    HRESULT STDMETHODCALLTYPE CreateComputeShader(
        const void *pShaderBytecode,
        SIZE_T BytecodeLength,
        ID3D11ClassLinkage *pClassLinkage,
        ID3D11ComputeShader **ppComputeShader) override
    {
        return m_real->CreateComputeShader(pShaderBytecode, BytecodeLength,
                                           pClassLinkage, ppComputeShader);
    }

    HRESULT STDMETHODCALLTYPE CreateClassLinkage(
        ID3D11ClassLinkage **ppLinkage) override
    {
        return m_real->CreateClassLinkage(ppLinkage);
    }

    HRESULT STDMETHODCALLTYPE CreateBlendState(
        const D3D11_BLEND_DESC *pDesc,
        ID3D11BlendState **ppBlendState) override
    {
        return m_real->CreateBlendState(pDesc, ppBlendState);
    }

    HRESULT STDMETHODCALLTYPE CreateDepthStencilState(
        const D3D11_DEPTH_STENCIL_DESC *pDesc,
        ID3D11DepthStencilState **ppDepthStencilState) override
    {
        return m_real->CreateDepthStencilState(pDesc, ppDepthStencilState);
    }

    HRESULT STDMETHODCALLTYPE CreateRasterizerState(
        const D3D11_RASTERIZER_DESC *pDesc,
        ID3D11RasterizerState **ppRasterizerState) override
    {
        return m_real->CreateRasterizerState(pDesc, ppRasterizerState);
    }

    HRESULT STDMETHODCALLTYPE CreateSamplerState(
        const D3D11_SAMPLER_DESC *pDesc,
        ID3D11SamplerState **ppSamplerState) override
    {
        return m_real->CreateSamplerState(pDesc, ppSamplerState);
    }

    HRESULT STDMETHODCALLTYPE CreateQuery(
        const D3D11_QUERY_DESC *pDesc,
        ID3D11Query **ppQuery) override
    {
        return m_real->CreateQuery(pDesc, ppQuery);
    }

    HRESULT STDMETHODCALLTYPE CreatePredicate(
        const D3D11_QUERY_DESC *pDesc,
        ID3D11Predicate **ppPredicate) override
    {
        return m_real->CreatePredicate(pDesc, ppPredicate);
    }

    HRESULT STDMETHODCALLTYPE CreateCounter(
        const D3D11_COUNTER_DESC *pDesc,
        ID3D11Counter **ppCounter) override
    {
        return m_real->CreateCounter(pDesc, ppCounter);
    }

    HRESULT STDMETHODCALLTYPE CreateDeferredContext(
        UINT ContextFlags,
        ID3D11DeviceContext **ppDeferredContext) override
    {
        return m_real->CreateDeferredContext(ContextFlags, ppDeferredContext);
    }

    HRESULT STDMETHODCALLTYPE OpenSharedResource(
        HANDLE hResource,
        REFIID ReturnedInterface,
        void **ppResource) override
    {
        return m_real->OpenSharedResource(hResource, ReturnedInterface, ppResource);
    }

    HRESULT STDMETHODCALLTYPE CheckFormatSupport(
        DXGI_FORMAT Format,
        UINT *pFormatSupport) override
    {
        return m_real->CheckFormatSupport(Format, pFormatSupport);
    }

    HRESULT STDMETHODCALLTYPE CheckMultisampleQualityLevels(
        DXGI_FORMAT Format,
        UINT SampleCount,
        UINT *pNumQualityLevels) override
    {
        return m_real->CheckMultisampleQualityLevels(Format, SampleCount, pNumQualityLevels);
    }

    void STDMETHODCALLTYPE CheckCounterInfo(
        D3D11_COUNTER_INFO *pCounterInfo) override
    {
        m_real->CheckCounterInfo(pCounterInfo);
    }

    HRESULT STDMETHODCALLTYPE CheckCounter(
        const D3D11_COUNTER_DESC *pDesc,
        D3D11_COUNTER_TYPE *pType,
        UINT *pActiveCounters,
        LPSTR szName,
        UINT *pNameLength,
        LPSTR szUnits,
        UINT *pUnitsLength,
        LPSTR szDescription,
        UINT *pDescriptionLength) override
    {
        return m_real->CheckCounter(pDesc, pType, pActiveCounters,
                                    szName, pNameLength, szUnits, pUnitsLength,
                                    szDescription, pDescriptionLength);
    }

    HRESULT STDMETHODCALLTYPE CheckFeatureSupport(
        D3D11_FEATURE Feature,
        void *pFeatureSupportData,
        UINT FeatureSupportDataSize) override
    {
        return m_real->CheckFeatureSupport(Feature, pFeatureSupportData, FeatureSupportDataSize);
    }

    HRESULT STDMETHODCALLTYPE GetPrivateData(
        REFGUID guid,
        UINT *pDataSize,
        void *pData) override
    {
        return m_real->GetPrivateData(guid, pDataSize, pData);
    }

    HRESULT STDMETHODCALLTYPE SetPrivateData(
        REFGUID guid,
        UINT DataSize,
        const void *pData) override
    {
        return m_real->SetPrivateData(guid, DataSize, pData);
    }

    HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(
        REFGUID guid,
        const IUnknown *pData) override
    {
        return m_real->SetPrivateDataInterface(guid, pData);
    }

    D3D_FEATURE_LEVEL STDMETHODCALLTYPE GetFeatureLevel() override
    {
        return m_real->GetFeatureLevel();
    }

    UINT STDMETHODCALLTYPE GetCreationFlags() override
    {
        return m_real->GetCreationFlags();
    }

    HRESULT STDMETHODCALLTYPE GetDeviceRemovedReason() override
    {
        return m_real->GetDeviceRemovedReason();
    }

    // wrap immediate context with our proxy
    void STDMETHODCALLTYPE GetImmediateContext(
        ID3D11DeviceContext **ppImmediateContext) override
    {
        ID3D11DeviceContext *realContext = nullptr;
        m_real->GetImmediateContext(&realContext);
        if (realContext)
        {
            *ppImmediateContext = new ProxyDeviceContext(realContext); // return proxy
            realContext->Release();                                    // balance reference count
        }
        else
        {
            *ppImmediateContext = nullptr;
        }
    }

    HRESULT STDMETHODCALLTYPE SetExceptionMode(
        UINT RaiseFlags) override
    {
        return m_real->SetExceptionMode(RaiseFlags);
    }

    UINT STDMETHODCALLTYPE GetExceptionMode() override
    {
        return m_real->GetExceptionMode();
    }

private:
    ID3D11Device *m_real;
    std::atomic<uint32_t> m_ref;
};
