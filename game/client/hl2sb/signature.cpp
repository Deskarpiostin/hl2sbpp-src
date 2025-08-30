//========= Copyright [[BIG SHOT!!!]], All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "signature.h"
#include "tier0/dbg.h"
#include "tier1/utlbuffer.h"
#include "filesystem.h"

#include <string>
#include <vector>
#include <cstdint>

static uint32_t crc32(const std::vector<uint8_t>& data)
{
    static uint32_t table[256] = {0};
    static bool initialized = false;
    if (!initialized)
    {
        for (uint32_t i = 0; i < 256; i++)
        {
            uint32_t crc = i;
            for (uint32_t j = 0; j < 8; j++)
                crc = (crc >> 1) ^ (0xEDB88320 & (~(crc & 1) - 1));
            table[i] = crc;
        }
        initialized = true;
    }

    uint32_t crc = 0xFFFFFFFF;
    for (auto b : data)
        crc = (crc >> 8) ^ table[(crc ^ b) & 0xFF];
    return crc ^ 0xFFFFFFFF;
}

static bool LoadFileToVector(const char* path, std::vector<uint8_t>& outData)
{
    FileHandle_t hFile = g_pFullFileSystem->Open(path, "rb", "MOD");
    if (!hFile) return false;

    int nSize = g_pFullFileSystem->Size(hFile);
    outData.resize(nSize);
    g_pFullFileSystem->Read(outData.data(), nSize, hFile);
    g_pFullFileSystem->Close(hFile);

    return true;
}

static void CheckFileSig(const char* filePath)
{
    char fullPath[MAX_PATH];
    const char* result = g_pFullFileSystem->RelativePathToFullPath(filePath, "MOD", fullPath, sizeof(fullPath));
    Msg("Checking signature for: %s (full path: %s)\n", filePath, result ? result : "NOT FOUND");

    std::vector<uint8_t> fileData, sigData;
    
    if (!LoadFileToVector(filePath, fileData))
        Error("Cannot open file %s for CRC32 signature check!", filePath);
    
    std::string sigPath = std::string(filePath) + ".sig";
    
    if (!LoadFileToVector(sigPath.c_str(), sigData))
        Error("Cannot open signature file %s!", sigPath.c_str());
    
    if (sigData.size() < sizeof(uint32_t))
        Error("Signature file %s is invalid!", sigPath.c_str());
    
    uint32_t storedSig = 0;
    memcpy(&storedSig, sigData.data(), sizeof(uint32_t));
    
    uint32_t actualSig = crc32(fileData);
    
    if (storedSig != actualSig)
        Error("Signature verification failed for %s (expected: %08X, got: %08X)", 
              filePath, storedSig, actualSig);
}

bool checkFilesSignature()
{
    const char* filesToCheck[] = {
        "gameinfo.txt",
    };

    for (const char* filePath : filesToCheck)
        CheckFileSig(filePath);

    return true; // success
}

// todo: fix
class CCRC32SigChecker : public CAutoGameSystemPerFrame
{
public:
    CCRC32SigChecker() : CAutoGameSystemPerFrame("CRC32SigChecker") {}

    virtual void LevelInitPreEntity() override
    {
        static bool bChecked = false;
        if (!bChecked)
        {
            //checkFilesSignature();  // filesystem is initialized
            bChecked = true;        // run only once
        }
    }
};

static CCRC32SigChecker g_AutoSigChecker;
