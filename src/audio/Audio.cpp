//MIT License

//Copyright (c) 2026 Z-Multiplier
#include "Audio.hpp"
Audio::Noise::Noise(std::wstring path):path(path){
    HRESULT hr=S_OK;
    IMFSourceReader *pReader=nullptr;
    IMFMediaType *pType=nullptr;
    IMFSample *pSample=nullptr;
    hr=MFStartup(MF_VERSION);
    if(FAILED(hr)){
        throw std::runtime_error("MFStartup failed");
    }
    hr=MFCreateSourceReaderFromURL(this->path.c_str(),nullptr,&pReader);
    if(FAILED(hr)){
        MFShutdown();
        throw std::runtime_error("Cannot open file with Media Foundation");
    }
    hr=MFCreateMediaType(&pType);
    if(SUCCEEDED(hr)){
        pType->SetGUID(MF_MT_MAJOR_TYPE,MFMediaType_Audio);
        pType->SetGUID(MF_MT_SUBTYPE,MFAudioFormat_Float);
        pType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS,2);
        pType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE,32);
        hr=pReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,nullptr,pType);
        if(FAILED(hr)){
            pType->SetGUID(MF_MT_SUBTYPE,MFAudioFormat_PCM);
            pType->SetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE,16);
            hr=pReader->SetCurrentMediaType(
                (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,
                nullptr,pType);
        }
    }
    if(FAILED(hr)){
        pReader->Release();
        MFShutdown();
        throw std::runtime_error("Failed to set PCM output format");
    }
    pType->Release();
    pType=nullptr;
    IMFMediaType *pOutputType=nullptr;
    hr=pReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,&pOutputType);
    if(SUCCEEDED(hr)){
        sampleRate=MFGetAttributeUINT32(pOutputType,MF_MT_AUDIO_SAMPLES_PER_SECOND,0);
        UINT32 channels=MFGetAttributeUINT32(pOutputType,MF_MT_AUDIO_NUM_CHANNELS,0);
        UINT32 bitsPerSample=MFGetAttributeUINT32(pOutputType,MF_MT_AUDIO_BITS_PER_SAMPLE,0);
        if(sampleRate==0||channels==0||(bitsPerSample!=16&&bitsPerSample!=32)){
            pOutputType->Release();
            pReader->Release();
            MFShutdown();
            throw std::runtime_error("Unsupported audio format");
        }
        cache.clear();
        DWORD flags=0;
        while(true){
            hr=pReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM,0,nullptr,&flags,nullptr,&pSample);
            if(FAILED(hr)||(flags & MF_SOURCE_READERF_ENDOFSTREAM))
                break;
            if(pSample){
                IMFMediaBuffer *pBuffer=nullptr;
                hr=pSample->ConvertToContiguousBuffer(&pBuffer);
                if(SUCCEEDED(hr)){
                    BYTE *data=nullptr;
                    DWORD maxLength=0,currentLength=0;
                    hr=pBuffer->Lock(&data,&maxLength,&currentLength);
                    if(SUCCEEDED(hr)){
                        size_t sampleCount=currentLength/(bitsPerSample/8);
                        if(bitsPerSample==32){
                            float *floatData=reinterpret_cast<float*>(data);
                            if(channels==1){
                                for(size_t i=0;i<sampleCount;++i){
                                    float s=floatData[i];
                                    cache.push_back(s);
                                    cache.push_back(s);
                                }
                            }
                            else if(channels==2){
                                cache.insert(cache.end(),floatData,floatData+sampleCount);
                            }
                            else{
                                for(size_t i=0;i<sampleCount;i+=channels){
                                    float left=floatData[i];
                                    float right=(channels>1)?floatData[i+1]:left;
                                    cache.push_back(left);
                                    cache.push_back(right);
                                }
                            }
                        }
                        else if(bitsPerSample==16){
                            int16_t *intData=reinterpret_cast<int16_t*>(data);
                            if(channels==1){
                                for(size_t i=0;i<sampleCount;++i){
                                    float s=intData[i]/32768.0f;
                                    cache.push_back(s);
                                    cache.push_back(s);
                                }
                            }
                            else if(channels==2){
                                for(size_t i=0;i<sampleCount;++i){
                                    cache.push_back(intData[i]/32768.0f);
                                }
                            }
                            else{
                                for(size_t i=0;i<sampleCount;i+=channels){
                                    float left=intData[i]/32768.0f;
                                    float right=(channels>1)?intData[i+1]/32768.0f:left;
                                    cache.push_back(left);
                                    cache.push_back(right);
                                }
                            }
                        }
                        pBuffer->Unlock();
                    }
                    pBuffer->Release();
                }
                pSample->Release();
                pSample=nullptr;
            }
        }
        pOutputType->Release();
    }
    else{
        pReader->Release();
        MFShutdown();
        throw std::runtime_error("Failed to get output media type");
    }
    pReader->Release();
    MFShutdown();
    if(cache.empty()){
        throw std::runtime_error("No audio data decoded");
    }
}
Audio::AudioManager::AudioManager(){
    active.store(false);
}
Audio::AudioManager::~AudioManager(){
    stop();
}
void Audio::AudioManager::start(){
    if(active.exchange(true)){
        return;
    }
    audioThread=std::thread(&AudioManager::audioThreadFunc,this);
}
void Audio::AudioManager::stop(){
    active=false;
    queueCV.notify_one();
    if(audioThread.joinable()){
        audioThread.join();
    }
}
void Audio::AudioManager::pushRequest(const PlayRequest& request){
    std::lock_guard<std::mutex> lock(audioMutex);
    requestQueue.push(request);
    queueCV.notify_one();
}
void Audio::AudioManager::audioThreadFunc(){
    HRESULT hr=CoInitializeEx(nullptr,COINIT_MULTITHREADED);
    if(FAILED(hr)){
        return;
    }
    IMMDeviceEnumerator *pEnumerator=nullptr;
    IMMDevice *pDevice=nullptr;
    IAudioClient *pAudioClient=nullptr;
    IAudioRenderClient *pRenderClient=nullptr;
    WAVEFORMATEX *pwfx=nullptr;
    HANDLE hBufferEvent=nullptr;
    UINT32 bufferFrameCount=0;
    UINT32 deviceSampleRate=0;
    hr=CoCreateInstance(__uuidof(MMDeviceEnumerator),nullptr,CLSCTX_ALL,__uuidof(IMMDeviceEnumerator),(void**)&pEnumerator);
    if(SUCCEEDED(hr)){
        hr=pEnumerator->GetDefaultAudioEndpoint(eRender,eConsole,&pDevice);
    }
    if(SUCCEEDED(hr)){
        hr=pDevice->Activate(__uuidof(IAudioClient),CLSCTX_ALL,nullptr,(void**)&pAudioClient);
    }
    if(SUCCEEDED(hr)){
        hr=pAudioClient->GetMixFormat(&pwfx);
        if(SUCCEEDED(hr)){
            deviceSampleRate=pwfx->nSamplesPerSec;
        }
    }
    if(SUCCEEDED(hr)){
        hBufferEvent=CreateEvent(nullptr,FALSE,FALSE,nullptr);
        if(!hBufferEvent) hr=E_FAIL;
    }
    if(SUCCEEDED(hr)){
        hr=pAudioClient->Initialize(AUDCLNT_SHAREMODE_SHARED,AUDCLNT_STREAMFLAGS_EVENTCALLBACK,0,0,pwfx,nullptr);
    }
    if(SUCCEEDED(hr)){
        hr=pAudioClient->SetEventHandle(hBufferEvent);
    }
    if(SUCCEEDED(hr)){
        hr=pAudioClient->GetBufferSize(&bufferFrameCount);
    }
    if(SUCCEEDED(hr)){
        hr=pAudioClient->GetService(__uuidof(IAudioRenderClient),(void**)&pRenderClient);
    }
    if(SUCCEEDED(hr)){
        hr=pAudioClient->Start();
    }
    if(FAILED(hr)){
        if(pEnumerator) pEnumerator->Release();
        if(pDevice) pDevice->Release();
        if(pAudioClient) pAudioClient->Release();
        if(pRenderClient) pRenderClient->Release();
        if(pwfx) CoTaskMemFree(pwfx);
        if(hBufferEvent) CloseHandle(hBufferEvent);
        CoUninitialize();
        return;
    }
    struct ActiveRequest{
        std::shared_ptr<Noise> noise;
        float volume;
        float pitch;
        double position;
        bool removeAfterPlay;
    };
    std::vector<ActiveRequest> activeRequests;
    while(active){
        DWORD waitRes=WaitForSingleObject(hBufferEvent,10);
        if(waitRes==WAIT_OBJECT_0){
            UINT32 padding;
            hr=pAudioClient->GetCurrentPadding(&padding);
            if(SUCCEEDED(hr)){
                UINT32 framesToWrite=bufferFrameCount-padding;
                if(framesToWrite>0){
                    float *pData;
                    hr=pRenderClient->GetBuffer(framesToWrite,(BYTE**)&pData);
                    if(SUCCEEDED(hr)){
                        std::memset(pData,0,framesToWrite*2*sizeof(float));
                        for(auto it=activeRequests.begin();it!=activeRequests.end();){
                            ActiveRequest& req=*it;
                            const auto& cache=req.noise->cache;
                            if(cache.empty()){
                                it=activeRequests.erase(it);
                                continue;
                            }
                            double step=req.pitch*(static_cast<double>(req.noise->sampleRate)/deviceSampleRate);
                            double pos=req.position;
                            float vol=req.volume;
                            size_t totalFrames=cache.size()/2;
                            bool finished=false;
                            for(UINT32 frame=0;frame<framesToWrite;++frame){
                                if(pos>=totalFrames){
                                    finished=true;
                                    break;
                                }
                                int index=static_cast<int>(pos);
                                double frac=pos-index;
                                int leftIdx=index*2;
                                int rightIdx=leftIdx+1;
                                float left,right;
                                if(frac<1e-6||static_cast<size_t>(index+1)>=totalFrames){
                                    left=cache[leftIdx];
                                    right=cache[rightIdx];
                                }
                                else{
                                    float left1=cache[leftIdx];
                                    float left2=cache[leftIdx+2];
                                    float right1=cache[rightIdx];
                                    float right2=cache[rightIdx+2];
                                    left=static_cast<float>(left1*(1-frac)+left2*frac);
                                    right=static_cast<float>(right1*(1-frac)+right2*frac);
                                }
                                pData[frame*2]+=left*vol;
                                pData[frame*2+1]+=right*vol;
                                pos+=step;
                            }
                            req.position=pos;
                            if(finished){
                                it=activeRequests.erase(it);
                            }
                            else{
                                ++it;
                            }
                        }
                        for(UINT32 i=0;i<framesToWrite*2;++i){
                            if(pData[i]>1.0f) pData[i]=1.0f;
                            else if(pData[i]<-1.0f) pData[i]=-1.0f;
                        }
                        pRenderClient->ReleaseBuffer(framesToWrite,0);
                    }
                }
            }
        }
        {
            std::lock_guard<std::mutex> lock(audioMutex);
            while(!requestQueue.empty()){
                PlayRequest pr=std::move(requestQueue.front());
                requestQueue.pop();
                ActiveRequest ar;
                ar.noise=std::move(pr.target);
                ar.volume=pr.volume;
                ar.pitch=pr.pitch;
                ar.position=0.0;
                ar.removeAfterPlay=pr.removeAfterPlay;
                activeRequests.push_back(std::move(ar));
            }
        }
    }
    pAudioClient->Stop();
    pEnumerator->Release();
    pDevice->Release();
    pAudioClient->Release();
    pRenderClient->Release();
    CoTaskMemFree(pwfx);
    CloseHandle(hBufferEvent);
    CoUninitialize();
}
Audio::AudioManager& Audio::AudioManager::instance(){
    static AudioManager instance;
    return instance;
}