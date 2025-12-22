#pragma once

// c++ includes
#include <iostream>
#include <iomanip>
#include <sstream>
#include <atomic>

// windows headers
#define NOMINMAX // disable min/max macros
#include <Windows.h>

// directx headers
#include <dxgi.h>
#include <dxgi1_2.h>

// helpers from d3d11.cpp
extern std::string timeStamp();
extern void output();

// forward declare adapter proxy (pointer only)
class ProxyAdapter;

// include after the forward-decl to avoid a cycle
#include "ProxyAdapter.h"

// IDXGIDevice proxy wrapper
//  • logs GetAdapter -> adapter
//  • forwards everything else unaltered
class ProxyDXGIDevice : public IDXGIDevice
{
public:
    explicit ProxyDXGIDevice(IDXGIDevice *real)
        : m_real(real), m_ref(1)
    {
        m_real->AddRef();
    }

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) override
    {
        if (riid == __uuidof(IDXGIDevice) || riid == __uuidof(IUnknown))
        {
            *ppv = static_cast<IDXGIDevice *>(this);
            AddRef();
            return S_OK;
        }
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

    // IDXGIObject
    HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID g, UINT s, const void *d) override { return m_real->SetPrivateData(g, s, d); }
    HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID g, const IUnknown *u) override { return m_real->SetPrivateDataInterface(g, u); }
    HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID g, UINT *s, void *d) override { return m_real->GetPrivateData(g, s, d); }
    HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void **ppParent) override { return m_real->GetParent(riid, ppParent); }

    // *** intercepted call ***
    HRESULT STDMETHODCALLTYPE GetAdapter(IDXGIAdapter **ppAdapter) override
    {
        HRESULT hr = m_real->GetAdapter(ppAdapter);
        if (SUCCEEDED(hr) && ppAdapter && *ppAdapter)
        {
#if DEBUG
            output();
#endif
#if DEBUG
            std::cout << timeStamp() << "IDXGIDevice::GetAdapter → wrapping adapter" << std::endl;
#endif

            IDXGIAdapter *realAdapter = *ppAdapter;
            *ppAdapter = new ProxyAdapter(realAdapter);
            realAdapter->Release();
        }
        return hr;
    }

    HRESULT STDMETHODCALLTYPE CreateSurface(const DXGI_SURFACE_DESC *d, UINT n,
                                            DXGI_USAGE u, const DXGI_SHARED_RESOURCE *s,
                                            IDXGISurface **pp) override
    {
        return m_real->CreateSurface(d, n, u, s, pp);
    }
    HRESULT STDMETHODCALLTYPE QueryResourceResidency(IUnknown *const *r, DXGI_RESIDENCY *rs, UINT n) override
    {
        return m_real->QueryResourceResidency(r, rs, n);
    }
    HRESULT STDMETHODCALLTYPE SetGPUThreadPriority(INT p) override { return m_real->SetGPUThreadPriority(p); }
    HRESULT STDMETHODCALLTYPE GetGPUThreadPriority(INT *p) override { return m_real->GetGPUThreadPriority(p); }

private:
    IDXGIDevice *m_real;
    std::atomic<uint32_t> m_ref;
};
