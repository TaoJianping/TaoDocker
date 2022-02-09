//
// Created by tjp56 on 2022/1/31.
//

#ifndef TAODOCKER_CGROUP_H
#define TAODOCKER_CGROUP_H

#include <string>


namespace cgroup
{

	struct ResourceConfig
	{
		std::string MemoryLimit;
		std::string CpuShare;
		std::string CpuSet;
	};

	class SubSystem
	{
	public:
		virtual std::string Name() = 0;
		virtual void Set(std::string path, const ResourceConfig& res) = 0;
		virtual void Apply(std::string path, int pid) = 0;
		virtual void Remove(std::string path) = 0;
	};

	class CpuSubSystem : public SubSystem
	{
	public:
		 std::string Name() override;
		 void Set(std::string path, const ResourceConfig& res) override;
		 void Apply(std::string groupName, int pid) override;
		 void Remove(std::string groupName) override;
	};

	namespace V1
	{
		class CgroupManager
		{
		public:
			explicit CgroupManager(std::string groupName);
			~CgroupManager();
			void Apply(int pid);
			void Set(ResourceConfig res);
			void Destroy();

		private:
			std::string groupName_;
			// 其实应该放在一个列表里面，因为他们都是继承SubSystem的，但是图简单，算了
			CpuSubSystem cpuSubSystem {};
		};
	}

	namespace V2
	{

	}
}


#endif //TAODOCKER_CGROUP_H
