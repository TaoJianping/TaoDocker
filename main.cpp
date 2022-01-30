#include <iostream>
#include <CLI/CLI.hpp>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdio>
#include <sched.h>
#include <csignal>
#include <unistd.h>

#include "Container.h"


int main(int argc, char** argv)
{
	CLI::App app("It's just a Docker Demo");

	CLI::App* runCommand = app.add_subcommand("run", "Run image");
	app.require_subcommand();  // 1 or more

	std::string command{};
	runCommand->add_option("Command", command, "Command Args");

	CLI11_PARSE(app, argc, argv);

	std::cout << "Working on --Command from Run Command: " << command << std::endl;

	for (auto subCom: app.get_subcommands())
	{
		std::cout << "Subcommand: " << subCom->get_name() << std::endl;
		if (runCommand->parsed())
		{
			auto container = Container{command};
			return container.Run();;
		}
		else
		{
			throw std::runtime_error{ "Not Support Command!" };
		}
	}

	return 0;
}
