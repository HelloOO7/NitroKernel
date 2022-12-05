#ifndef __KDLL_H
#define __KDLL_H

#define LIBRARY_PATH_PREFIX "lib/"
#define LIBRARY_PATH_SUFFIX ".dll"

#include "kTypes.h"
#include "k_Init.h"
#include "RPM_Api.h"

#include "k_DllExport.h"

namespace k {
	namespace dll {
		namespace detail {
			class LibraryState;
		};

		class _InitClass {
		private:
			static void _SysInit(rpm::mgr::ModuleManager* mgr);
			static void _SysTerminate();

		friend class ::k::KInitializer;
		};

		typedef detail::LibraryState* LibraryHandle;

		static LibraryHandle _FindLibrary(const char* name);
		
		/**
		 * @brief Loads a shared library. Duplicate load calls for one library will be redirected to the same handle.
		 * 
		 * @param name Friendly name of the requested shared library.
		 * @return A handle to the shared library, or nullptr if the library could not be loaded.
		 */
		K_PUBLIC LibraryHandle LoadLibrary(const char* name);
		/**
		 * @brief Releases a shared library. The library won't be truly released until all parent contexts call this function.
		 * 
		 * @param name Friendly name of the shared library to be deleted.
		 */
		K_PUBLIC void ReleaseLibrary(const char* name);
		/**
		 * @brief Releases a shared library without string lookup slowdown. The library won't be truly released until all parent contexts call this function.
		 * 
		 * @param handle Handle of the shared library to be deleted.
		 */
		K_PUBLIC void ReleaseLibrary(LibraryHandle handle);
		/**
		 * @brief Finds an exported procedure inside a dynamically linked module.
		 * 
		 * @param handle Handle to the module to search within.
		 * @param procName Name of the procedure to search for.
		 * @return Pointer to the procedure (with the Thumb bit set), or nullptr if not found.
		 */
		K_PUBLIC void* GetProcAddress(LibraryHandle handle, const char* procName);
	}
}

#endif