/*
-------------------------------------------------------------
Undefine previous definitions to allow for a different DLL
specification (export / import) to affect these macros
-------------------------------------------------------------*/
#ifdef __DECLARE
#  undef __DECLARE
#endif

#ifdef DECLARE_CLASS
#  undef DECLARE_CLASS
#endif

#ifdef DECLARE_SUBCLASS
#  undef DECLARE_SUBCLASS
#endif

#ifdef DECLARE_ABSTRACT
#  undef DECLARE_ABSTRACT
#endif

#ifdef DECLARE_SUBABSTRACT
#  undef DECLARE_SUBABSTRACT
#endif

#undef CLASS_DLL_ACTION

#pragma warning(pop)