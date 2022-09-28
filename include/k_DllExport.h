#ifndef __K_DLLEXPORT_H
#define __K_DLLEXPORT_H

//https://gcc.gnu.org/wiki/Visibility

#if defined _WIN32 || defined __CYGWIN__
	#ifdef ODL_DLLAPI //import kernel functions
		#ifdef __GNUC__
			#define K_PUBLIC __attribute__ ((dllimport))
		#else
			#define K_PUBLIC __declspec(dllimport) // Note: actually gcc seems to also supports this syntax.
		#endif
		
	#else //building kernel - export kernel functions
		#ifdef __GNUC__
			#define K_PUBLIC __attribute__ ((dllexport))
		#else
			#define K_PUBLIC __declspec(dllexport) // Note: actually gcc seems to also supports this syntax.
		#endif
	#endif
	#define K_LOCAL
#else
	#if __GNUC__ >= 4
		#define K_PUBLIC __attribute__ ((visibility ("default")))
		#define K_LOCAL  __attribute__ ((visibility ("hidden")))
	#else
		#define K_PUBLIC
		#define K_LOCAL
	#endif
#endif

#endif