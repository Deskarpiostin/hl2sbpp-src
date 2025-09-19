//========= Copyright Me idk lol, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "advanced_visuals_tab.h"
#include "fmtstr.h"

CAdvancedOptionsVisuals::CAdvancedOptionsVisuals( Panel *parent, const char *panelName )
    : BaseClass( parent, panelName )
{
	m_pFPSCheckbox = new vgui::CheckButton(this, "FPSCheckbox", "Show FPS");
	m_pFPSCheckbox->SetCommand("FPSCheckboxToggled");
}

void CAdvancedOptionsVisuals::PerformLayout()
{
    int margin = 12;
    int w = 150;
    int h = 20;

    m_pFPSCheckbox->SetBounds(margin, margin, w, h);
}


void CAdvancedOptionsVisuals::OnTick()
{
    BaseClass::OnTick();
}

void CAdvancedOptionsVisuals::OnCommand(const char* command)
{
    if (!Q_stricmp(command, "FPSCheckboxToggled"))
    {
        int value = m_pFPSCheckbox->IsSelected() ? 1 : 0;

        CFmtStr cmd("cl_showfps %d", value);
        engine->ClientCmd_Unrestricted(cmd.Get());

        return;
    }

    BaseClass::OnCommand(command);
}
