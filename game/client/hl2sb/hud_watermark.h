// hud_watermark.h
#pragma once
#include "hudelement.h"
#include "vgui_controls/Panel.h"

class CHudWatermark : public CHudElement, public vgui::Panel
{
    DECLARE_CLASS_SIMPLE(CHudWatermark, vgui::Panel);

public:
    CHudWatermark(const char* pElementName);

protected:
    virtual void Paint() OVERRIDE;

private:
    vgui::Label* m_pGameName;
    vgui::Label* m_pDiscord;
};
