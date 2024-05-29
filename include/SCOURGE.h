#pragma once

#define AV_COUNT 12

struct AV_KW_STRING_LIST
{
	RE::ActorValueInfo* actorValue;
	RE::BGSKeyword* exclusionKeyword;
	std::string actorValueID;
};

class PluginData
{
public:
	void LoadPluginData()
	{
		RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();

		if ((spellForm = dataHandler->LookupForm(0x1, "SCOURGE.esp")) != nullptr) {
			logger::warn("Loaded SCOURGE_SPEL");
		} else {
			logger::warn("Unable to load SCOURGE_SPEL");
			loadedCorrectly = false;
		}

		GetActorValues(dataHandler);
		GetKeywords(dataHandler);
		GetGlobalMults(dataHandler);
		LinkPluginRecords();
	}

	int RobCoPatcherCheck(CSimpleIniA& a_ini)
	{
		a_ini.LoadFile("Data\\F4SE\\Plugins\\RobCo_Patcher.ini");
		int result = std::stoi(a_ini.GetValue("Features", "iEnableReCalculateStatsWithSaveLoad", "0"));
		a_ini.Reset();
		return result;
	}

	std::vector<AV_KW_STRING_LIST>	 pluginRecords;
	std::vector<RE::TESGlobal*>		 globalStatMults;
	RE::ActorValueInfo*				 av_SCOURGE;
	RE::BGSKeyword*					 kw_All;
	RE::TESForm*					 spellForm;
	bool							 loadedCorrectly = true;

private:
	// grabs the ActorValues used in SCOURGE
	void GetActorValues(RE::TESDataHandler* a_dataHandler)
	{
		RE::ActorValue* actorValue = RE::ActorValue::GetSingleton();

		av_SCOURGE		  = a_dataHandler->LookupForm<RE::ActorValueInfo>(0x3, "SCOURGE.esp");
		av_DamageResist	  = actorValue->damageResistance;
		av_ElectricResist = actorValue->electricalResistance;
		av_EnergyResist	  = actorValue->energyResistance;
		av_FireResist	  = actorValue->fireResistance;
		av_FrostResist	  = actorValue->frostResistance;
		av_Health		  = actorValue->health;
		av_PoisonResist   = actorValue->poisonResistance;
		av_RadResist	  = actorValue->radExposureResistance;
		av_RangedDmg	  = a_dataHandler->LookupForm<RE::ActorValueInfo>(0x1504FB, "Fallout4.esm");  // why is it not inside ActorValue wtf is this???
		av_SpeedMult	  = actorValue->speedMult;
		av_UnarmedDamage  = actorValue->unarmedDamage;

		if (av_SCOURGE && av_DamageResist && av_ElectricResist && av_EnergyResist
			&& av_FireResist && av_FrostResist && av_Health && av_PoisonResist &&
			av_RadResist && av_RangedDmg && av_SpeedMult && av_UnarmedDamage)
		{
			logger::warn("Loaded ActorValues");
		} else {
			logger::warn("Unable to load ActorValues");
			loadedCorrectly = false;
		}
	}

	// grabs the exclusion Keywords used in SCOURGE
	void GetKeywords(RE::TESDataHandler* a_dataHandler)
	{
		kw_All			  = a_dataHandler->LookupForm<RE::BGSKeyword>(0x101, "SCOURGE.esp");
		kw_DamageResist	  = a_dataHandler->LookupForm<RE::BGSKeyword>(0x101, "SCOURGE.esp");
		kw_ElectricResist = a_dataHandler->LookupForm<RE::BGSKeyword>(0x102, "SCOURGE.esp");
		kw_EnergyResist   = a_dataHandler->LookupForm<RE::BGSKeyword>(0x103, "SCOURGE.esp");
		kw_FireResist	  = a_dataHandler->LookupForm<RE::BGSKeyword>(0x104, "SCOURGE.esp");
		kw_FrostResist	  = a_dataHandler->LookupForm<RE::BGSKeyword>(0x105, "SCOURGE.esp");
		kw_Health		  = a_dataHandler->LookupForm<RE::BGSKeyword>(0x106, "SCOURGE.esp");
		kw_PoisonResist	  = a_dataHandler->LookupForm<RE::BGSKeyword>(0x107, "SCOURGE.esp");
		kw_RadResist	  = a_dataHandler->LookupForm<RE::BGSKeyword>(0x108, "SCOURGE.esp");
		kw_RangedDmg	  = a_dataHandler->LookupForm<RE::BGSKeyword>(0x109, "SCOURGE.esp");
		kw_Scale		  = a_dataHandler->LookupForm<RE::BGSKeyword>(0x10A, "SCOURGE.esp");
		kw_SpeedMult	  = a_dataHandler->LookupForm<RE::BGSKeyword>(0x10B, "SCOURGE.esp");
		kw_UnarmedDamage  = a_dataHandler->LookupForm<RE::BGSKeyword>(0x10C, "SCOURGE.esp");

		if (kw_All && kw_DamageResist && kw_ElectricResist && kw_EnergyResist
			&& kw_FireResist && kw_FrostResist && kw_Health && kw_PoisonResist
			&& kw_RadResist && kw_RangedDmg && kw_Scale && kw_SpeedMult && kw_UnarmedDamage)
		{
			logger::warn("Loaded Keywords");
		} else {
			logger::warn("Unable to load Keywords");
			loadedCorrectly = false;
		}
	}

	// grabs the Globals used 
	void GetGlobalMults(RE::TESDataHandler* a_dataHandler) {
		glob_DamageResist_Mult	 = a_dataHandler->LookupForm<RE::TESGlobal>(0x4, "SCOURGE.esp");
		glob_ElectricResist_Mult = a_dataHandler->LookupForm<RE::TESGlobal>(0x5, "SCOURGE.esp");
		glob_EnergyResist_Mult	 = a_dataHandler->LookupForm<RE::TESGlobal>(0x6, "SCOURGE.esp");
		glob_FireResist_Mult	 = a_dataHandler->LookupForm<RE::TESGlobal>(0x7, "SCOURGE.esp");
		glob_FrostResist_Mult	 = a_dataHandler->LookupForm<RE::TESGlobal>(0x8, "SCOURGE.esp");
		glob_Health_Mult		 = a_dataHandler->LookupForm<RE::TESGlobal>(0x9, "SCOURGE.esp");
		glob_PoisonResist_Mult	 = a_dataHandler->LookupForm<RE::TESGlobal>(0xA, "SCOURGE.esp");
		glob_RadResist_Mult		 = a_dataHandler->LookupForm<RE::TESGlobal>(0xB, "SCOURGE.esp");
		glob_RangedDmg_Mult		 = a_dataHandler->LookupForm<RE::TESGlobal>(0xC, "SCOURGE.esp");
		glob_Scale_Mult			 = a_dataHandler->LookupForm<RE::TESGlobal>(0xD, "SCOURGE.esp");
		glob_SpeedMult_Mult		 = a_dataHandler->LookupForm<RE::TESGlobal>(0xE, "SCOURGE.esp");
		glob_UnarmedDamage_Mult  = a_dataHandler->LookupForm<RE::TESGlobal>(0xF, "SCOURGE.esp");

		globalStatMults.push_back(glob_Scale_Mult);
		globalStatMults.push_back(glob_Health_Mult);
		globalStatMults.push_back(glob_RangedDmg_Mult);
		globalStatMults.push_back(glob_SpeedMult_Mult);
		globalStatMults.push_back(glob_UnarmedDamage_Mult);
		globalStatMults.push_back(glob_DamageResist_Mult);
		globalStatMults.push_back(glob_ElectricResist_Mult);
		globalStatMults.push_back(glob_EnergyResist_Mult);
		globalStatMults.push_back(glob_FireResist_Mult);
		globalStatMults.push_back(glob_FrostResist_Mult);
		globalStatMults.push_back(glob_PoisonResist_Mult);
		globalStatMults.push_back(glob_RadResist_Mult);

		if (glob_DamageResist_Mult && glob_ElectricResist_Mult && glob_EnergyResist_Mult
			&& glob_FireResist_Mult && glob_FrostResist_Mult && glob_Health_Mult
			&& glob_PoisonResist_Mult && glob_RadResist_Mult && glob_RangedDmg_Mult
			&& glob_Scale_Mult && glob_SpeedMult_Mult && glob_UnarmedDamage_Mult)
		{
			logger::warn("Loaded global mults");
		} else {
			logger::warn("Unable to load global mults");
			loadedCorrectly = false;
		}
	}

	// links ActorValues to the respective exclusion Keywords and the String used for TESGlobal construction
	void LinkPluginRecords()
	{
		// not an AV
		pluginRecords.emplace_back(nullptr, kw_Scale, "Scale");
		// SetValue()
		pluginRecords.emplace_back(av_Health, kw_Health, "Health");
		pluginRecords.emplace_back(av_RangedDmg, kw_RangedDmg, "RangedDmg");
		pluginRecords.emplace_back(av_SpeedMult, kw_SpeedMult, "SpeedMult");
		pluginRecords.emplace_back(av_UnarmedDamage, kw_UnarmedDamage, "UnarmedDamage");
		// ModValue()
		pluginRecords.emplace_back(av_DamageResist, kw_DamageResist, "DamageResist");
		pluginRecords.emplace_back(av_ElectricResist, kw_ElectricResist, "ElectricResist");
		pluginRecords.emplace_back(av_EnergyResist, kw_EnergyResist, "EnergyResist");
		pluginRecords.emplace_back(av_FireResist, kw_FireResist, "FireResist");
		pluginRecords.emplace_back(av_FrostResist, kw_FrostResist, "FrostResist");
		pluginRecords.emplace_back(av_PoisonResist, kw_PoisonResist, "PoisonResist");
		pluginRecords.emplace_back(av_RadResist, kw_RadResist, "RadResist");

		if (pluginRecords.size() != AV_COUNT) {
			logger::warn("!!! AV_COUNT does not match the size of pluginRecords !!!");
		}

		logger::warn("Linked plugin records");
	}

	RE::ActorValueInfo* av_DamageResist;
	RE::ActorValueInfo* av_ElectricResist;
	RE::ActorValueInfo* av_EnergyResist;
	RE::ActorValueInfo* av_FireResist;
	RE::ActorValueInfo* av_FrostResist;
	RE::ActorValueInfo* av_Health;
	RE::ActorValueInfo* av_PoisonResist;
	RE::ActorValueInfo* av_RadResist;
	RE::ActorValueInfo* av_RangedDmg;
	RE::ActorValueInfo* av_SpeedMult;
	RE::ActorValueInfo* av_UnarmedDamage;

	RE::BGSKeyword* kw_DamageResist;
	RE::BGSKeyword* kw_ElectricResist;
	RE::BGSKeyword* kw_EnergyResist;
	RE::BGSKeyword* kw_FireResist;
	RE::BGSKeyword* kw_FrostResist;
	RE::BGSKeyword* kw_Health;
	RE::BGSKeyword* kw_PoisonResist;
	RE::BGSKeyword* kw_RadResist;
	RE::BGSKeyword* kw_RangedDmg;
	RE::BGSKeyword* kw_Scale;
	RE::BGSKeyword* kw_SpeedMult;
	RE::BGSKeyword* kw_UnarmedDamage;

	RE::TESGlobal* glob_DamageResist_Mult;
	RE::TESGlobal* glob_ElectricResist_Mult;
	RE::TESGlobal* glob_EnergyResist_Mult;
	RE::TESGlobal* glob_FireResist_Mult;
	RE::TESGlobal* glob_FrostResist_Mult;
	RE::TESGlobal* glob_Health_Mult;
	RE::TESGlobal* glob_PoisonResist_Mult;
	RE::TESGlobal* glob_RadResist_Mult;
	RE::TESGlobal* glob_RangedDmg_Mult;
	RE::TESGlobal* glob_Scale_Mult;
	RE::TESGlobal* glob_SpeedMult_Mult;
	RE::TESGlobal* glob_UnarmedDamage_Mult;
};
