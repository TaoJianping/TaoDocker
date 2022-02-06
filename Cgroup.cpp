//
// Created by tjp56 on 2022/1/31.
//

#include "Cgroup.h"

#include <utility>
#include <vector>
#include <fstream>
#include <filesystem>
#include <iostream>
#include "absl/strings/str_split.h"

namespace
{
	std::string FindCgroupMountPoint(const std::string& subsystem)
	{
		std::ifstream ifs;
		ifs.open("/proc/self/mountinfo",std::ios::in);

		std::string buf;
		while (getline(ifs, buf))
		{
			std::vector<std::string> v = absl::StrSplit(buf, absl::ByAnyChar(", "));
			if (v.back() == subsystem)
			{
				if (v.size() < 5) {
					throw std::runtime_error("out of index");
				}
				return v.at(4);
			}
		}

		return std::string {};
	}

	std::string GetCgroupPath(const std::string& subsystem, const std::string& cgroupName, bool autoCreate)
	{
		auto cgroupRoot = FindCgroupMountPoint(subsystem);
		auto path = std::filesystem::path(cgroupRoot) / cgroupName;
		if (!std::filesystem::exists(path) && autoCreate)
		{
			if (!std::filesystem::create_directories(path)) {
				throw std::runtime_error("Failed to create cgroup " + path.string());
			}
			std::filesystem::permissions(path, std::filesystem::perms::all);
			return path.string();
		}
		else
		{
			throw std::runtime_error("cgroup " + path.string() + " exists!");
		}
	};
}


cgroup::V1::CgroupManager::CgroupManager(std::string groupName): groupName_(std::move(groupName))
{

}

void cgroup::V1::CgroupManager::Set(cgroup::ResourceConfig res)
{
	cpuSubSystem.Set(groupName_, std::move(res));
}

std::string cgroup::CpuSubSystem::Name()
{
	return "cpu";
}

void cgroup::CpuSubSystem::Set(std::string groupName, cgroup::ResourceConfig res)
{
	auto subSystemCgroupPath = GetCgroupPath(Name(), groupName, true);
	if (subSystemCgroupPath.empty()) {
		throw std::runtime_error("subSystemCgroupPath Get Failed!");
	}
	if (res.CpuShare.empty()) {
		throw std::runtime_error("CpuShare Not Set!");
	}
	// TODO 我就写死了，以后可以完善
	auto cpuSharePath = std::filesystem::path(subSystemCgroupPath) / "cpu.shares";
	std::ofstream MyFile(cpuSharePath.string());
	MyFile << "20000";
	// Close the file
	MyFile.close();
}
