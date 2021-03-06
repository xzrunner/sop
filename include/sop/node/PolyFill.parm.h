#ifndef PARAM_INFO
#error "You must define PARAM_INFO macro before include this file"
#endif

PARAM_INFO(FillMode,       PolyFill::Mode, fillmode,       m_fill_mode,       (PolyFill::Mode::QuadrilateralGrid))
PARAM_INFO(SmoothToggle,   bool,           smoothtoggle,   m_smooth_toggle,   (true))
PARAM_INFO(SmoothStrength, float,          smoothstrength, m_smooth_strength, (50.0f))
