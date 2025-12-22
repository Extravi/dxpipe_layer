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

// include our factory proxy
#include "ProxyFactory.h"

// IDXGIAdapter proxy wrapper
//  • logs GetParent -> factory
//  • forwards everything else unaltered
class ProxyAdapter : public IDXGIAdapter
{
public:
    explicit ProxyAdapter(IDXGIAdapter *real)
        : m_real(real), m_ref(1)
    {
        m_real->AddRef();
    }

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) override
    {
        if (riid == __uuidof(IDXGIAdapter) || riid == __uuidof(IUnknown))
        {
            *ppv = static_cast<IDXGIAdapter *>(this);
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
    HRESULT STDMETHODCALLTYPE GetParent(REFIID riid, void **ppParent) override
    {
        HRESULT hr = m_real->GetParent(riid, ppParent);
        if (SUCCEEDED(hr) && ppParent && *ppParent &&
            (riid == __uuidof(IDXGIFactory) ||
             riid == __uuidof(IDXGIFactory1) ||
             riid == __uuidof(IDXGIFactory2)))
        {
#if DEBUG
            output();
#endif
#if DEBUG
            std::cout << timeStamp() << "IDXGIAdapter::GetParent → wrapping factory" << std::endl;
#endif

            IDXGIFactory2 *f2 = nullptr;
            if (SUCCEEDED(((IUnknown *)*ppParent)->QueryInterface(__uuidof(IDXGIFactory2), (void **)&f2)))
            {
                *ppParent = new ProxyFactory(f2);
                f2->Release();
            }
        }
        return hr;
    }

    // IDXGIAdapter
    HRESULT STDMETHODCALLTYPE EnumOutputs(UINT o, IDXGIOutput **pp) override { return m_real->EnumOutputs(o, pp); }
    HRESULT STDMETHODCALLTYPE GetDesc(DXGI_ADAPTER_DESC *d) override { return m_real->GetDesc(d); }
    HRESULT STDMETHODCALLTYPE CheckInterfaceSupport(REFGUID i, LARGE_INTEGER *v) override { return m_real->CheckInterfaceSupport(i, v); }

private:
    IDXGIAdapter *m_real;
    std::atomic<uint32_t> m_ref;
};
