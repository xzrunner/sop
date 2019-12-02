#ifndef PARAM_INFO
#error "You must define PARAM_INFO macro before include this file"
#endif

PARAM_INFO(GroupName, String,    group,     m_group_name, (""))
PARAM_INFO(GroupType, GroupType, grouptype, m_group_type, (GroupType::GuessFromGroup))

PARAM_INFO(RecomputeNormals, Bool, computenorms, m_recompute_normals, (false))
PARAM_INFO(DelNonSelected,   Bool, negate,       m_del_non_selected,  (false))
PARAM_INFO(DelUnusedGroups,  Bool, removegrp,    m_del_unused_groups, (false))