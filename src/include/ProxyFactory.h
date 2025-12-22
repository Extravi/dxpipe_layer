#pragma once

// c++ includes
#include <iostream>
#include <iomanip>
#include <sstream>
#include <atomic>

// windows headers
#define NOMINMAX
#include <Windows.h>

// directx headers
#include <dxgi1_3.h> // IDXGIFactory2 / SwapChain2

// helper decls
extern std::string timeStamp();
extern void output();

// include our swap-chain proxy
#include "ProxySwapChain.h"

// IDXGIFactory2 proxy wrapper
//  • logs every CreateSwapChain*
//  • forwards everything else unaltered
class ProxyFactory : public IDXGIFactory2
{
public:
    explicit ProxyFactory(IDXGIFactory2 *real)
        : m_real(real), m_ref(1)
    {
        m_real->AddRef();
    }

    // -------- IUnknown --------
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) override
    {
        if (riid == __uuidof(IDXGIFactory) ||
            riid == __uuidof(IDXGIFactory1) ||
            riid == __uuidof(IDXGIFactory2) ||
            riid == __uuidof(IUnknown))
        {
            *ppv = static_cast<IDXGIFactory2 *>(this);
            AddRef();
            return S_OK;
        }
        return m_real->QueryInterface(riid, ppv);
    }
    ULONG STDMETHODCALLTYPE AddRef() override { return static_cast<ULONG>(++m_ref); }
    ULONG STDMETHODCALLTYPE Release() override
    {
        ULONG c = static_cast<ULONG>(--m_ref);
        if (!c)
        {
            m_real->Release();
            delete this;
        }
        return c;
    }

    // -------- IDXGIObject --------
    HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID g, UINT s, const void *d) override { return m_real->SetPrivateData(g, s, d); }
    HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID g, const IUnknown *u) override { return m_real->SetPrivateDataInterface(g, u); }
    HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID g, UINT *s, void *d) override { return m_real->GetPrivateData(g, s, d); }
    HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void **pp) override { return m_real->GetParent(riid, pp); }

    // -------- IDXGIFactory --------
    HRESULT STDMETHODCALLTYPE EnumAdapters(UINT i, IDXGIAdapter **pp) override { return m_real->EnumAdapters(i, pp); }
    HRESULT STDMETHODCALLTYPE MakeWindowAssociation(HWND h, UINT f) override { return m_real->MakeWindowAssociation(h, f); }
    HRESULT STDMETHODCALLTYPE GetWindowAssociation(HWND *h) override { return m_real->GetWindowAssociation(h); }
    HRESULT STDMETHODCALLTYPE CreateSwapChain(IUnknown *pDev,
                                              DXGI_SWAP_CHAIN_DESC *d,
                                              IDXGISwapChain **ppSwapChain) override
    {
#if DEBUG
        output();
#endif
#if DEBUG
        std::cout << timeStamp() << "IDXGIFactory::CreateSwapChain" << std::endl;
#endif

        HRESULT hr = m_real->CreateSwapChain(pDev, d, ppSwapChain);
        if (SUCCEEDED(hr) && ppSwapChain && *ppSwapChain)
        {
            *ppSwapChain = new ProxySwapChain(reinterpret_cast<IDXGISwapChain2 *>(*ppSwapChain));
        }
        return hr;
    }
    HRESULT STDMETHODCALLTYPE CreateSoftwareAdapter(HMODULE m, IDXGIAdapter **pp) override
    {
        return m_real->CreateSoftwareAdapter(m, pp);
    }

    // -------- IDXGIFactory1 additions --------
    HRESULT STDMETHODCALLTYPE EnumAdapters1(UINT i, IDXGIAdapter1 **pp) override { return m_real->EnumAdapters1(i, pp); }
    BOOL STDMETHODCALLTYPE IsCurrent() override { return m_real->IsCurrent(); }

    // -------- IDXGIFactory2 additions --------
    BOOL STDMETHODCALLTYPE IsWindowedStereoEnabled() override { return m_real->IsWindowedStereoEnabled(); }

    HRESULT STDMETHODCALLTYPE CreateSwapChainForHwnd(IUnknown *pDev, HWND hwnd,
                                                     const DXGI_SWAP_CHAIN_DESC1 *d,
                                                     const DXGI_SWAP_CHAIN_FULLSCREEN_DESC *fd,
                                                     IDXGIOutput *out,
                                                     IDXGISwapChain1 **ppSwapChain) override
    {
#if DEBUG
        output();
#endif
#if DEBUG
        std::cout << timeStamp() << "IDXGIFactory2::CreateSwapChainForHwnd" << std::endl;
#endif

        HRESULT hr = m_real->CreateSwapChainForHwnd(pDev, hwnd, d, fd, out, ppSwapChain);
        if (SUCCEEDED(hr) && ppSwapChain && *ppSwapChain)
        {
            *ppSwapChain = new ProxySwapChain(reinterpret_cast<IDXGISwapChain2 *>(*ppSwapChain));
        }
        return hr;
    }

    HRESULT STDMETHODCALLTYPE CreateSwapChainForCoreWindow(IUnknown *pDev, IUnknown *pWin,
                                                           const DXGI_SWAP_CHAIN_DESC1 *d,
                                                           IDXGIOutput *out,
                                                           IDXGISwapChain1 **ppSwapChain) override
    {
#if DEBUG
        output();
#endif
#if DEBUG
        std::cout << timeStamp() << "IDXGIFactory2::CreateSwapChainForCoreWindow" << std::endl;
#endif

        HRESULT hr = m_real->CreateSwapChainForCoreWindow(pDev, pWin, d, out, ppSwapChain);
        if (SUCCEEDED(hr) && ppSwapChain && *ppSwapChain)
        {
            *ppSwapChain = new ProxySwapChain(reinterpret_cast<IDXGISwapChain2 *>(*ppSwapChain));
        }
        return hr;
    }

    HRESULT STDMETHODCALLTYPE CreateSwapChainForComposition(IUnknown *pDev,
                                                            const DXGI_SWAP_CHAIN_DESC1 *d,
                                                            IDXGIOutput *out,
                                                            IDXGISwapChain1 **ppSwapChain) override
    {
#if DEBUG
        output();
#endif
#if DEBUG
        std::cout << timeStamp() << "IDXGIFactory2::CreateSwapChainForComposition" << std::endl;
#endif

        HRESULT hr = m_real->CreateSwapChainForComposition(pDev, d, out, ppSwapChain);
        if (SUCCEEDED(hr) && ppSwapChain && *ppSwapChain)
        {
            *ppSwapChain = new ProxySwapChain(reinterpret_cast<IDXGISwapChain2 *>(*ppSwapChain));
        }
        return hr;
    }

    HRESULT STDMETHODCALLTYPE GetSharedResourceAdapterLuid(HANDLE h, LUID *l) override
    {
        return m_real->GetSharedResourceAdapterLuid(h, l);
    }
    HRESULT STDMETHODCALLTYPE RegisterStereoStatusWindow(HWND h, UINT m, DWORD *c) override
    {
        return m_real->RegisterStereoStatusWindow(h, m, c);
    }
    HRESULT STDMETHODCALLTYPE RegisterStereoStatusEvent(HANDLE e, DWORD *c) override
    {
        return m_real->RegisterStereoStatusEvent(e, c);
    }
    void STDMETHODCALLTYPE UnregisterStereoStatus(DWORD c) override
    {
        m_real->UnregisterStereoStatus(c);
    }
    HRESULT STDMETHODCALLTYPE RegisterOcclusionStatusWindow(HWND h, UINT m, DWORD *c) override
    {
        return m_real->RegisterOcclusionStatusWindow(h, m, c);
    }
    HRESULT STDMETHODCALLTYPE RegisterOcclusionStatusEvent(HANDLE e, DWORD *c) override
    {
        return m_real->RegisterOcclusionStatusEvent(e, c);
    }
    void STDMETHODCALLTYPE UnregisterOcclusionStatus(DWORD c) override
    {
        m_real->UnregisterOcclusionStatus(c);
    }

private:
    IDXGIFactory2 *m_real;
    std::atomic<uint32_t> m_ref;
};
