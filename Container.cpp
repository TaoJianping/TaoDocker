//
// Created by tjp56 on 2022/1/30.
//


#include <utility>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/mount.h>
#include <unistd.h>
#include <stdexcept>
#include <filesystem>

#include "Container.h"
#include "Cgroup.h"

/* 定义一个给 clone 用的栈，栈大小1M */
static constexpr u_int32_t STACK_SIZE = 1024 * 1024;
static char container_stack[STACK_SIZE];
char* const container_args[] = {
		"/bin/bash",
		NULL
};

namespace
{
    int pivot_root(const char *new_root, const char *put_old)
    {
        return syscall(SYS_pivot_root, new_root, put_old);
    }

    void SetupMount()
    {
        const std::string newRoot = "";
        const std::filesystem::path newRootPath {newRoot};

        /*
         * Ensure that 'new_root' and its parent mount don't have
         * shared propagation (which would cause pivot_root() to
         * return an error), and prevent propagation of mount
         * events to the initial mount namespace.
         * systemd加入linux之后, mount namespace就变成 shared by default, 所以必须显式声明你要这个新的mount namespace独立
         * https://github.com/xianlubird/mydocker/issues/41
         * */
        if (mount(nullptr, "/", nullptr, MS_REC | MS_PRIVATE, nullptr) == -1)
            throw std::runtime_error("mount-MS_PRIVATE");

        /*
         * Ensure that 'new_root' is a mount point.
         * 这一步最主要是要将我们要挂载的rootfs变成一个mount point, 方便我们之后pivot_root的时候能够挂载上，因为pivot_root的new_root
         * 和old_root都必须是不同的mount point
         *
         * 绑定挂载(MS_BIND)使得一个文件或目录在系统目录树的另外一个点上可以看得见，而对原目录的操作将实际上应用于绑定的目录，而并不改变原目录。如，
         * mount --bind /vz/apt/ /var/cache/apt/archives/
         * 上面命令的意思是把 /vz/apt/ 目录绑定挂载到 /var/cache/apt/archives/，以后只要是写到 /var/cache/apt/archives/ 目录中的数据，
         * 都会自动写到其绑定目录 /vz/apt/ 中，真正的 /var/cache/apt/archives/ 中并没有数据
         * */
        if (mount(newRoot.c_str(), newRoot.c_str(), NULL, MS_BIND | MS_REC, NULL) == -1)
            throw std::runtime_error("mount-MS_BIND");

        // create put old dir
        std::filesystem::path putOldPath = newRootPath / ".oldrootfs";
        if (!std::filesystem::create_directory(putOldPath)) {
            throw std::runtime_error("create old put dir failed");
        };
        std::filesystem::permissions(putOldPath, std::filesystem::perms::all);

        /*
         * And pivot the root filesystem.
         * */
        if (pivot_root(newRootPath.c_str(), putOldPath.c_str()) == -1)
            throw std::runtime_error("pivot_root");

        /*
         * Switch the current working directory to "/".
         * */
        if (chdir("/") == -1)
            throw std::runtime_error("chdir");

        /*
         * Unmount old root and remove mount point.
         * */
        if (umount2(putOldPath.c_str(), MNT_DETACH) == -1)
            throw std::runtime_error("umount2");
        if (rmdir(putOldPath.c_str()) == -1)
            throw std::runtime_error("rmdir");

        mount("proc", "/proc", "proc", MS_NOEXEC | MS_NODEV | MS_NOSUID, nullptr);
        mount("tmpfs", "/dev", "tmpfs", MS_NOSUID | MS_STRICTATIME, nullptr);
    }

	int container_main(void* arg)
	{
		printf("Container [%5d] - inside the container!\n", getpid());

        // 设置hostname
		sethostname("container", 10);

        // 挂载文件系统
        SetupMount();

        // 直接执行一个shell，以便我们观察这个进程空间里的资源是否被隔离了
		execv(container_args[0], container_args);
		printf("Something's wrong!\n");
		return 1;
	}
}

Container::Container(std::string command): command_(std::move(command))
{

}

int Container::Run()
{
	printf("Parent - start a container!\n");
	/*
	 * 调用clone函数，其中传出一个函数，还有一个栈空间的（为什么传尾指针，因为栈是反着的）
	 * */
	int container_pid = clone(container_main,
			container_stack + STACK_SIZE,
			CLONE_NEWUTS | CLONE_NEWIPC | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD,
			nullptr);

	// set cgroup
    auto res = cgroup::ResourceConfig {
            "",
            "20000",
            ""
    };
    auto cgroupManager = cgroup::V1::CgroupManager{"TaoDockerTestCgroup"};
    cgroupManager.Set(res);
    cgroupManager.Apply(container_pid);

	// wait
	waitpid(container_pid, nullptr, 0);
	printf("Parent - container stopped!\n");
	return 0;
}
