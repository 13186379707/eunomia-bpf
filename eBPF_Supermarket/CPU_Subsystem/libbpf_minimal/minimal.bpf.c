// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/* Copyright (c) 2020 Facebook */
#include <linux/bpf.h>			//����һЩ��BPF��صĻ������ͺͳ���
#include <bpf/bpf_helpers.h>	//������BPF ��������

char LICENSE[] SEC("license") = "Dual BSD/GPL";		//LICENSE��������BPF��������֤��SEC()�����ṩbpf_helpers.h��ָ���ڣ����������ͺ�������ָ���Ĳ��֡�

int my_pid = 0;		//����������û��ռ������ʹ�ý��̵�ʵ��PID���г�ʼ��

//���彫�����ص��ں��е�BPF����ÿ��write()���κ��û��ռ�Ӧ�ó������syscallʱ������øó���
SEC("tp/syscalls/sys_enter_write")			//tp��tracepoint����˼��SEC��section����˼
int handle_tp(void *ctx)
{
	int pid = bpf_get_current_pid_tgid() >> 32;

	if (pid != my_pid)
		return 0;

	bpf_printk("BPF triggered from PID %d.\n", pid);

	return 0;
}
