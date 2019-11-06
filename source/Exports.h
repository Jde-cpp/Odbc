#pragma once

#ifdef JDE_ODBC_EXPORTS
	#ifdef _MSC_VER
		#define JDE_ODBC_VISIBILITY __declspec( dllexport )
	#else
		#define JDE_ODBC_VISIBILITY __attribute__((visibility("default")))
	#endif
#else 
	#ifdef _MSC_VER
		#define JDE_ODBC_VISIBILITY __declspec( dllimport )
		#if NDEBUG
			#pragma comment(lib, "Jde.DB.Odbc.lib")
		#else
			#pragma comment(lib, "Jde.DB.Odbc.lib")
		#endif
	#else
		#define JDE_ODBC_VISIBILITY
	#endif
#endif