#pragma once

/*
This file is taken from shad0wshayd3's GPLv3 project BakaFramework

https://github.com/shad0wshayd3

https://github.com/shad0wshayd3/BakaFramework

Comments have been placed where changes have been made.

*/

namespace Patches
{
	class LoadEditorIDs
	{
	private:
		template <class Form>
		static void InstallHook()
		{
			REL::Relocation<std::uintptr_t> vtbl{ Form::VTABLE[0] };
			_GetFormEditorID = vtbl.write_vfunc(0x3A, GetFormEditorID);
			_SetFormEditorID = vtbl.write_vfunc(0x3B, SetFormEditorID);
		}

		static const char* GetFormEditorID(RE::TESForm* a_this)
		{
			auto it = rmap.find(a_this->formID);
			if (it != rmap.end()) {
				return it->second.c_str();
			}

			return _GetFormEditorID(a_this);
		}

		static bool SetFormEditorID(RE::TESForm* a_this, const char* a_editor)
		{
			auto edid = std::string_view{ a_editor };
			if (a_this->formID < 0xFF000000 && !edid.empty()) {
				AddToGameMap(a_this, a_editor);
			}

			return _SetFormEditorID(a_this, a_editor);
		}

	private:
		static void AddToGameMap(RE::TESForm* a_this, const char* a_editorID)
		{
			const auto& [map, lock] = RE::TESForm::GetAllFormsByEditorID();
			const RE::BSAutoWriteLock locker{ lock.get() };
			if (map) {
				map->emplace(a_editorID, a_this);
				rmap.emplace(a_this->formID, a_editorID);
			}
		}

		inline static std::unordered_map<std::uint32_t, std::string> rmap;

		inline static REL::Relocation<decltype(&RE::TESForm::SetFormEditorID)> _SetFormEditorID;
		inline static REL::Relocation<decltype(&RE::TESForm::GetFormEditorID)> _GetFormEditorID;

	public:
		static void Install()
		{
			// Changed by GELUXRUM: Modified function to only load the EditorIDs of Form types used by SCOURGE by removing other EditorID loaders.

			// InstallHook<RE::BGSKeyword>();
			// InstallHook<RE::TESGlobal>();
			InstallHook<RE::TESFaction>();
			// InstallHook<RE::TESRace>();
			InstallHook<RE::TESNPC>();

			logger::debug("Loaded EdtorIDs"sv);
		}
	};
}
