//========= Copyright Me idk lol, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "advanced_multiplayer_tab.h"
#include "advanced_options.h"
#include <vgui/ISurface.h>
#include "filesystem.h"
#include "fmtstr.h"
#include "luamanager.h"

using namespace vgui;

static bool s_bPopulatingNameField = false;
static const char *s_sPlayerNameCfg = "cfg/plus_playername.cfg";

static void SavePlayerNameToFile( const char *name )
{
    KeyValues *kv = new KeyValues( "PlayerName" );
    kv->SetString( "name", name ? name : "" );
    kv->SaveToFile( g_pFullFileSystem, s_sPlayerNameCfg );
    kv->deleteThis();
}

static void LoadPlayerNameFromFile( CUtlString &out )
{
    KeyValues *kv = new KeyValues( "PlayerName" );
    if ( kv->LoadFromFile( g_pFullFileSystem, s_sPlayerNameCfg ) )
    {
        const char *n = kv->GetString( "name", "" );
        out = n ? n : "";
    }
    else
    {
        out = "";
    }
    kv->deleteThis();
}

ColorPreset GlobalPresets[] = {
    { "Red",    255, 0, 0 },
    { "Green",  0, 255, 0 },
    { "Blue",   0, 0, 255 },
    { "White",  255, 255, 255 },
    { "Cyan",   0, 255, 255 },
    { "Purple", 128, 0, 128 },
    { "Yellow", 255, 255, 0 },
    { "Orange", 255, 128, 0 }
};

static void LoadHandModelsFromLua(std::vector<HandModelInfo> &out)
{
    out.clear();

    lua_State* Lstate = nullptr;
    Lstate = (LGameUI ? LGameUI : L);

    lua_getglobal(Lstate, "HandModels");
    if (!lua_istable(Lstate, -1))
    {
        lua_pop(Lstate, 1);
        return;
    }

    lua_pushnil(Lstate); // first key
    while (lua_next(Lstate, -2) != 0)
    {
        const char* topKey = nullptr;
        if (lua_type(Lstate, -2) == LUA_TSTRING)
        {
            topKey = lua_tostring(Lstate, -2);
        }
        else
        {
            lua_tostring(Lstate, -2);
            topKey = lua_tostring(Lstate, -1);
            lua_pop(Lstate, 1); // remove tostring result
        }

        if (topKey && lua_istable(Lstate, -1))
        {
            lua_getfield(Lstate, -1, "model");
            bool hasModel = lua_isstring(Lstate, -1);
            lua_pop(Lstate, 1);

            if (hasModel)
            {
                HandModelInfo info;
                info.key = topKey;

                lua_getfield(Lstate, -1, "model");
                info.model = lua_isstring(Lstate, -1) ? lua_tostring(Lstate, -1) : "";
                lua_pop(Lstate, 1);

                lua_getfield(Lstate, -1, "skin");
                info.skin = lua_isnumber(Lstate, -1) ? (int)lua_tointeger(Lstate, -1) : 0;
                lua_pop(Lstate, 1);

                lua_getfield(Lstate, -1, "name");
                info.name = lua_isstring(Lstate, -1) ? lua_tostring(Lstate, -1) : std::string(topKey);
                lua_pop(Lstate, 1);

                out.push_back(info);
            }
            else
            {
                lua_pushnil(Lstate); // first subkey
                while (lua_next(Lstate, -2) != 0)
                {
                    // -1 = subvalue table, -2 = subkey
                    const char* subKey = nullptr;
                    if (lua_type(Lstate, -2) == LUA_TSTRING)
                        subKey = lua_tostring(Lstate, -2);
                    else
                    {
                        lua_tostring(Lstate, -2);
                        subKey = lua_tostring(Lstate, -1);
                        lua_pop(Lstate, 1);
                    }

                    if (subKey && lua_istable(Lstate, -1))
                    {
                        HandModelInfo info;
                        info.key = subKey;

                        lua_getfield(Lstate, -1, "model");
                        info.model = lua_isstring(Lstate, -1) ? lua_tostring(Lstate, -1) : "";
                        lua_pop(Lstate, 1);

                        lua_getfield(Lstate, -1, "skin");
                        info.skin = lua_isnumber(Lstate, -1) ? (int)lua_tointeger(Lstate, -1) : 0;
                        lua_pop(Lstate, 1);

                        lua_getfield(Lstate, -1, "name");
                        std::string subName = lua_isstring(Lstate, -1) ? lua_tostring(Lstate, -1) : std::string(subKey);
                        lua_pop(Lstate, 1);

                        info.name = std::string(topKey) + " - " + subName;

                        out.push_back(info);
                    }

                    lua_pop(Lstate, 1);
                }
            }
        }

        lua_pop(Lstate, 1);
    }

    lua_pop(Lstate, 1);
}

static void ApplyColorToConvarsByTarget( int target, int r, int g, int b )
{
    if ( target == CAdvancedOptionsMultiplayer::COLORTARGET_PLAYER )
    {
        ConVar *cr = cvar->FindVar( "playercolor_r" );
        ConVar *cg = cvar->FindVar( "playercolor_g" );
        ConVar *cb = cvar->FindVar( "playercolor_b" );
        if ( cr ) cr->SetValue( r );
        if ( cg ) cg->SetValue( g );
        if ( cb ) cb->SetValue( b );
    }
    else if ( target == CAdvancedOptionsMultiplayer::COLORTARGET_WEAPON )
    {
        ConVar *cr = cvar->FindVar( "physgun_r" );
        ConVar *cg = cvar->FindVar( "physgun_g" );
        ConVar *cb = cvar->FindVar( "physgun_b" );
        if ( cr ) cr->SetValue( r );
        if ( cg ) cg->SetValue( g );
        if ( cb ) cb->SetValue( b );
    }
}

CAdvancedOptionsMultiplayer::CAdvancedOptionsMultiplayer( Panel *parent, const char *panelName )
    : BaseClass( parent, panelName ),
      m_pPlayerColorBtn(nullptr),
      m_pWeaponColorBtn(nullptr),
      m_pPlayerForward(nullptr),
      m_pWeaponForward(nullptr)
{
    m_pNameLabel = new Label( this, "NameLabel", "Player Name:" );
    m_pNameEntry = new TextEntry( this, "NameEntry" );
	m_pPMModel = new CMDLPanel( this, "PMModel" );

    m_pNameEntry->SetAllowNonAsciiCharacters( true );
    m_pNameEntry->SetMaximumCharCount( 32 );
    m_pNameEntry->AddActionSignalTarget( this );

    m_pPMSelector = new ComboBox( this, "PMSelector", 8, false );
    m_pPMSelector->SetAllowNonAsciiCharacters( false );

    m_pPlayerForward = new CColorPickerForwardPanel( this, this, COLORTARGET_PLAYER );
    m_pWeaponForward = new CColorPickerForwardPanel( this, this, COLORTARGET_WEAPON );

    m_pPlayerColorBtn = new CColorPickerButton( this, "PlayerColorBtn", m_pPlayerForward );
	m_pPlayerColorBtn->SetTooltip(nullptr, "Change the color of the player character");

    m_pWeaponColorBtn = new CColorPickerButton( this, "WeaponColorBtn", m_pWeaponForward );
	m_pWeaponColorBtn->SetTooltip(nullptr, "Change the color of the physics gun");

	m_pPlayerColorLabel = new Label( this, "PlayerColorLabel", "Player Color" );
	m_pWeaponColorLabel = new Label( this, "WeaponColorLabel", "Weapon Color" );

	m_pVerticalSeparator = new Panel(this, "VerticalSeparator");
	m_pVerticalSeparator->SetBgColor(Color(128, 128, 128, 255)); // gray

	m_pHorizontalSeparator = new Panel(this, "HorizontalSeparator");
	m_pHorizontalSeparator->SetBgColor(Color(128, 128, 128, 255)); // gray

	m_pPlayerPresetsLabel = new Label( this, "PlayerPresetsLabel", "Player Color Presets" );
	m_pWeaponPresetsLabel = new Label( this, "WeaponPresetsLabel", "Weapon Color Presets" );

	m_pHandModelSelector = new ComboBox(this, "HandModelSelector", 4, false);

	for (int i = 0; i < ARRAYSIZE(GlobalPresets); i++)
	{
		char buf[256];
		Q_snprintf(buf, sizeof(buf), "PlayerPresetBtn%d", i);
		Button* btn = new Button(this, buf, GlobalPresets[i].name, this, buf);
		btn->SetCommand(buf);
		btn->SetFgColor(Color(GlobalPresets[i].r, GlobalPresets[i].g, GlobalPresets[i].b, 255));
		m_PlayerPresetBtns.AddToTail(btn);
	}

	for (int i = 0; i < ARRAYSIZE(GlobalPresets); i++)
	{
		char buf[256];
		Q_snprintf(buf, sizeof(buf), "WeaponPresetBtn%d", i);
		Button* btn = new Button(this, buf, GlobalPresets[i].name, this, buf);
		btn->SetCommand(buf);
		btn->SetFgColor(Color(GlobalPresets[i].r, GlobalPresets[i].g, GlobalPresets[i].b, 255));
		m_WeaponPresetBtns.AddToTail(btn);
	}

    vgui::ivgui()->AddTickSignal( GetVPanel(), 100 );
}

void CAdvancedOptionsMultiplayer::PopulatePlayerModels()
{
    m_PMPaths.clear();
    if (!m_pPMSelector) return;
    m_pPMSelector->DeleteAllItems();

    std::vector<std::string> dirs = { "models/player" };

    while (!dirs.empty())
    {
        std::string path = dirs.back(); dirs.pop_back();
        FileFindHandle_t fh;
        const char* file = g_pFullFileSystem->FindFirst((path + "/*").c_str(), &fh);

        while (file)
        {
            if (Q_stricmp(file, ".") && Q_stricmp(file, ".."))
            {
                std::string fullPath = path + "/" + file;

                FileFindHandle_t subfh;
                const char* test = g_pFullFileSystem->FindFirst((fullPath + "/*").c_str(), &subfh);
                if (test) 
                {
                    dirs.push_back(fullPath);
                    g_pFullFileSystem->FindClose(subfh);
                }
                else if (Q_stristr(file, ".mdl"))
                {
                    m_PMPaths.push_back(fullPath);
					m_pPMSelector->AddItem(fullPath.c_str(), nullptr);
                }
            }
            file = g_pFullFileSystem->FindNext(fh);
        }

        g_pFullFileSystem->FindClose(fh);
    }

    ConVar* pm = cvar->FindVar("cl_playermodel");
	if (pm && !m_PMPaths.empty())
	{
		char curBuf[MAX_PATH];
		Q_strncpy(curBuf, pm->GetString(), sizeof(curBuf));
		V_FixSlashes(curBuf);
		Q_strlower(curBuf);

		auto normalize = [](const char* in, char* out, int outSize) {
			Q_strncpy(out, in, outSize);
			V_FixSlashes(out);
			Q_strlower(out);
		};

		auto ends_with = [](const char* str, const char* suffix) -> bool {
			if (!str || !suffix) return false;
			int slen = Q_strlen(str);
			int suflen = Q_strlen(suffix);
			if (suflen > slen) return false;
			return (Q_stricmp(str + slen - suflen, suffix) == 0);
		};

		int foundIndex = -1;
		for (int i = 0; i < (int)m_PMPaths.size(); ++i)
		{
			char normalizedPath[MAX_PATH];
			normalize(m_PMPaths[i].c_str(), normalizedPath, sizeof(normalizedPath));
			if (!Q_stricmp(normalizedPath, curBuf))
			{
				foundIndex = i;
				break;
			}
		}

		if (foundIndex == -1)
		{
			for (int i = 0; i < (int)m_PMPaths.size(); ++i)
			{
				char normalizedPath[MAX_PATH];
				normalize(m_PMPaths[i].c_str(), normalizedPath, sizeof(normalizedPath));

				if (ends_with(normalizedPath, curBuf) || ends_with(curBuf, normalizedPath))
				{
					foundIndex = i;
					break;
				}

				const char* lastSlash = Q_strrchr(normalizedPath, '/');
				const char* baseName = lastSlash ? lastSlash + 1 : normalizedPath;
				if (!Q_stricmp(baseName, curBuf) || ends_with(curBuf, baseName))
				{
					foundIndex = i;
					break;
				}
			}
		}

		if (foundIndex != -1)
		{
			m_pPMSelector->ActivateItem(foundIndex);
			m_iLastPMIndex = foundIndex;

			if (m_pPMModel)
			{
				m_pPMModel->SetMDL(m_PMPaths[foundIndex].c_str());
				m_pPMModel->LookAtMDL();
			}
		}
		else
		{
			int curSel = m_pPMSelector->GetActiveItem();
			m_iLastPMIndex = (curSel >= 0) ? curSel : 0;

			if (m_pPMModel)
			{
				m_pPMModel->SetMDL(pm->GetString());
				m_pPMModel->LookAtMDL();
			}
		}
	}
}

void CAdvancedOptionsMultiplayer::OnTextChanged( KeyValues *pKeyValues )
{
    if ( s_bPopulatingNameField )
        return;

    char buf[ MAX_PATH ];
    m_pNameEntry->GetText( buf, sizeof( buf ) );

    char tmp[MAX_PATH];
    int j = 0;
    for (int i = 0; buf[i] != '\0' && j < (int)sizeof(tmp) - 1; ++i)
    {
        if ( buf[i] == '"' ) continue;
        tmp[j++] = buf[i];
    }
    tmp[j] = '\0';
    Q_strncpy( buf, tmp, sizeof(buf) );

    // trim left
    int start = 0;
    while ( buf[start] && isspace((unsigned char)buf[start]) ) start++;
    if ( start > 0 ) memmove( buf, buf + start, Q_strlen(buf + start) + 1 );
    // trim right
    int len = Q_strlen(buf);
    while ( len > 0 && isspace((unsigned char)buf[len-1]) ) { buf[len-1] = '\0'; --len; }

    buf[ m_pNameEntry->GetMaximumCharCount() ] = '\0';

    SavePlayerNameToFile( buf );

    ConVar* nameVar = cvar->FindVar( "name" );
    if ( nameVar )
        nameVar->SetValue( buf );

    char cmd[ MAX_PATH ];
    Q_snprintf( cmd, sizeof( cmd ), "name \"%s\"", buf );
    engine->ClientCmd_Unrestricted( cmd );
}

void CAdvancedOptionsMultiplayer::OnTick()
{
    BaseClass::OnTick();

    int sel = m_pPMSelector->GetActiveItem();
    if (sel != m_iLastPMIndex && sel >= 0 && sel < (int)m_PMPaths.size())
    {
        m_iLastPMIndex = sel;
        const char* modelPath = m_PMPaths[sel].c_str();

        ConVar* pm = cvar->FindVar("cl_playermodel");
        if (pm)
        {
            pm->SetValue(modelPath);
            m_pszCurrentPM = modelPath;
        }

        if (m_pPMModel)
        {
            m_pPMModel->SetMDL(modelPath);
            m_pPMModel->LookAtMDL();
        }
    }

    sel = m_pHandModelSelector->GetActiveItem();
    if (sel >= 0 && sel < (int)m_HandModels.size())
    {
        char cmd[256];
        Q_snprintf(cmd, sizeof(cmd), "c_handmodel %s", m_HandModels[sel].key.c_str());
        engine->ClientCmd_Unrestricted(cmd);
    }
}

void CAdvancedOptionsMultiplayer::OnCommand( const char *command )
{
    BaseClass::OnCommand( command );

    for (int i = 0; i < ARRAYSIZE(GlobalPresets); i++)
    {
        char buf[MAX_PATH];
        Q_snprintf(buf, sizeof(buf), "PlayerPresetBtn%d", i);
        if (!Q_stricmp(command, buf))
        {
            ApplyColorToConvarsByTarget(COLORTARGET_PLAYER, GlobalPresets[i].r, GlobalPresets[i].g, GlobalPresets[i].b);
            if (m_pPlayerColorBtn)
                m_pPlayerColorBtn->SetColor(GlobalPresets[i].r, GlobalPresets[i].g, GlobalPresets[i].b, 255);
            return;
        }
    }

    for (int i = 0; i < ARRAYSIZE(GlobalPresets); i++)
    {
        char buf[MAX_PATH];
        Q_snprintf(buf, sizeof(buf), "WeaponPresetBtn%d", i);
        if (!Q_stricmp(command, buf))
        {
            ApplyColorToConvarsByTarget(COLORTARGET_WEAPON, GlobalPresets[i].r, GlobalPresets[i].g, GlobalPresets[i].b);
            if (m_pWeaponColorBtn)
                m_pWeaponColorBtn->SetColor(GlobalPresets[i].r, GlobalPresets[i].g, GlobalPresets[i].b, 255);
            return;
        }
    }
}

void CAdvancedOptionsMultiplayer::PerformLayout()
{
    BaseClass::PerformLayout();

	PopulatePlayerModels();

    m_pNameLabel->SetBounds(12, 12, 100, 30);
    m_pNameEntry->SetBounds(97, 12, 100, 30);

    ConVar *pm = cvar->FindVar("cl_playermodel");
    m_pszCurrentPM = pm->GetString();

    int modelX = 10;
    int modelY = 50;
    int modelW = 250;
    int modelH = 250;

    m_pPMModel->SetBounds(modelX, modelY, modelW, modelH);

    int comboX = modelX;
    int comboY = modelY + modelH + 10;
    int comboW = modelW;
    int comboH = 25;

    m_pPMSelector->SetBounds(comboX, comboY, comboW, comboH);

    int btnY = comboY + comboH + 15;
    int spacing = 8;
    int btnW = (comboW - spacing) / 2;
    int btnH = 30;

    m_pPlayerColorBtn->SetBounds( comboX, btnY, btnW, btnH );
    m_pWeaponColorBtn->SetBounds( comboX + btnW + spacing, btnY, btnW, btnH );
	
    if ( m_pPlayerColorBtn )
    {
        int pr = 128, pg = 128, pb = 128;
        ConVar *cr = cvar->FindVar( "playercolor_r" );
        ConVar *cg = cvar->FindVar( "playercolor_g" );
        ConVar *cb = cvar->FindVar( "playercolor_b" );
        if ( cr ) pr = cr->GetInt();
        if ( cg ) pg = cg->GetInt();
        if ( cb ) pb = cb->GetInt();
        m_pPlayerColorBtn->SetColor( pr, pg, pb, 255 );
    }

    if ( m_pWeaponColorBtn )
    {
        int wr = 128, wg = 128, wb = 128;
        ConVar *cr = cvar->FindVar( "physgun_r" );
        ConVar *cg = cvar->FindVar( "physgun_g" );
        ConVar *cb = cvar->FindVar( "physgun_b" );
        if ( cr ) wr = cr->GetInt();
        if ( cg ) wg = cg->GetInt();
        if ( cb ) wb = cb->GetInt();
        m_pWeaponColorBtn->SetColor( wr, wg, wb, 255 );
    }

    int labelSpacing = 4;
    int labelHeight = 16;
    int labelY = btnY + btnH + labelSpacing;

    m_pPlayerColorLabel->SetBounds( comboX, labelY, btnW, labelHeight );
	
    m_pWeaponColorLabel->SetBounds( comboX + btnW + spacing, labelY, btnW, labelHeight );
	
    m_pVerticalSeparator->SetBounds(modelX + modelW + 15, modelY, 2, modelH);
    
    int presetAreaX = modelX + modelW + 30;
    int presetStartY = modelY - 20;
    int presetBtnW = 60;
    int presetBtnH = 24;
    int presetSpacing = 16;
    int presetLabelHeight = 18;
    int columns = 2;

    m_pPlayerPresetsLabel->SetBounds(
        presetAreaX,
        presetStartY - presetLabelHeight - 4,
        columns * presetBtnW + (columns - 1) * presetSpacing,
        presetLabelHeight
    );

    int playerRows = (m_PlayerPresetBtns.Count() + columns - 1) / columns;
    for (int i = 0; i < m_PlayerPresetBtns.Count(); i++)
    {
        int row = i / columns;
        int col = i % columns;
        m_PlayerPresetBtns[i]->SetBounds(
            presetAreaX + col * (presetBtnW + presetSpacing),
            presetStartY + row * (presetBtnH + presetSpacing),
            presetBtnW,
            presetBtnH
        );
    }

    int playerPresetsEndY = presetStartY + playerRows * (presetBtnH + presetSpacing);
    int sepY = playerPresetsEndY + 15;
    m_pHorizontalSeparator->SetBounds(presetAreaX, sepY, columns * presetBtnW + (columns - 1) * presetSpacing, 2);

    int weaponPresetsStartY = sepY + 20;
    m_pWeaponPresetsLabel->SetBounds(
        presetAreaX,
        weaponPresetsStartY - presetLabelHeight - 4,
        columns * presetBtnW + (columns - 1) * presetSpacing,
        presetLabelHeight
    );

    int weaponRows = (m_WeaponPresetBtns.Count() + columns - 1) / columns;
    for (int i = 0; i < m_WeaponPresetBtns.Count(); i++)
    {
        int row = i / columns;
        int col = i % columns;
        m_WeaponPresetBtns[i]->SetBounds(
            presetAreaX + col * (presetBtnW + presetSpacing),
            weaponPresetsStartY + row * (presetBtnH + presetSpacing),
            presetBtnW,
            presetBtnH
        );
    }
    
    m_pHandModelSelector->DeleteAllItems();
    m_HandModels.clear();

    LoadHandModelsFromLua(m_HandModels);

    if (m_HandModels.empty())
    {
        struct HandModelEntry { const char* key; const char* path; const char* name; int skin; };
        HandModelEntry models[] = {
            { "citizen",     "models/weapons/c_arms_citizen.mdl", "Citizen", 0 },
            { "combine",     "models/weapons/c_arms_combine.mdl", "Combine", 0 },
            { "refugee",     "models/weapons/c_arms_refugee.mdl", "Refugee", 0 },
            { "cstrike",     "models/weapons/c_arms_cstrike.mdl", "CS:S", 0 },
            { "dod",         "models/weapons/c_arms_dod.mdl", "DOD:S", 0 },
            { "darkcitizen", "models/weapons/c_arms_citizen.mdl", "Dark Citizen", 1 },
            { "zombie",      "models/weapons/c_arms_citizen.mdl", "Zombie", 2 },
            { "default",     "", "Default", 0 }
        };

        for (int i = 0; i < ARRAYSIZE(models); ++i)
        {
            HandModelInfo info;
            info.key = models[i].key;
            info.model = models[i].path;
            info.skin = models[i].skin;
            info.name = models[i].name;
            m_HandModels.push_back(info);
        }
    }

    for (size_t i = 0; i < m_HandModels.size(); ++i)
    {
        m_pHandModelSelector->AddItem(m_HandModels[i].name.c_str(), nullptr);
    }

    const char* c_handmodel = cvar->FindVar("c_handmodel")->GetString();
    int selIndex = 0;
    for (size_t i = 0; i < m_HandModels.size(); ++i)
    {
        if (Q_stricmp(c_handmodel, m_HandModels[i].key.c_str()) == 0)
        {
            selIndex = (int)i;
            break;
        }
    }
    m_pHandModelSelector->ActivateItem(selIndex);

	int handX = 10;
	int handY = comboY + comboH + 80;
	int handW = 250;
	int handH = 25;
	m_pHandModelSelector->SetBounds(handX, handY, handW, handH);

    // model
    m_pPMModel->SetGroundGrid( true );

	s_bPopulatingNameField = true;
	CUtlString fileLoadedName;
	LoadPlayerNameFromFile( fileLoadedName );

	if ( fileLoadedName.Length() > 0 )
	{
		m_pNameEntry->SetText( fileLoadedName.Get() );
	}
	else
	{
		ConVar *nameVar = cvar->FindVar("name");
		if ( nameVar && nameVar->GetString() && nameVar->GetString()[0] != '\0' )
			m_pNameEntry->SetText( nameVar->GetString() );
		else
			m_pNameEntry->SetText( "" );
	}

	s_bPopulatingNameField = false;
}

void CAdvancedOptionsMultiplayer::OnColorPicked( KeyValues *data )
{
    if ( !data ) return;

    int target = data->GetInt( "target", COLORTARGET_NONE );

    int r = data->GetInt( "r", -1 );
    int g = data->GetInt( "g", -1 );
    int b = data->GetInt( "b", -1 );

    if ( (r < 0 || g < 0 || b < 0) && data->GetInt( "color", -1 ) != -1 )
    {
        int packed = data->GetInt( "color", -1 );
        // try ARGB (A R G B) lmao
        r = ( packed >> 16 ) & 0xFF;
        g = ( packed >> 8 ) & 0xFF;
        b = ( packed ) & 0xFF;
    }

    if ( r >= 0 && g >= 0 && b >= 0 && target != COLORTARGET_NONE )
    {
        ApplyColorToConvarsByTarget( target, r, g, b );

        if ( target == COLORTARGET_PLAYER && m_pPlayerColorBtn )
            m_pPlayerColorBtn->SetColor( r, g, b, 255 );
        else if ( target == COLORTARGET_WEAPON && m_pWeaponColorBtn )
            m_pWeaponColorBtn->SetColor( r, g, b, 255 );
    }
}
