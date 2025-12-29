//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//===========================================================================//

#include "cbase.h"
#include "modelentities.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CFuncReflectiveGlass : public CFuncBrush
{
	DECLARE_DATADESC();
	DECLARE_CLASS( CFuncReflectiveGlass, CFuncBrush );
	DECLARE_SERVERCLASS();

	CFuncReflectiveGlass()
	{
#ifdef HL2SB
		m_iszReflectRenderTarget = AllocPooledString( "_rt_WaterReflection" );
		m_iszRefractRenderTarget = AllocPooledString( "_rt_WaterRefraction" );
#endif
	}

#ifdef HL2SB
	void InputSetReflectRenderTarget( inputdata_t &inputdata ) { m_iszReflectRenderTarget = inputdata.value.StringID(); }
	void InputSetRefractRenderTarget( inputdata_t &inputdata ) { m_iszRefractRenderTarget = inputdata.value.StringID(); }

	CNetworkVar( string_t, m_iszReflectRenderTarget );
	CNetworkVar( string_t, m_iszRefractRenderTarget );
#endif
};

// automatically hooks in the system's callbacks
BEGIN_DATADESC( CFuncReflectiveGlass )

#ifdef HL2SB
	DEFINE_KEYFIELD( m_iszReflectRenderTarget, FIELD_STRING, "ReflectRenderTarget" ),
	DEFINE_KEYFIELD( m_iszRefractRenderTarget, FIELD_STRING, "RefractRenderTarget" ),

	DEFINE_INPUTFUNC( FIELD_STRING, "SetReflectRenderTarget", InputSetReflectRenderTarget ),
	DEFINE_INPUTFUNC( FIELD_STRING, "SetRefractRenderTarget", InputSetRefractRenderTarget ),
#endif

END_DATADESC()

LINK_ENTITY_TO_CLASS( func_reflective_glass, CFuncReflectiveGlass );

IMPLEMENT_SERVERCLASS_ST( CFuncReflectiveGlass, DT_FuncReflectiveGlass )

#ifdef HL2SB
	SendPropStringT( SENDINFO( m_iszReflectRenderTarget ) ),
	SendPropStringT( SENDINFO( m_iszRefractRenderTarget ) ),
#endif

END_SEND_TABLE()