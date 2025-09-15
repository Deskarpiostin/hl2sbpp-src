//========== Copyleft � 2011, Team Sandbox, Some rights reserved. ===========//
//
// Purpose:
//
//===========================================================================//

#include "cbase.h"
#include "filesystem.h"
#include "luamanager.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void MountAddons()
{
    char originalDir[512] = {0};
    bool bGotCurrentDir = V_GetCurrentDirectory(originalDir, sizeof(originalDir));

    if (bGotCurrentDir)
    {
#ifdef CLIENT_DLL
        const char *gamePath = engine->GetGameDirectory();
#else
        char gamePath[256];
        engine->GetGameDir(gamePath, 256);
#endif
        V_SetCurrentDirectory(gamePath);
    }

    filesystem->AddSearchPath(LUA_PATH_CACHE, "MOD", PATH_ADD_TO_HEAD);
    filesystem->AddSearchPath(LUA_PATH_CACHE, "GAME", PATH_ADD_TO_HEAD);

    if (bGotCurrentDir)
        V_SetCurrentDirectory(originalDir);

    FileFindHandle_t fh;

    char relativePath[MAX_PATH] = {0};
    char addonName[255] = {0};

    const char *fn = g_pFullFileSystem->FindFirstEx(LUA_PATH_ADDONS "/*", "MOD", &fh);
    while (fn)
    {
        if (fn[0] != '.')
        {
            Q_strcpy(addonName, fn);

            if (g_pFullFileSystem->FindIsDirectory(fh))
            {
#ifdef GAME_DLL
                Msg("Mounting addon \"%s\"...\n", addonName);
#endif
                // Build relative path to addon
                Q_snprintf(relativePath, sizeof(relativePath), LUA_PATH_ADDONS "/%s", addonName);

                // Save current directory again
                char addonCWD[512] = {0};
                bool bGotCWD = V_GetCurrentDirectory(addonCWD, sizeof(addonCWD));

                if (bGotCWD)
                {
#ifdef CLIENT_DLL
                    const char *gamePath = engine->GetGameDirectory();
#else
                    char gamePath[256];
                    engine->GetGameDir(gamePath, 256);
#endif
                    V_SetCurrentDirectory(gamePath);
                }

                filesystem->AddSearchPath(relativePath, "MOD", PATH_ADD_TO_HEAD);
                filesystem->AddSearchPath(relativePath, "GAME", PATH_ADD_TO_HEAD);

                if (bGotCWD)
                    V_SetCurrentDirectory(addonCWD);
            }
        }

        fn = g_pFullFileSystem->FindNext(fh);
    }

    g_pFullFileSystem->FindClose(fh);
}
