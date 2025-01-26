#define WIN32_LEAN_AND_MEAN
#include "pch.h"
#include <sal.h>
#include <httpserv.h>
#include <windows.h>

// Create the module class.
class IPEchoModule : public CHttpModule
{
public:
    REQUEST_NOTIFICATION_STATUS
        OnBeginRequest(
            IN IHttpContext* pHttpContext,
            IN IHttpEventProvider* pProvider
        )
    {

        UNREFERENCED_PARAMETER(pProvider);

        IHttpResponse* pHttpResponse = pHttpContext->GetResponse();

        DWORD length;
        PCSTR value;
        HRESULT result = pHttpContext->GetServerVariable("REMOTE_ADDR", &value, &length);

        if (FAILED(result))
        {
            pProvider->SetErrorStatus(result);
            return RQ_NOTIFICATION_FINISH_REQUEST;
        }

        if (pHttpResponse != NULL && value != NULL)
        {
            pHttpResponse->Clear();  // Clear any previous response data

            // Set Content-Type header
            const char contentType[] = "text/plain";
            pHttpResponse->SetHeader("Content-Type", contentType, (USHORT)strlen(contentType), TRUE);
            // disable buffering as well as caching
            pHttpResponse->DisableBuffering();
            pHttpResponse->DisableKernelCache(8);

            DWORD cbSent;
            HTTP_DATA_CHUNK dc;
            dc.DataChunkType = HttpDataChunkFromMemory;

            ULONG bufferLength = (ULONG)(strlen(value));

            char contentLengthStr[20];
            _itoa_s(bufferLength, contentLengthStr, sizeof(contentLengthStr), 10);
            pHttpResponse->SetHeader("Content-Length", contentLengthStr, (USHORT)strlen(contentLengthStr), TRUE);

            // Allocate memory for the response buffer
            dc.FromMemory.pBuffer = pHttpContext->AllocateRequestMemory(bufferLength);
            dc.FromMemory.BufferLength = bufferLength;

            if (!dc.FromMemory.pBuffer) {
                return RQ_NOTIFICATION_FINISH_REQUEST;
            }

            // Copy the remote address into the allocated buffer (without null terminator)
            memcpy(dc.FromMemory.pBuffer, value, bufferLength);

            // Write the response
            HRESULT hr = pHttpResponse->WriteEntityChunks(&dc, 1, FALSE, TRUE, &cbSent);
            if (FAILED(hr)) {
                return RQ_NOTIFICATION_FINISH_REQUEST;
            }
            BOOL complete = TRUE;
            pHttpResponse->Flush(FALSE, FALSE, &bufferLength, &complete);
            return RQ_NOTIFICATION_FINISH_REQUEST;
        }

        return RQ_NOTIFICATION_CONTINUE;
    }

};

// Create the module factory class
class MyHttpModuleFactory : public IHttpModuleFactory
{
public:
    HRESULT
        GetHttpModule(
            OUT CHttpModule** ppModule,
            IN IModuleAllocator* pAllocator
        )
    {
        UNREFERENCED_PARAMETER(pAllocator);

        // Create an instance of the module
        IPEchoModule* pModule = new IPEchoModule;
        if (!pModule)
        {
            return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
        }

        *ppModule = pModule;
        return S_OK;
    }

    void Terminate()
    {
        delete this;
    }
};

// Create the module's exported registration function.
HRESULT
__stdcall
RegisterModule(
    DWORD dwServerVersion,
    IHttpModuleRegistrationInfo* pModuleInfo,
    IHttpServer* pGlobalInfo
)
{
    UNREFERENCED_PARAMETER(dwServerVersion);
    UNREFERENCED_PARAMETER(pGlobalInfo);

    // Register the module factory
    return pModuleInfo->SetRequestNotifications(
        new MyHttpModuleFactory,
        RQ_BEGIN_REQUEST,
        0
    );
}

