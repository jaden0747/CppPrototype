#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif

// ---------------------------------------------------------------------------
// cli_server_main.cpp
//
// CLI server that exposes the full SettingsRegistry + CommandRegistry API
// over a local terminal and a Telnet interface.
//
// Registered items demonstrate every feature of the settings framework:
//   AppConfig      — flat scalar settings (no DirtyTracker)
//   RenderSettings — array + enum-string fields, inherits DirtyTracker
//   NetworkConfig  — nested sub-object (EndpointInfo)
//
// Available CLI commands (see command_registry.hpp):
//   coding list                           — list all registered item keys
//   coding list <item>                    — list members of one item
//   coding get                            — dump all items as JSON
//   coding get <item>                     — dump one item as JSON
//   coding set <item> <member> <value>    — set a member by JSON value
//   coding export <file>                  — write all items to a JSON file
//   save <file>                           — alias for export
//   status                                — show registered item names
//   echo <message>                        — echo a string
// ---------------------------------------------------------------------------
#include "settings/command_registry.hpp"
#include "settings/examples/app_config.hpp"
#include "settings/examples/network_config.hpp"
#include "settings/examples/render_settings.hpp"
#include "settings/settings_item.hpp"

#include <cli/cli.h>
#include <cli/clilocalsession.h>
#include <cli/standaloneasioremotecli.h>
#include <cli/standaloneasioscheduler.h>

#include <iostream>
#include <string>

SettingsItem<AppConfig>      g_app("AppConfig");
SettingsItem<RenderSettings> g_render("RenderSettings");
SettingsItem<NetworkConfig>  g_network("NetworkConfig");

int main(int argc, char* argv[])
{
    uint16_t port = 5000;
    for (int i = 1; i < argc - 1; ++i)
    {
        if (std::string(argv[i]) == "--port")
            port = static_cast<uint16_t>(std::stoi(argv[i + 1]));
    }

    SettingsRegistry::instance().loadJson("settings.json");
    std::cout << "Settings loaded from settings.json\n";

    auto rootMenu = buildRootMenu();
    rootMenu->Insert(
        "save",
        [](std::ostream& out, const std::string& filename)
        {
            SettingsRegistry::instance().saveJson(filename);
            out << "Saved to \"" << filename << "\"\n";
        },
        "Save all settings to a JSON file");

    cli::Cli cli(std::move(rootMenu));
    cli.ExitAction([](std::ostream& out) { out << "Goodbye!\n"; });

    cli::StandaloneAsioScheduler scheduler;

    cli::CliLocalTerminalSession localSession(cli, scheduler, std::cout);
    localSession.ExitAction(
        [&scheduler](std::ostream& out)
        {
            out << "Shutting down...\n";
            scheduler.Stop();
        });

    cli::StandaloneAsioCliTelnetServer server(cli, scheduler, port);

    std::cout << "CLI server listening on port " << port << "\n";
    std::cout << "Connect:  telnet localhost " << port << "\n";
    std::cout << "Or:       python test/test_client.py\n\n";

    scheduler.Run();
    return 0;
}
