// definitions for symbol hiding
#pragma once

#ifndef _WIN32
	#define EXPORT_SYM __attribute__ ((visibility ("default")))
	#define LOCAL_SYM __attribute__ ((visibility ("hidden")))
#else
	#define EXPORT_SYM _declspec(dllexport)
	// Unlike *Nix shared libraries,
	// Windows DLLs do not export anything unless explicitly told to.
	// So we don't need to worry about this!
	#define LOCAL_SYM
#endif

