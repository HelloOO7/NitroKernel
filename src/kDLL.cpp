#ifndef __KDLL_CPP
#define __KDLL_CPP

#define ODL_WORKMEM_SIZE 4096

#ifdef GFL
#include "nds/fs.h"
#elif defined(_WIN32)
#include <stdio.h>
#endif

#include "kTypes.h"

#include "Heap/exl_Allocator.h"
#include "Heap/exl_HeapArea.h"
#include "Util/exl_StrEq.h"

#include "RPM_Api.h"
#include "kDLL.h"
#include "k_Debug.h"

namespace k {
	namespace dll {
		/**
		 * @brief Work memory heap.
		 */
		static exl::heap::Allocator*    g_Allocator;
		/**
		 * @brief Reference to the system-global module manager.
		 */
		static rpm::mgr::ModuleManager* g_ModuleMgr;
		/**
		 * @brief Raw work memory area.
		 */
		static void*					g_WorkMemory;
		/**
		 * @brief Last element in the library chain linked list.
		 */
		static detail::LibraryState*	g_LastLibrary;

		namespace detail {
			class LibraryState {
			private:
				u32 			m_LinkCount;
				rpm::Module* 	m_Module;
			public:
				/**
				 * @brief Friendly name of the underlying shared library.
				 * 
				 * F.e. for libShared.dll, this would be "Shared".
				 */
				char*			m_Name;
				/**
				 * @brief Previous library in the library chain.
				 */
				LibraryState*	m_PrevLibrary;
				/**
				 * @brief Next library in the library chain.
				 */
				LibraryState*	m_NextLibrary;

			public:
				/**
				 * @brief Creates a library state handle.
				 * 
				 * @param name Friendly name/identifier of the shared library.
				 * @param module Handle to the library code module.
				 */
				LibraryState(const char* name, rpm::Module* module);

				/**
				 * @brief Destroys a library state handle.
				 */
				~LibraryState();

				/**
				 * @brief Increments the reference counter.
				 */
				void AddLink();

				/**
				 * @brief Decrements the reference counter.
				 * 
				 * @return true If this was the last registered reference and as such the library is eligible for termination.
				 * @return false If, after decrementing the counter, there are one or more references remaining.
				 */
				bool RemoveLink();

				/**
				 * @brief Calls the unload function of the underlying module.
				 * 
				 * @param mgr ModuleManager to use for unloading.
				 */
				void Unload(rpm::mgr::ModuleManager* mgr);
			};

			LibraryState::LibraryState(const char* name, rpm::Module* module) {
				m_Name = static_cast<char*>(g_Allocator->Alloc(strlen(name) + 1));
				strcpy(m_Name, name);
				m_LinkCount = 0;
				m_Module = module;
				m_PrevLibrary = nullptr;
				m_NextLibrary = nullptr;
			}

			LibraryState::~LibraryState() {
				g_Allocator->Free(m_Name);
			}

			void LibraryState::AddLink() {
				m_LinkCount++;
			}

			bool LibraryState::RemoveLink() {
				if (m_LinkCount) {
					m_LinkCount--;
				}
				return m_LinkCount == 0;
			}

			void LibraryState::Unload(rpm::mgr::ModuleManager* mgr) {
				mgr->UnloadModule(m_Module);
			}
		}

		static void _SysTerminate();

		/**
		 * @brief Initializes the LibrarySystem global state.
		 * 
		 * @param mgr System-global parent module manager.
		 */
		static void _SysInit(rpm::mgr::ModuleManager* mgr) {
			if (g_ModuleMgr) {
				_SysTerminate();
			}
			g_ModuleMgr = mgr;
			g_WorkMemory = mgr->AllocModuleWorkMemory(ODL_WORKMEM_SIZE);
			g_Allocator = new(mgr->AllocModuleWorkMemory(sizeof(exl::heap::HeapArea))) exl::heap::HeapArea("DLLWorkHeap", g_WorkMemory, ODL_WORKMEM_SIZE);
		}

		/**
		 * @brief Unloads all child modules and terminates the LibrarySystem global state.
		 */
		static void _SysTerminate() {
			while (g_LastLibrary) {
				detail::LibraryState* lastLib = g_LastLibrary;
				lastLib->Unload(g_ModuleMgr);
				g_LastLibrary = lastLib->m_PrevLibrary;
				delete lastLib;
			}

			if (g_ModuleMgr) {
				g_ModuleMgr->FreeModuleWorkMemory(g_WorkMemory);
				delete g_Allocator;
				g_ModuleMgr = nullptr;
			}
		}

		extern "C" RPM_DLLAPI_DLLMAIN_DEFINE {
			switch (reason) {
				case rpm::DllMainReason::MODULE_LOAD:
					k::dll::_SysInit(mgr);
					break;
				case rpm::DllMainReason::MODULE_UNLOAD:
					k::dll::_SysTerminate();
					break;
			}
			return rpm::DllMainReturnCode::OK;
		}

		static rpm::init::ModuleAllocation ReadLibrary(const char* path, rpm::mgr::ModuleManager* mgr);

		static char* MakeLibraryPath(const char* name) {
			size_t prefixLen = strlen(LIBRARY_PATH_PREFIX);
			size_t suffixLen = strlen(LIBRARY_PATH_SUFFIX);

			size_t nameLen = strlen(name);
			char* fullPath = static_cast<char*>(g_Allocator->Alloc(prefixLen + nameLen + suffixLen + 1));
			char* out = fullPath;
			strcpy(out, LIBRARY_PATH_PREFIX);
			out += prefixLen;
			strcpy(out, name);
			out += nameLen;
			strcpy(out, LIBRARY_PATH_SUFFIX);
			return fullPath;
		}

		LibraryHandle _FindLibrary(const char* name) {
			if (name) {
				detail::LibraryState* lib = g_LastLibrary;
				while (lib) {
					if (strequal(lib->m_Name, name)) {
						return lib;
					}
					else {
						lib = lib->m_PrevLibrary;
					}
				}
			}
			return nullptr;
		}

		LibraryHandle LoadLibrary(const char* name) {
			K_DEBUG_PRINTF("LoadLibrary requested for %s\n", name);
			if (!name) {
				return nullptr;
			}
			if (!g_ModuleMgr) {
				return nullptr;
			}
			detail::LibraryState* state = _FindLibrary(name);

			if (!state) {
				K_DEBUG_PRINTF("Loading library %s...\n", name);
				char* libraryPath = MakeLibraryPath(name);
				rpm::init::ModuleAllocation moduleAlloc = ReadLibrary(libraryPath, g_ModuleMgr);
				g_Allocator->Free(libraryPath);

				if (moduleAlloc == nullptr) {
					return nullptr;
				}

				rpm::Module* module = g_ModuleMgr->LoadModule(moduleAlloc);

				state = new(g_Allocator) detail::LibraryState(name, module);

				if (g_LastLibrary) {
					g_LastLibrary->m_NextLibrary = state;
					state->m_PrevLibrary = g_LastLibrary;
				}
				g_LastLibrary = state;

				g_ModuleMgr->StartModule(module, rpm::FixLevel::INTERNAL_RELOCATIONS);
				K_DEBUG_PRINTF("Library loaded!\n");
			}
			else {
				K_DEBUG_PRINTF("Library %s is already loaded. (sanity check state name: %s)\n", name, state->m_Name);
			}

			state->AddLink();

			return state;
		}

		void ReleaseLibrary(const char* name) {
			LibraryHandle hnd = _FindLibrary(name);
			if (hnd) {
				ReleaseLibrary(hnd);
			}
		}

		void ReleaseLibrary(LibraryHandle handle) {
			if (handle->RemoveLink()) {
				detail::LibraryState* state = handle;

				if (state->m_PrevLibrary) {
					state->m_PrevLibrary->m_NextLibrary = state->m_NextLibrary;
				}
				if (state->m_NextLibrary) {
					state->m_NextLibrary->m_PrevLibrary = state->m_PrevLibrary;
				}
				if (g_LastLibrary == state) {
					g_LastLibrary = state->m_PrevLibrary;
				}

				state->Unload(g_ModuleMgr);

				delete state;
			}
		}

		rpm::init::ModuleAllocation ReadLibrary(const char* path, rpm::mgr::ModuleManager* mgr) {
			size_t size;

			#ifdef GFL
			FSFile file;
			romfs_fopen(&file, path);

			size = romfs_fgetsize(&file);

			#elif defined(_WIN32)
			FILE* file = fopen(path, "rb");
			fseek(file, 0, SEEK_END);
			
			size = ftell(file);

			#endif

			if (size == 0) {
				return nullptr;
			}

			rpm::init::ModuleAllocation alloc = mgr->AllocModule(size);

			#ifdef GFL
			romfs_fseek(&file, 0, SeekOrigin::IO_SEEK_SET);
			romfs_fread(&file, alloc, size);
			romfs_fclose(&file);

			#elif defined(_WIN32)
			fseek(file, 0, SEEK_SET);
			fread(alloc, 1, size, file);
			fclose(file);
			
			#else
			#error Invalid platform!
			#endif

			return alloc;
		}
	}
}

#endif