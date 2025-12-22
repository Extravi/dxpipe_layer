#pragma once

// c++ includes
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <atomic>

// windows headers
#define NOMINMAX // disable min/max macros
#include <Windows.h>

// directx 11 headers
#include <d3d11.h>
#include <dxgi1_2.h>

// forward decls for helpers implemented in d3d11.cpp
extern std::string timeStamp();
extern void output();
extern int g_Width;  // target width
extern int g_Height; // target height

// depth texture address
extern ID3D11Texture2D *g_DepthTexture;

// ID3D11DeviceContext proxy wrapper
// intercepts DrawIndexed (called every frame), forwards everything else unaltered
class ProxyDeviceContext : public ID3D11DeviceContext
{
public:
    explicit ProxyDeviceContext(ID3D11DeviceContext *real)
        : m_real(real), m_ref(1)
    {
        m_real->AddRef();
    }

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppv) override
    {
        if (riid == __uuidof(ID3D11DeviceContext) || riid == __uuidof(IUnknown))
        {
            *ppv = static_cast<ID3D11DeviceContext *>(this);
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

    // -------- ID3D11DeviceChild --------
    void STDMETHODCALLTYPE GetDevice(ID3D11Device **ppDevice) override { m_real->GetDevice(ppDevice); }
    HRESULT STDMETHODCALLTYPE GetPrivateData(REFGUID guid, UINT *pDataSize, void *pData) override { return m_real->GetPrivateData(guid, pDataSize, pData); }
    HRESULT STDMETHODCALLTYPE SetPrivateData(REFGUID guid, UINT DataSize, const void *pData) override { return m_real->SetPrivateData(guid, DataSize, pData); }
    HRESULT STDMETHODCALLTYPE SetPrivateDataInterface(REFGUID guid, const IUnknown *pData) override { return m_real->SetPrivateDataInterface(guid, pData); }

    // -------- our single intercepted call --------
    void STDMETHODCALLTYPE DrawIndexed(UINT IndexCount, UINT StartIndexLocation, INT BaseVertexLocation) override
    {
#if DEBUG
        output();
#endif
#if DEBUG
// std::cout << timeStamp() << "DrawIndexed called!" << std::endl;
#endif
#if DEBUG
// std::cout << "IndexCount: " << IndexCount << std::endl;
#endif
#if DEBUG
// std::cout << "StartIndexLocation: " << StartIndexLocation << std::endl;
#endif
#if DEBUG
// std::cout << "BaseVertexLocation: " << BaseVertexLocation << std::endl;
#endif
        m_real->DrawIndexed(IndexCount, StartIndexLocation, BaseVertexLocation);
    }

    // -------- block of pure-virtuals, all forwarded --------
    // *** stage setters ***
    void VSSetConstantBuffers(UINT s, UINT n, ID3D11Buffer *const *b) override { m_real->VSSetConstantBuffers(s, n, b); }
    void PSSetShaderResources(UINT s, UINT n, ID3D11ShaderResourceView *const *v) override { m_real->PSSetShaderResources(s, n, v); }
    void PSSetShader(ID3D11PixelShader *sh, ID3D11ClassInstance *const *ci, UINT nci) override { m_real->PSSetShader(sh, ci, nci); }
    void PSSetSamplers(UINT s, UINT n, ID3D11SamplerState *const *ss) override { m_real->PSSetSamplers(s, n, ss); }
    void VSSetShader(ID3D11VertexShader *sh, ID3D11ClassInstance *const *ci, UINT nci) override { m_real->VSSetShader(sh, ci, nci); }
    void Draw(UINT vc, UINT sv) override { m_real->Draw(vc, sv); }
    HRESULT Map(ID3D11Resource *r, UINT sub, D3D11_MAP t, UINT f, D3D11_MAPPED_SUBRESOURCE *m) override { return m_real->Map(r, sub, t, f, m); }
    void Unmap(ID3D11Resource *r, UINT sub) override { m_real->Unmap(r, sub); }
    void PSSetConstantBuffers(UINT s, UINT n, ID3D11Buffer *const *b) override { m_real->PSSetConstantBuffers(s, n, b); }
    void IASetInputLayout(ID3D11InputLayout *l) override { m_real->IASetInputLayout(l); }
    void IASetVertexBuffers(UINT s, UINT n, ID3D11Buffer *const *v, const UINT *st, const UINT *o) override { m_real->IASetVertexBuffers(s, n, v, st, o); }
    void IASetIndexBuffer(ID3D11Buffer *ib, DXGI_FORMAT f, UINT o) override { m_real->IASetIndexBuffer(ib, f, o); }
    void DrawIndexedInstanced(UINT ic, UINT i, UINT si, INT bv, UINT si2) override { m_real->DrawIndexedInstanced(ic, i, si, bv, si2); }
    void DrawInstanced(UINT vc, UINT ic, UINT sv, UINT si) override { m_real->DrawInstanced(vc, ic, sv, si); }
    void GSSetConstantBuffers(UINT s, UINT n, ID3D11Buffer *const *b) override { m_real->GSSetConstantBuffers(s, n, b); }
    void GSSetShader(ID3D11GeometryShader *sh, ID3D11ClassInstance *const *ci, UINT nci) override { m_real->GSSetShader(sh, ci, nci); }
    void IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY t) override { m_real->IASetPrimitiveTopology(t); }
    void VSSetShaderResources(UINT s, UINT n, ID3D11ShaderResourceView *const *v) override { m_real->VSSetShaderResources(s, n, v); }
    void VSSetSamplers(UINT s, UINT n, ID3D11SamplerState *const *ss) override { m_real->VSSetSamplers(s, n, ss); }
    void Begin(ID3D11Asynchronous *a) override { m_real->Begin(a); }
    void End(ID3D11Asynchronous *a) override { m_real->End(a); }
    HRESULT GetData(ID3D11Asynchronous *a, void *d, UINT sz, UINT fl) override { return m_real->GetData(a, d, sz, fl); }
    void SetPredication(ID3D11Predicate *p, BOOL v) override { m_real->SetPredication(p, v); }
    void GSSetShaderResources(UINT s, UINT n, ID3D11ShaderResourceView *const *v) override { m_real->GSSetShaderResources(s, n, v); }
    void GSSetSamplers(UINT s, UINT n, ID3D11SamplerState *const *ss) override { m_real->GSSetSamplers(s, n, ss); }
    void OMSetRenderTargets(UINT n, ID3D11RenderTargetView *const *rt, ID3D11DepthStencilView *dsv) override { m_real->OMSetRenderTargets(n, rt, dsv); }
    void OMSetRenderTargetsAndUnorderedAccessViews(UINT nrt, ID3D11RenderTargetView *const *rt, ID3D11DepthStencilView *dsv, UINT uavStart, UINT nuav, ID3D11UnorderedAccessView *const *uav, const UINT *init) override { m_real->OMSetRenderTargetsAndUnorderedAccessViews(nrt, rt, dsv, uavStart, nuav, uav, init); }
    void OMSetBlendState(ID3D11BlendState *bs, const FLOAT bf[4], UINT sm) override { m_real->OMSetBlendState(bs, bf, sm); }
    void OMSetDepthStencilState(ID3D11DepthStencilState *ds, UINT sr) override { m_real->OMSetDepthStencilState(ds, sr); }
    void SOSetTargets(UINT n, ID3D11Buffer *const *t, const UINT *o) override { m_real->SOSetTargets(n, t, o); }
    void DrawAuto() override { m_real->DrawAuto(); }
    void DrawIndexedInstancedIndirect(ID3D11Buffer *b, UINT off) override { m_real->DrawIndexedInstancedIndirect(b, off); }
    void DrawInstancedIndirect(ID3D11Buffer *b, UINT off) override { m_real->DrawInstancedIndirect(b, off); }
    void Dispatch(UINT X, UINT Y, UINT Z) override { m_real->Dispatch(X, Y, Z); }
    void DispatchIndirect(ID3D11Buffer *b, UINT off) override { m_real->DispatchIndirect(b, off); }
    void RSSetState(ID3D11RasterizerState *rs) override { m_real->RSSetState(rs); }
    void RSSetViewports(UINT n, const D3D11_VIEWPORT *vp) override { m_real->RSSetViewports(n, vp); }
    void RSSetScissorRects(UINT n, const D3D11_RECT *rc) override { m_real->RSSetScissorRects(n, rc); }
    void CopySubresourceRegion(ID3D11Resource *dst, UINT dsub, UINT dx, UINT dy, UINT dz, ID3D11Resource *src, UINT ssub, const D3D11_BOX *box) override { m_real->CopySubresourceRegion(dst, dsub, dx, dy, dz, src, ssub, box); }
    void CopyResource(ID3D11Resource *dst, ID3D11Resource *src) override { m_real->CopyResource(dst, src); }
    void UpdateSubresource(ID3D11Resource *dst, UINT dsub, const D3D11_BOX *box, const void *src, UINT rp, UINT dp) override { m_real->UpdateSubresource(dst, dsub, box, src, rp, dp); }
    void CopyStructureCount(ID3D11Buffer *dst, UINT off, ID3D11UnorderedAccessView *src) override { m_real->CopyStructureCount(dst, off, src); }
    void ClearRenderTargetView(ID3D11RenderTargetView *rt, const FLOAT c[4]) override { m_real->ClearRenderTargetView(rt, c); }
    void ClearUnorderedAccessViewUint(ID3D11UnorderedAccessView *uav, const UINT v[4]) override { m_real->ClearUnorderedAccessViewUint(uav, v); }
    void ClearUnorderedAccessViewFloat(ID3D11UnorderedAccessView *uav, const FLOAT v[4]) override { m_real->ClearUnorderedAccessViewFloat(uav, v); }
    void ClearDepthStencilView(ID3D11DepthStencilView *dsv, UINT f, FLOAT d, UINT8 s) override { m_real->ClearDepthStencilView(dsv, f, d, s); }
    void GenerateMips(ID3D11ShaderResourceView *srv) override { m_real->GenerateMips(srv); }
    void SetResourceMinLOD(ID3D11Resource *r, FLOAT l) override { m_real->SetResourceMinLOD(r, l); }
    FLOAT GetResourceMinLOD(ID3D11Resource *r) override { return m_real->GetResourceMinLOD(r); }
    void ResolveSubresource(ID3D11Resource *dst, UINT dsub, ID3D11Resource *src, UINT ssub, DXGI_FORMAT f) override { m_real->ResolveSubresource(dst, dsub, src, ssub, f); }
    void ExecuteCommandList(ID3D11CommandList *cl, BOOL rst) override { m_real->ExecuteCommandList(cl, rst); }

    // Hull-, domain-, compute-stage setters we missed last time
    void HSSetShaderResources(UINT s, UINT n, ID3D11ShaderResourceView *const *v) override { m_real->HSSetShaderResources(s, n, v); }
    void HSSetShader(ID3D11HullShader *sh, ID3D11ClassInstance *const *ci, UINT nci) override { m_real->HSSetShader(sh, ci, nci); }
    void HSSetSamplers(UINT s, UINT n, ID3D11SamplerState *const *ss) override { m_real->HSSetSamplers(s, n, ss); }
    void HSSetConstantBuffers(UINT s, UINT n, ID3D11Buffer *const *b) override { m_real->HSSetConstantBuffers(s, n, b); }
    void DSSetShaderResources(UINT s, UINT n, ID3D11ShaderResourceView *const *v) override { m_real->DSSetShaderResources(s, n, v); }
    void DSSetShader(ID3D11DomainShader *sh, ID3D11ClassInstance *const *ci, UINT nci) override { m_real->DSSetShader(sh, ci, nci); }
    void DSSetSamplers(UINT s, UINT n, ID3D11SamplerState *const *ss) override { m_real->DSSetSamplers(s, n, ss); }
    void DSSetConstantBuffers(UINT s, UINT n, ID3D11Buffer *const *b) override { m_real->DSSetConstantBuffers(s, n, b); }
    void CSSetShaderResources(UINT s, UINT n, ID3D11ShaderResourceView *const *v) override { m_real->CSSetShaderResources(s, n, v); }
    void CSSetUnorderedAccessViews(UINT s, UINT n, ID3D11UnorderedAccessView *const *uav, const UINT *init) override { m_real->CSSetUnorderedAccessViews(s, n, uav, init); }
    void CSSetShader(ID3D11ComputeShader *sh, ID3D11ClassInstance *const *ci, UINT nci) override { m_real->CSSetShader(sh, ci, nci); }
    void CSSetSamplers(UINT s, UINT n, ID3D11SamplerState *const *ss) override { m_real->CSSetSamplers(s, n, ss); }
    void CSSetConstantBuffers(UINT s, UINT n, ID3D11Buffer *const *b) override { m_real->CSSetConstantBuffers(s, n, b); }

    // State-query / getter group we missed
    void VSGetConstantBuffers(UINT s, UINT n, ID3D11Buffer **b) override { m_real->VSGetConstantBuffers(s, n, b); }
    void PSGetShaderResources(UINT s, UINT n, ID3D11ShaderResourceView **v) override { m_real->PSGetShaderResources(s, n, v); }
    void PSGetShader(ID3D11PixelShader **sh, ID3D11ClassInstance **ci, UINT *nci) override { m_real->PSGetShader(sh, ci, nci); }
    void PSGetSamplers(UINT s, UINT n, ID3D11SamplerState **ss) override { m_real->PSGetSamplers(s, n, ss); }
    void VSGetShader(ID3D11VertexShader **sh, ID3D11ClassInstance **ci, UINT *nci) override { m_real->VSGetShader(sh, ci, nci); }
    void PSGetConstantBuffers(UINT s, UINT n, ID3D11Buffer **b) override { m_real->PSGetConstantBuffers(s, n, b); }
    void IAGetInputLayout(ID3D11InputLayout **l) override { m_real->IAGetInputLayout(l); }
    void IAGetVertexBuffers(UINT s, UINT n, ID3D11Buffer **v, UINT *st, UINT *o) override { m_real->IAGetVertexBuffers(s, n, v, st, o); }
    void IAGetIndexBuffer(ID3D11Buffer **ib, DXGI_FORMAT *f, UINT *o) override { m_real->IAGetIndexBuffer(ib, f, o); }
    void GSGetConstantBuffers(UINT s, UINT n, ID3D11Buffer **b) override { m_real->GSGetConstantBuffers(s, n, b); }
    void GSGetShader(ID3D11GeometryShader **sh, ID3D11ClassInstance **ci, UINT *nci) override { m_real->GSGetShader(sh, ci, nci); }
    void IAGetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY *t) override { m_real->IAGetPrimitiveTopology(t); }
    void VSGetShaderResources(UINT s, UINT n, ID3D11ShaderResourceView **v) override { m_real->VSGetShaderResources(s, n, v); }
    void VSGetSamplers(UINT s, UINT n, ID3D11SamplerState **ss) override { m_real->VSGetSamplers(s, n, ss); }
    void GetPredication(ID3D11Predicate **p, BOOL *v) override { m_real->GetPredication(p, v); }
    void GSGetShaderResources(UINT s, UINT n, ID3D11ShaderResourceView **v) override { m_real->GSGetShaderResources(s, n, v); }
    void GSGetSamplers(UINT s, UINT n, ID3D11SamplerState **ss) override { m_real->GSGetSamplers(s, n, ss); }
    void OMGetRenderTargets(UINT n, ID3D11RenderTargetView **rt, ID3D11DepthStencilView **dsv) override { m_real->OMGetRenderTargets(n, rt, dsv); }
    void OMGetRenderTargetsAndUnorderedAccessViews(UINT nrt, ID3D11RenderTargetView **rt, ID3D11DepthStencilView **dsv, UINT uavStart, UINT nuav, ID3D11UnorderedAccessView **uav) override { m_real->OMGetRenderTargetsAndUnorderedAccessViews(nrt, rt, dsv, uavStart, nuav, uav); }
    void OMGetBlendState(ID3D11BlendState **bs, FLOAT bf[4], UINT *sm) override { m_real->OMGetBlendState(bs, bf, sm); }
    void OMGetDepthStencilState(ID3D11DepthStencilState **ds, UINT *sr) override { m_real->OMGetDepthStencilState(ds, sr); }
    void SOGetTargets(UINT n, ID3D11Buffer **t) override { m_real->SOGetTargets(n, t); }
    void RSGetState(ID3D11RasterizerState **rs) override { m_real->RSGetState(rs); }
    void RSGetViewports(UINT *n, D3D11_VIEWPORT *vp) override { m_real->RSGetViewports(n, vp); }
    void RSGetScissorRects(UINT *n, D3D11_RECT *rc) override { m_real->RSGetScissorRects(n, rc); }
    void HSGetShaderResources(UINT s, UINT n, ID3D11ShaderResourceView **v) override { m_real->HSGetShaderResources(s, n, v); }
    void HSGetShader(ID3D11HullShader **sh, ID3D11ClassInstance **ci, UINT *nci) override { m_real->HSGetShader(sh, ci, nci); }
    void HSGetSamplers(UINT s, UINT n, ID3D11SamplerState **ss) override { m_real->HSGetSamplers(s, n, ss); }
    void HSGetConstantBuffers(UINT s, UINT n, ID3D11Buffer **b) override { m_real->HSGetConstantBuffers(s, n, b); }
    void DSGetShaderResources(UINT s, UINT n, ID3D11ShaderResourceView **v) override { m_real->DSGetShaderResources(s, n, v); }
    void DSGetShader(ID3D11DomainShader **sh, ID3D11ClassInstance **ci, UINT *nci) override { m_real->DSGetShader(sh, ci, nci); }
    void DSGetSamplers(UINT s, UINT n, ID3D11SamplerState **ss) override { m_real->DSGetSamplers(s, n, ss); }
    void DSGetConstantBuffers(UINT s, UINT n, ID3D11Buffer **b) override { m_real->DSGetConstantBuffers(s, n, b); }
    void CSGetShaderResources(UINT s, UINT n, ID3D11ShaderResourceView **v) override { m_real->CSGetShaderResources(s, n, v); }
    void CSGetUnorderedAccessViews(UINT s, UINT n, ID3D11UnorderedAccessView **uav) override { m_real->CSGetUnorderedAccessViews(s, n, uav); }
    void CSGetShader(ID3D11ComputeShader **sh, ID3D11ClassInstance **ci, UINT *nci) override { m_real->CSGetShader(sh, ci, nci); }
    void CSGetSamplers(UINT s, UINT n, ID3D11SamplerState **ss) override { m_real->CSGetSamplers(s, n, ss); }
    void CSGetConstantBuffers(UINT s, UINT n, ID3D11Buffer **b) override { m_real->CSGetConstantBuffers(s, n, b); }

    // -------- context-wide operations --------
    void ClearState() override { m_real->ClearState(); }
    void Flush() override { m_real->Flush(); }
    D3D11_DEVICE_CONTEXT_TYPE GetType() override { return m_real->GetType(); }
    UINT GetContextFlags() override { return m_real->GetContextFlags(); }
    HRESULT FinishCommandList(BOOL restore, ID3D11CommandList **cl) override { return m_real->FinishCommandList(restore, cl); }

private:
    ID3D11DeviceContext *m_real;
    std::atomic<uint32_t> m_ref;
};
