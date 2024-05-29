#include "LoadEditorIDs.h"
#include "PapyrusManager.h"
#include "RNJesus.h"
#include "SCOURGE.h"
#include "YamlParser.h"

CSimpleIniA simpleIni(true, false, false);
ModConfigs configData;
PluginData pluginData;
RNJesus mathUtils;

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(const F4SE::QueryInterface* a_f4se, F4SE::PluginInfo* a_info)
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		return false;
	}

	*path /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::warn);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("--> %v"s);

	logger::info("SCOURGE v{}.{}.{} log:", Version::MAJOR, Version::MINOR, Version::PATCH);
	logger::info("---------------------------------------------------------------------");

	a_info->infoVersion = F4SE::PluginInfo::kVersion;
	a_info->name = "GLXRM_SCOURGE";
	a_info->version = Version::MAJOR;

	if (a_f4se->IsEditor()) {
		logger::critical("loaded in editor");
		return false;
	}

	const auto ver = a_f4se->RuntimeVersion();
	if (ver < F4SE::RUNTIME_1_10_162) {
		logger::critical("unsupported runtime v{}", ver.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se);
	
	// check if Baka Framework is installed
	if (F4SE::GetPluginInfo("BakaFramework").has_value()) {
		logger::warn("Baka Framework is installed");
	} else {
		logger::warn("Baka Framework is not installed. Loading EditorIDs");
		Patches::LoadEditorIDs::Install();
	}

	const F4SE::PapyrusInterface* papyrus = F4SE::GetPapyrusInterface();
	papyrus->Register(Papyrus::BindFunctions);

	const F4SE::MessagingInterface* messageInterface = F4SE::GetMessagingInterface();
	messageInterface->RegisterListener([](F4SE::MessagingInterface::Message* msg) -> void {
		if (msg->type == F4SE::MessagingInterface::kGameDataReady) {
			// this setting resets values, so if it's enabled we bother the user and close their game
			if (pluginData.RobCoPatcherCheck(simpleIni) != 0.0f) {
				std::string message = "WARNING!\n\nYou must disable iEnableReCalculateStatsWithSaveLoad inside Data\\F4SE\\Plugins\\RobCo_Patcher.ini before using SCOURGE to prevent complications.\n\nThe game will terminate once this message box is closed.";
#undef MessageBox
				F4SE::WinAPI::MessageBox(nullptr, message.c_str(), "SCOURGE", 0x00001000);
				std::abort();
			}

			// make my own mod a requirement for double the DP and CTD the game if they don't have it :V
			if (F4SE::GetPluginInfo("GLXRM_ScalingFlagRemover").has_value() == false) {
				std::string message = "WARNING!\n\nScaling Flag Remover is not installed!\nPlease download it before using SCOURGE: https://www.nexusmods.com/fallout4/mods/73976\n\nThe game will terminate once this message box is closed.";
#undef MessageBox
				F4SE::WinAPI::MessageBox(nullptr, message.c_str(), "SCOURGE", 0x00001000);
				std::abort();
			}

			pluginData.LoadPluginData();
			
			logger::warn("---------------------------------------------------------------------");

			if (pluginData.loadedCorrectly == false) {
				logger::warn("Plugin has not been able to load correctly. Aborting further operations...");
			} else {
				configData.LoadConfigs();

				if (auto dataHandler = RE::TESDataHandler::GetSingleton(); dataHandler) {
					std::uint32_t DLC01CompWorkbenchBot{ 0 };
					std::uint32_t DLC01LvlCompWorkbenchBot{ 0 };
					// exclude automatron robot companion
					if (dataHandler->LookupModByName("DLCRobot.esm")) {
						DLC01CompWorkbenchBot = dataHandler->LookupFormID(69753, "DLCRobot.esm");
						DLC01LvlCompWorkbenchBot = dataHandler->LookupFormID(7917, "DLCRobot.esm");
					}

					for (auto currentNPC : dataHandler->GetFormArray<RE::TESNPC>()) {
						// ignore the player
						// MQ101 Nate and Nora so that they don't become midgets
						if (currentNPC->IsPlayer() ||                                                                       // exclude player
							currentNPC->formID == 687412 || currentNPC->formID == 687413 ||                                 // MQ101 Nate and Nora so that they don't become midgets
							currentNPC->formID == DLC01LvlCompWorkbenchBot || currentNPC->formID == DLC01CompWorkbenchBot)  // robot workbench bots
						{
							continue;
						}

						currentNPC->AddSpell(pluginData.spellForm);
					}
				} else {
					logger::warn("Unable to distribute SCOURGE_SPEL! The plugin will not work...");
				}
			}
		}
	});

	return true;
}
