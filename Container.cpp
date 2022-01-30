//
// Created by tjp56 on 2022/1/30.
//

#include "Container.h"

#include <utility>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <unistd.h>

/* 定义一个给 clone 用的栈，栈大小1M */
static constexpr u_int32_t STACK_SIZE = 1024 * 1024;
static char container_stack[STACK_SIZE];
char* const container_args[] = {
		"/bin/bash",
		NULL
};

namespace
{
	int container_main(void* arg)
	{
		printf("Container [%5d] - inside the container!\n", getpid());
		// 设置hostname
		sethostname("container", 10);
		// systemd加入linux之后, mount namespace就变成 shared by default, 所以必须显式声明你要这个新的mount namespace独立
		// https://github.com/xianlubird/mydocker/issues/41
		mount("", "/", "", MS_PRIVATE | MS_REC, nullptr);
		mount("proc", "/proc", "proc", MS_NOEXEC | MS_NODEV | MS_NOSUID, nullptr);
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
	/* 等待子进程结束 */
	waitpid(container_pid, nullptr, 0);
	printf("Parent - container stopped!\n");
	return 0;
}
