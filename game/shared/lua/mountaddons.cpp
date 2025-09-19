//========== Copyleft � 2011, Team Sandbox, Some rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "filesystem.h"
#include "luamanager.h"
#include "tier0/icommandline.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void MountAddons()
{
    if (CommandLine()->CheckParm("-noaddons"))
    {
        DevMsg("Addons mounting skipped due to -noaddons parameter.\n");
        return;
    }

    char gamePath[MAX_PATH] = {0};
#ifdef CLIENT_DLL
    const char* gameDir = engine->GetGameDirectory();
    Q_strncpy(gamePath, gameDir, sizeof(gamePath));
#else
    engine->GetGameDir(gamePath, sizeof(gamePath));
#endif

    filesystem->AddSearchPath(LUA_PATH_CACHE, "MOD", PATH_ADD_TO_HEAD);
    filesystem->AddSearchPath(LUA_PATH_CACHE, "GAME", PATH_ADD_TO_HEAD);

    FileFindHandle_t fh;
    char relativePath[MAX_PATH] = {0};
    char addonName[MAX_PATH] = {0};
    char fullPath[MAX_PATH] = {0};

    const char* fn = g_pFullFileSystem->FindFirstEx(LUA_PATH_ADDONS "/*", "MOD", &fh);
    while (fn)
    {
        if (fn[0] != '.')
        {
            Q_strncpy(addonName, fn, sizeof(addonName));

            if (g_pFullFileSystem->FindIsDirectory(fh))
            {
#ifdef GAME_DLL
                DevMsg("Mounting addon \"%s\"...\n", addonName);
#endif
                Q_snprintf(relativePath, sizeof(relativePath), "%s/%s/%s", gamePath, LUA_PATH_ADDONS, addonName);
                g_pFullFileSystem->GetCaseCorrectFullPath_Ptr(relativePath, fullPath, sizeof(fullPath));

                filesystem->AddSearchPath(fullPath, "MOD", PATH_ADD_TO_HEAD);
                filesystem->AddSearchPath(fullPath, "GAME", PATH_ADD_TO_HEAD);
            }
        }

        fn = g_pFullFileSystem->FindNext(fh);
    }

    g_pFullFileSystem->FindClose(fh);
}

