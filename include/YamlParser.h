#pragma once

#define fuck_around try
#define find_out catch
#define YAML_STRING(node) ((node) ? (node).as<std::string>() : "")
#define YAML_FLOAT(node) ((node) ? (node).as<float>() : 1.0f)

#include <yaml-cpp/yaml.h>

extern PluginData pluginData;

struct Gender
{
	bool male;
	bool female;
};

struct FilterData
{
	std::uint16_t filterID;
	std::string filterName;
	std::string globalString;
	std::unordered_set<std::uint32_t> npcList;
	std::unordered_set<RE::TESRace*> raceList;
	std::unordered_set<RE::TESFaction*> factionList;
	std::unordered_set<RE::BGSKeyword*> keywordList;
	Gender gender;
	std::unordered_map<std::string, float> mults;
};

struct ValueData
{
	// float is the default value obtained when the Form is loaded
	std::pair<RE::TESGlobal*, float> enableGlobal;
	std::pair<RE::TESGlobal*, float> minGlobal;
	std::pair<RE::TESGlobal*, float> maxGlobal;
	std::pair<RE::TESGlobal*, float> biasGlobal;
	std::pair<RE::TESGlobal*, float> influenceGlobal;
};

class ModConfigs
{
public:
	void LoadConfigs() {
		//RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
		auto fileList = GetFiles();

		if (fileList.empty()) {
			logger::warn("No YAML files detected");
			return;
		} else {
			if (fileList.size() != 1) {
				logger::warn("Found {} files", fileList.size());
			} else {
				logger::warn("Found {} file", fileList.size());
			}
			logger::warn("---------------------------------------------------------------------");
		}

		uint16_t filterCount{ 0 };

		for (std::string currentFile : fileList) {
			YAML::Node yamlConfig;

			fuck_around {
				yamlConfig = YAML::LoadFile(currentFile);
				logger::warn("Loaded file: \"{}\"", currentFile);
			} find_out (const YAML::Exception& except) {
				logger::error("Error loading YAML file: {}", except.what());
				continue;
			}

			if (yamlConfig.IsNull()) {
				logger::error("The file is empty: \"{}\"", currentFile);
				continue;
			}

			logger::warn("---------------------------------------------------------------------");

			for (const auto& node : yamlConfig) {
				FilterData filterStruct;

				filterStruct.filterName = YAML_STRING(node.first);

				filterStruct.globalString = YAML_STRING(node.second["GlobalVariables"]["String"]);

				// check if the list of Globals for this string has already been constructed
				if (valueMap.find(filterStruct.globalString) == valueMap.end()) {
					std::vector<ValueData> valueArray;

					for (AV_KW_STRING_LIST record : pluginData.pluginRecords) {
						ValueData valueStruct{};
						std::string globalID = "SCOURGE_" + filterStruct.globalString + "_" + record.actorValueID;

						valueStruct.enableGlobal.first = RE::TESForm::GetFormByEditorID<RE::TESGlobal>(globalID + "_Enable");
						if (valueStruct.enableGlobal.first) {
							valueStruct.enableGlobal.second = valueStruct.enableGlobal.first->GetValue();
						} else {
							valueStruct.enableGlobal.second = 0.0f;
						}

						valueStruct.minGlobal.first = RE::TESForm::GetFormByEditorID<RE::TESGlobal>(globalID + "_Min");
						valueStruct.minGlobal.second = valueStruct.minGlobal.first->GetValue();
						if (valueStruct.minGlobal.first) {
							valueStruct.minGlobal.second = valueStruct.minGlobal.first->GetValue();
						} else {
							valueStruct.minGlobal.second = 0.0f;
						}

						valueStruct.maxGlobal.first = RE::TESForm::GetFormByEditorID<RE::TESGlobal>(globalID + "_Max");
						valueStruct.maxGlobal.second = valueStruct.maxGlobal.first->GetValue();
						if (valueStruct.maxGlobal.first) {
							valueStruct.maxGlobal.second = valueStruct.maxGlobal.first->GetValue();
						} else {
							valueStruct.maxGlobal.second = 0.0f;
						}

						valueStruct.biasGlobal.first = RE::TESForm::GetFormByEditorID<RE::TESGlobal>(globalID + "_Bias");
						valueStruct.biasGlobal.second = valueStruct.biasGlobal.first->GetValue();
						if (valueStruct.biasGlobal.first) {
							valueStruct.biasGlobal.second = valueStruct.biasGlobal.first->GetValue();
						} else {
							valueStruct.biasGlobal.second = 0.0f;
						}

						valueStruct.influenceGlobal.first = RE::TESForm::GetFormByEditorID<RE::TESGlobal>(globalID + "_Influence");
						valueStruct.influenceGlobal.second = valueStruct.influenceGlobal.first->GetValue();
						if (valueStruct.influenceGlobal.first) {
							valueStruct.influenceGlobal.second = valueStruct.influenceGlobal.first->GetValue();
						} else {
							valueStruct.influenceGlobal.second = 0.0f;
						}

						valueArray.push_back(valueStruct);
					}
					valueMap.emplace(filterStruct.globalString, valueArray);
				}
				
				for (AV_KW_STRING_LIST actorValues : pluginData.pluginRecords) {
					if (node.second["ActorValues"] && node.second["ActorValues"][actorValues.actorValueID]) {
						std::pair<std::string, float> multInfo;
						const YAML::Node& avNode = node.second["ActorValues"][actorValues.actorValueID];
						float valueMult = YAML_FLOAT(avNode["Mult"]);
						filterStruct.mults.emplace(actorValues.actorValueID, valueMult);
					} else {
						filterStruct.mults.emplace(actorValues.actorValueID, 1.0f);
					}
				}

				if (node.second["Forms"]) {
					PopulateLists<std::uint32_t>(node.second["Forms"], filterStruct);
				}

				if (node.second["Races"]) {
					PopulateLists<RE::TESRace>(node.second["Races"], filterStruct);
				}

				if (node.second["Factions"]) {
					PopulateLists<RE::TESFaction>(node.second["Factions"], filterStruct);
				}

				if (node.second["Keywords"]) {
					PopulateLists<RE::BGSKeyword>(node.second["Keywords"], filterStruct);
				}

				if (node.second["Gender"]) {
					GetGender(node.second["Gender"], filterStruct);
				} else {
					filterStruct.gender.male = true;
					filterStruct.gender.female = true;
				}

				filterCount++;
				filterStruct.filterID = filterCount;
				
				logger::warn("{}: \"{}\" from \"{}\"", filterCount, filterStruct.filterName, currentFile);
				logger::warn("Global string:");
				logger::warn("   {}", filterStruct.globalString);
				
				if (filterStruct.npcList.empty() && filterStruct.raceList.empty() && filterStruct.factionList.empty() && filterStruct.keywordList.empty()) {  //&& filterStruct.gender == NPC_GENDER::any) {
					logger::warn("The non-gender conditions inside the filter {} are invalid or empty. Make sure that all Forms or EditorIDs are correct. If you're trying to distribute to all NPCs use the keyword ActorTypeNPC~13794~Fallout4.esm", filterStruct.filterName);
					continue;
				}

				filters.push_back(filterStruct);

				logger::warn("NPC Forms:");
				for (auto item : filterStruct.npcList) {
					logger::warn("   {}", uint32ToHexString(item));
				}

				logger::warn("Races:");
				for (auto item : filterStruct.raceList) {
					logger::warn("   {}", item->GetFormEditorID());
				}

				logger::warn("Factions:");
				for (auto item : filterStruct.factionList) {
					logger::warn("   {}", item->As<RE::TESForm>()->GetFormEditorID());
				}

				logger::warn("Keywords:");
				for (auto item : filterStruct.keywordList) {
					logger::warn("   {}", item->GetFormEditorID());
				}

				logger::warn("Gender:");
				logger::warn("   Male: {}", filterStruct.gender.male);
				logger::warn("   Female: {}", filterStruct.gender.female);

				logger::warn("---------------------------------------------------------------------");
			}
		}

		logger::warn("Finished reading files");
		logger::warn("---------------------------------------------------------------------");
	}

	std::unordered_map<std::string, std::vector<ValueData>> valueMap;
	std::vector<FilterData> filters;

private:
	// trim leading and trailing spaces
	std::string TrimSpaces(const std::string& a_input)
	{
		std::size_t firstNonSpace = a_input.find_first_not_of(' ');
		std::size_t lastNonSpace = a_input.find_last_not_of(' ');

		if (firstNonSpace == std::string::npos || lastNonSpace == std::string::npos) {
			// the string is either empty or contains only spaces
			return "";
		}

		return a_input.substr(firstNonSpace, lastNonSpace - firstNonSpace + 1);
	}

	// convert a string to lowercase
	std::string ToLower(std::string a_str)
	{
		std::string lowercaseStr;

		size_t length = std::strlen(a_str.c_str());

		for (size_t i = 0; i < length; ++i) {
			lowercaseStr += static_cast<char>(std::tolower(static_cast<unsigned char>(a_str.c_str()[i])));
		}

		return lowercaseStr;
	}

	// split a string into a pair using a delimiter
	std::pair<std::string, std::string> SplitString(const std::string& a_input, char a_delimiter)
	{
		size_t found = a_input.find(a_delimiter);

		if (found != std::string::npos) {
			return std::make_pair(a_input.substr(0, found), a_input.substr(found + 1));
		} else {
			// Return an empty string for the second part if the delimiter is not found
			return std::make_pair(a_input, "");
		}
	}

	// load all YAML files that are used by this plugin
	std::vector<std::string> GetFiles()
	{
		std::vector<std::string> result;

		// path to SCOURGE directory where the YAML config files are stored
		auto constexpr directory = R"(Data\SCOURGE\)";
		// SCOURGE directory contents
		for (const auto& entry : std::filesystem::directory_iterator(directory)) {
			// only look for YAML files
			if (entry.exists() && !entry.path().empty()) {
				std::string extension = entry.path().extension().string();
				extension = ToLower(extension);
				if (extension == ".yaml") {
					const auto path = entry.path().string();
					result.push_back(path);
				}
			}
		}

		return result;
	}

	// in case the FormID includes the LO index
	std::string TruncateFormID(std::string a_formID) {
		std::string result = a_formID;

		// just in case they use the hex prefix
		// decimal FormIDs aren't supported because
		// there's no reason to
		if (result.substr(0, 2) == "0x") {
			result.erase(0, 2);
		}

		if (a_formID.length() > 6) {
			result = a_formID.substr(a_formID.length() - 6);
		}

		return result;
	}

	std::string uint32ToHexString(std::uint32_t value)
	{
		std::stringstream stream;
		stream << std::hex << std::setw(8) << std::setfill('0') << value;
		return "0x" + stream.str();  // Prefix "0x" to indicate hexadecimal
	}

	template <typename T>
	void AddToList(T* a_type, FilterData& a_filterStruct){};

	template <>
	void AddToList<RE::TESRace>(RE::TESRace* a_race, FilterData& a_filterStruct)
	{
		a_filterStruct.raceList.insert(a_race);
	}

	template <>
	void AddToList<RE::TESFaction>(RE::TESFaction* a_faction, FilterData& a_filterStruct)
	{
		a_filterStruct.factionList.insert(a_faction);
	}

	template <>
	void AddToList<RE::BGSKeyword>(RE::BGSKeyword* a_keyword, FilterData& a_filterStruct)
	{
		a_filterStruct.keywordList.insert(a_keyword);
	}

	template<typename T>
	void PopulateLists(const YAML::Node& a_parent, FilterData& a_filterStruct)
	{
		if (a_parent.IsNull() == false) {
			RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
			for (const auto& child : a_parent) {
				std::string editorID = YAML_STRING(child["EditorID"]);
				std::string formID = YAML_STRING(child["FormID"]);
				std::string pluginName = YAML_STRING(child["Plugin"]);

				T* toAdd = nullptr;
				/*
				if (ToLower(editorID) != "null") {
					toAdd = RE::TESForm::GetFormByEditorID<T>(editorID);
					if (toAdd) {
						AddToList<T>(toAdd, a_filterStruct);
						continue;
					}
				}
				*/

				// make sure that both a FormID and a plugin name is provided
				if (formID == "null" || pluginName == "null") {
					continue;
				}

				formID = TruncateFormID(formID);
				std::uint32_t adjustedFormID = std::stoul(formID, nullptr, 16);
				toAdd = dataHandler->LookupForm<T>(adjustedFormID, pluginName);

				if (toAdd) {
					AddToList<T>(toAdd, a_filterStruct);
				}
			}
		}
	}

	// instead of RE::TESForm* we will use the raw integer value of the FormID
	// I think I had issues in the past when comparing Forms directly, but FormID
	// comparisons worked perfectly fine
	template<>
	void PopulateLists<std::uint32_t>(const YAML::Node& a_parent, FilterData& a_filterStruct)
	{
		if (a_parent.IsNull() == false) {
			RE::TESDataHandler* dataHandler = RE::TESDataHandler::GetSingleton();
			for (const auto& child : a_parent) {
				std::string editorID = YAML_STRING(child["EditorID"]);
				std::string formID = YAML_STRING(child["FormID"]);
				std::string pluginName = YAML_STRING(child["Plugin"]);

				if (editorID.empty() == false) {
					RE::TESForm* formPtr = RE::TESForm::GetFormByEditorID(editorID);
					if (formPtr) {
						logger::warn("Loaded NPC by EditorID");
						a_filterStruct.npcList.insert(formPtr->formID);
						continue;
					}
				}

				if (ToLower(editorID) == "null" || ToLower(pluginName) == "null") {
					continue;
				}

				formID = TruncateFormID(formID);
				std::uint32_t adjustedFormID = std::stoul(formID, nullptr, 16);
				adjustedFormID = dataHandler->LookupFormID(adjustedFormID, pluginName);

				if (adjustedFormID != 0) {
					a_filterStruct.npcList.insert(adjustedFormID);
				}
			}
		}
	}

	void GetGender(const YAML::Node& a_parent, FilterData& a_filterStruct)
	{
		if (a_parent["Male"]) {
			std::string male = YAML_STRING(a_parent["Male"]);

			if (ToLower(male) == "true" || ToLower(male) == "t" || ToLower(male) == "1") {
				a_filterStruct.gender.male = true;
			} else if (ToLower(male) == "false" || ToLower(male) == "f" || ToLower(male) == "0") {
				a_filterStruct.gender.male = false;
			} else {
				logger::warn("{} is not a valid setting for Gender. Defaulting to false", male);
				a_filterStruct.gender.male = false;
			}
		}

		if (a_parent["Female"]) {
			std::string female = YAML_STRING(a_parent["Female"]);

			if (ToLower(female) == "true" || ToLower(female) == "t" || ToLower(female) == "1") {
				a_filterStruct.gender.female = true;
			} else if (ToLower(female) == "false" || ToLower(female) == "f" || ToLower(female) == "0") {
				a_filterStruct.gender.female = false;
			} else {
				logger::warn("{} is not a valid setting for Gender. Defaulting to false", female);
				a_filterStruct.gender.female = false;
			}
		}
	}
};
