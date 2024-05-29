#include "PapyrusManager.h"
#include "SCOURGE.h"

namespace Utils
{
	RE::TESNPC* GetBaseNPC(RE::Actor* a_actor)
	{
		if (!a_actor) {
			return nullptr;
		}

		RE::BSExtraData* extraLvlCreature = a_actor->extraList->GetByType(RE::EXTRA_DATA_TYPE::kLeveledCreature);
		if (extraLvlCreature) {
			return *(RE::TESNPC**)((uintptr_t)extraLvlCreature + 0x18);
		}

		return a_actor->GetNPC();
	}

	bool IsValidNPC(RE::Actor* a_actor)
	{
		// CTD lovers hate this one simple trick
		if (!a_actor) {
			return false;
		}

		// skip the NPC if it has the Exclude_All keyword
		// skip the NPC if the SCOURGE_Tracker ActorValue isn't 0
		if (a_actor->HasKeyword(pluginData.kw_All) == true || a_actor->GetActorValue(*pluginData.av_SCOURGE) != 0.0f) {
			return false;
		}

		return true;
	}
}

namespace Papyrus
{
	// populate a_array with pointers to Globals from filters
	int BuildValueList(RE::Actor* a_actor, std::array<std::pair<ValueData, float>, AV_COUNT>& a_arrayToPopulate) {
		int lastFilter{ 0 };

		// see if the NPC matches the filter conditions
		for (auto filter : configData.filters) {
			bool skipFilter{ false };
			
			// NPC forms
			if (filter.npcList.empty() == false) {
				//logger::warn("npc form id: {}", Utils::GetBaseNPC(a_actor)->formID);
				if (filter.npcList.find(Utils::GetBaseNPC(a_actor)->formID) != filter.npcList.end()) {
					goto RECALCULATION;
				}

				// if the other lists are empty we do an early continue to prevent all NPCs from being
				// included in the recalculation as there are no conditions for them to meet
				if (filter.raceList.empty() && filter.factionList.empty() && filter.keywordList.empty()) {
					continue;
				}
			}

			if (filter.raceList.empty() == false) {
				// if the NPC's Race isn't listed we skip this filter
				if (filter.raceList.find(a_actor->race) == filter.raceList.end()) {
					continue;
				}
			}

			if (filter.factionList.empty() == false) {
				for (RE::TESFaction* faction : filter.factionList) {
					if (a_actor->IsInFaction(faction) == false) {
						skipFilter = true;
						break;
					}
				}
				if (skipFilter) {
					continue;
				}
			}

			if (filter.keywordList.empty() == false) {
				for (RE::BGSKeyword* keyword : filter.keywordList) {
					if (a_actor->HasKeyword(keyword) == false) {
						skipFilter = true;
						break;
					}
				}
				if (skipFilter) {
					continue;
				}
			}

			if (Utils::GetBaseNPC(a_actor)->IsFemale()) {
				if (filter.gender.female == false) {
					continue;
				}
			} else {
				if (filter.gender.male == false) {
					continue;
				}
			}

		RECALCULATION:

			// an NPC will have a single recalculation applied to them based on the combined values
			// of all filters that it passed
			const auto iterator = configData.valueMap.find(filter.globalString);

			// populate a_array's Globals with the ones from this filter. If any are nullptr then they
			// are most likely purposefully left out to create a partial lists of Globals that only
			// affect a single stat
			for (int i = 0; i < pluginData.pluginRecords.size(); i++) {
				if (iterator->second[i].enableGlobal.first) {
					a_arrayToPopulate[i].first.enableGlobal.first = iterator->second[i].enableGlobal.first;
				}

				if (iterator->second[i].minGlobal.first) {
					a_arrayToPopulate[i].first.minGlobal.first = iterator->second[i].minGlobal.first;
				}

				if (iterator->second[i].maxGlobal.first) {
					a_arrayToPopulate[i].first.maxGlobal.first = iterator->second[i].maxGlobal.first;
				}

				if (iterator->second[i].biasGlobal.first) {
					a_arrayToPopulate[i].first.biasGlobal.first = iterator->second[i].biasGlobal.first;
				}

				if (iterator->second[i].influenceGlobal.first) {
					a_arrayToPopulate[i].first.influenceGlobal.first = iterator->second[i].influenceGlobal.first;
				}

				a_arrayToPopulate[i].second = filter.mults.find(pluginData.pluginRecords[i].actorValueID)->second;
			}

			lastFilter = filter.filterID;
		}

		return lastFilter;
	}

	void RecalculateStats(RE::Actor* a_actor, std::array<std::pair<ValueData, float>, AV_COUNT>& a_statArray, int& a_trackerValue)
	{
		if (a_statArray[0].first.enableGlobal.first) {
			if (a_statArray[0].first.enableGlobal.first->GetValue() == 1.0f) {
				if (a_actor->HasKeyword(pluginData.pluginRecords[0].exclusionKeyword) == false) {
					if (a_statArray[0].first.minGlobal.first && a_statArray[0].first.maxGlobal.first) {
						float bias = 0.5f;

						if (a_statArray[0].first.biasGlobal.first != nullptr) {
							// if the user is stupid and forced a value outside of the MCM range then that's their problem
							bias = a_statArray[0].first.biasGlobal.first->GetValue();
						}

						float randomValue = mathUtils.DoGaussian(a_statArray[0].first.minGlobal.first->GetValue(), a_statArray[0].first.maxGlobal.first->GetValue(), bias);
						float newScale = a_statArray[0].second * randomValue * pluginData.globalStatMults[0]->GetValue();
						RE::TESObjectREFR* actorObject = a_actor->GetHandle().get().get();
						if (actorObject) {
							PapyrusCaller::CallPapyrusFunctionOnForm(actorObject, "ObjectReference", "SetScale", newScale);
						}
					}
				}
			}
		}

		for (int i = 1; i < a_statArray.size(); i++) {
			// check if the _Enable Global exists and isn't disabled
			if (a_statArray[i].first.enableGlobal.first == nullptr || a_statArray[i].first.enableGlobal.first->GetValue() == 0.0f) {
				continue;
			}
			// check if the NPC has the exclusion keyword
			if (a_actor->HasKeyword(pluginData.pluginRecords[i].exclusionKeyword)) {
				continue;
			}
			// make sure that both the _Min and _Max Globals exist
			if (a_statArray[i].first.minGlobal.first == nullptr || a_statArray[i].first.maxGlobal.first == nullptr) {
				continue;
			}

			float bias = 0.5f;

			if (a_statArray[i].first.biasGlobal.first != nullptr) {
				bias = a_statArray[i].first.biasGlobal.first->GetValue();
			}

			// calculate the new randomised value of the AV using the Globals
			float randomisedValue = a_statArray[i].second * mathUtils.DoGaussian(a_statArray[i].first.minGlobal.first->GetValue(), a_statArray[i].first.maxGlobal.first->GetValue(), bias) * pluginData.globalStatMults[i]->GetValue();

			if (i < 5) {
				a_actor->SetActorValue(*pluginData.pluginRecords[i].actorValue, randomisedValue);
			} else {
				// armor resistances are baked into BaseValue so we need to take away the value that's inside the NPC record
				// also can't use SetValue :todd:
				float armorValue = a_actor->GetBaseActorValue(*pluginData.pluginRecords[i].actorValue) - a_actor->data.objectReference->As<RE::TESNPC>()->GetActorValue(*pluginData.pluginRecords[i].actorValue);

				float trashValue = a_actor->GetActorValue(*pluginData.pluginRecords[i].actorValue) - armorValue;

				a_actor->ModActorValue(RE::ACTOR_VALUE_MODIFIER::Perm, *pluginData.pluginRecords[i].actorValue, randomisedValue - trashValue);
			}

			a_actor->SetActorValue(*pluginData.av_SCOURGE, static_cast<float>(a_trackerValue));
		}
	}

	void SCOURGE(std::monostate, RE::Actor* a_actor)
	{
		if (Utils::IsValidNPC(a_actor) == false) {
			return;
		}

		std::array<std::pair<ValueData, float>, AV_COUNT> globalsToUse;

		// the return value of BuildValueList will be used for the tracker AV
		int lastAppliedFilter = BuildValueList(a_actor, globalsToUse);

		if (lastAppliedFilter != 0) {
			RecalculateStats(a_actor, globalsToUse, lastAppliedFilter);
		}
	}

	void SCOURGE_Recalculate_Actor(std::monostate, RE::Actor* a_actor)
	{
		if (!a_actor) {
			return;
		}

		std::array<std::pair<ValueData, float>, AV_COUNT> globalsToUse;

		// the return value of BuildValueList will be used for the tracker AV
		int lastAppliedFilter = BuildValueList(a_actor, globalsToUse);

		if (lastAppliedFilter != 0) {
			RecalculateStats(a_actor, globalsToUse, lastAppliedFilter);
		}
	}

	void SCOURGE_ResetGlobals(std::monostate)
	{
		for (auto valueStructs : configData.valueMap) {
			for (auto globals : valueStructs.second) {
				if (globals.enableGlobal.first) {
					globals.enableGlobal.first->value = globals.enableGlobal.second;
				}

				if (globals.minGlobal.first) {
					globals.minGlobal.first->value = globals.minGlobal.second;
				}

				if (globals.maxGlobal.first) {
					globals.maxGlobal.first->value = globals.maxGlobal.second;
				}

				if (globals.biasGlobal.first) {
					globals.biasGlobal.first->value = globals.biasGlobal.second;
				}

				if (globals.influenceGlobal.first) {
					globals.influenceGlobal.first->value = globals.influenceGlobal.second;
				}
			}
		}
	}

	void SCOURGE_ResetGlobalsByName(std::monostate, std::string a_string) {
		auto iterator = configData.valueMap.find(a_string);
		if (iterator != configData.valueMap.end()) {
			for (auto globals : iterator->second) {
				if (globals.enableGlobal.first) {
					globals.enableGlobal.first->value = globals.enableGlobal.second;
				}

				if (globals.minGlobal.first) {
					globals.minGlobal.first->value = globals.minGlobal.second;
				}

				if (globals.maxGlobal.first) {
					globals.maxGlobal.first->value = globals.maxGlobal.second;
				}

				if (globals.biasGlobal.first) {
					globals.biasGlobal.first->value = globals.biasGlobal.second;
				}

				if (globals.influenceGlobal.first) {
					globals.influenceGlobal.first->value = globals.influenceGlobal.second;
				}
			}
		}
	}

	void SCOURGE_Recalculate_Global(std::monostate)
	{
		const auto processLists = RE::ProcessLists::GetSingleton();

		if (!processLists) {
			return;
		}

		for (const auto& actorHandle : processLists->highActorHandles) {
			const auto actorPtr = actorHandle.get();
			const auto currentActor = actorPtr.get();

			if (!currentActor) {
				continue;
			}

			if (currentActor->IsDead(true)) {
				continue;
			}

			SCOURGE_Recalculate_Actor(std::monostate{}, currentActor);
		}

		return;
	}

	bool BindFunctions(RE::BSScript::IVirtualMachine* a_vm)
	{
		a_vm->BindNativeMethod("SCOURGE_Native", "SCOURGE", SCOURGE, true);
		a_vm->BindNativeMethod("SCOURGE_Native", "SCOURGE_Recalculate_Actor", SCOURGE_Recalculate_Actor, true);
		a_vm->BindNativeMethod("SCOURGE_Native", "SCOURGE_Recalculate_Global", SCOURGE_Recalculate_Global, true);
		a_vm->BindNativeMethod("SCOURGE_Native", "SCOURGE_ResetGlobals", SCOURGE_ResetGlobals, true);
		a_vm->BindNativeMethod("SCOURGE_Native", "SCOURGE_ResetGlobalsByName", SCOURGE_ResetGlobalsByName, true);

		return true;
	}
}
