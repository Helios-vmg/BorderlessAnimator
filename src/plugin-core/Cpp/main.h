/*
Copyright (c), Helios
All rights reserved.

Distributed under a permissive license. See COPYING.txt for details.
*/

#ifndef CPPINTERPRETERMAIN_H
#define CPPINTERPRETERMAIN_H

#ifndef __cplusplus
#error Only use this file from C++!
#endif

#include "../CallResult.h"

#define CPP_PLUGIN_EXTERN_C extern "C"

#if defined WIN32 && defined BUILDING_CPPINTERPRETER
#define CPP_PLUGIN_EXPORT_C CPP_PLUGIN_EXTERN_C __declspec(dllexport)
#else
#define CPP_PLUGIN_EXPORT_C CPP_PLUGIN_EXTERN_C
#endif

class CppInterpreter;
typedef void *external_state;

struct CppInterpreterParameters{
	external_state state;
	int caller_image;

#define CppInterpreterParameters_DECLARE_FUNCTION(rt, x, ...) \
	typedef rt (*x##_f)(external_state, __VA_ARGS__); \
	x##_f x
	CppInterpreterParameters_DECLARE_FUNCTION(void, release_returned_string, char *);
	CppInterpreterParameters_DECLARE_FUNCTION(void, store_tls, void *);
	CppInterpreterParameters_DECLARE_FUNCTION(void *, retrieve_tls);
};

#define Cpp_DECLARE_EXPORTED_FUNCTION(rt, x, ...) \
	typedef rt (*x##_f)(__VA_ARGS__); \
	CPP_PLUGIN_EXPORT_C rt x(__VA_ARGS__)

Cpp_DECLARE_EXPORTED_FUNCTION(CppInterpreter *, new_CppInterpreter, CppInterpreterParameters *parameters);
Cpp_DECLARE_EXPORTED_FUNCTION(void, delete_CppInterpreter, CppInterpreter *);
Cpp_DECLARE_EXPORTED_FUNCTION(void, CppInterpreter_execute, CallResult *result, CppInterpreter *, const char *filename);
Cpp_DECLARE_EXPORTED_FUNCTION(void, delete_CppCallResult, CallResult *);

#endif
