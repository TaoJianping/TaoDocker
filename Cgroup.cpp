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
            if (subsystem == "cpu")
            {
                if (v.back() == subsystem || v.back() == "cpuacct")
                {
                    std::string str = buf;
                    std::vector<std::string> nv = absl::StrSplit(str, " ");
                    if (nv.size() < 5) {
                        throw std::runtime_error("out of index");
                    }
                    std::string res = std::string {nv.at(4)};
                    printf("%s", res.data());
                    return res;
                }
            } else
            {
                if (v.back() == subsystem)
                {
                    if (v.size() < 5) {
                        throw std::runtime_error("out of index");
                    }
                    return v.at(4);
                }
            }
		}

		return std::string {};
	}

	std::string GetCgroupPath(const std::string& subsystem, const std::string& cgroupName, bool autoCreate)
	{
		std::string cgroupRoot = FindCgroupMountPoint(subsystem);
        if (cgroupRoot.empty()) {
            throw std::runtime_error("NOT FOUND cgroupRoot");
        }
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
    // CPU Controller
	cpuSubSystem.Set(groupName_, res);
}

void cgroup::V1::CgroupManager::Apply(int pid)
{
    cpuSubSystem.Apply(groupName_, pid);
}

void cgroup::V1::CgroupManager::Destroy() {

}

std::string cgroup::CpuSubSystem::Name()
{
	return "cpu";
}

void cgroup::CpuSubSystem::Set(std::string groupName, const cgroup::ResourceConfig& res)
{
	auto subSystemCgroupPath = GetCgroupPath(Name(), groupName, true);
	if (subSystemCgroupPath.empty()) {
		throw std::runtime_error("subSystemCgroupPath Get Failed!");
	}
	if (res.CpuShare.empty()) {
		throw std::runtime_error("CpuShare Not Set!");
	}
	// TODO 我就写死了，以后可以完善
	auto cpuSharePath = std::filesystem::path(subSystemCgroupPath) / "cpu.cfs_quota_us";
	std::ofstream MyFile(cpuSharePath.string());
    if (!MyFile.is_open()) {
        throw std::runtime_error("File Open Failed!");
    }
	MyFile << "20000";
	// Close the file
	MyFile.close();
}

void cgroup::CpuSubSystem::Apply(std::string groupName, int pid)
{
    auto subSystemCgroupPath = GetCgroupPath(Name(), groupName, false);
    if (subSystemCgroupPath.empty()) {
        throw std::runtime_error("subSystemCgroupPath Get Failed!");
    }
    auto taskFile = std::filesystem::path(subSystemCgroupPath) / "tasks";
    std::ofstream MyFile(taskFile.string());
    if (!MyFile.is_open()) {
        throw std::runtime_error("File Open Failed!");
    }
    MyFile << pid;
    // Close the file
    MyFile.close();
}

void cgroup::CpuSubSystem::Remove(std::string path) {

}
