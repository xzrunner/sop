#ifndef PARAM_INFO
#error "You must define PARAM_INFO macro before include this file"
#endif

PARAM_INFO(MesureType, Measure::Type, type, m_type, (Measure::Type::Perimeter))

PARAM_INFO(OverrideName, bool,        override,   m_override,  (false))
PARAM_INFO(MesureName,   std::string, attribname, m_attr_name, (""))