// SPDX-License-Identifier: (LGPL-2.1 OR BSD-2-Clause)
/* Copyright (c) 2020 Facebook */
#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>
#include <bpf/libbpf.h>
#include "minimal.skel.h"	//������ BPF �ֽ������صĹ�����

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)		//arg���Ա���argument
{
	return vfprintf(stderr, format, args);		//���ɱ�����б�ĸ�ʽ������д������stderrָ���ʶ�������FILE�����ָ�롣format������ʽ�ַ�����C�ַ��������ʽ��printf�еĸ�ʽ��ͬ��args���������б��ֵ��
}

int main(int argc, char **argv)
{
	struct minimal_bpf *skel;
	int err;

	libbpf_set_strict_mode(LIBBPF_STRICT_ALL);
	/* ����libbpf����͵�����Ϣ�ص� */
	libbpf_set_print(libbpf_print_fn);

	/* ��BPFӦ�ó��� */
	skel = minimal_bpf__open();
	if (!skel) {
		fprintf(stderr, "Failed to open BPF skeleton\n");
		return 1;
	}

	/* ȷ��BPF����ֻ�������Խ��̵�write()ϵͳ���� */
	skel->bss->my_pid = getpid();		//bss��ͨ����ָ������ų�����δ��ʼ���Ļ��߳�ʼ��Ϊ0��ȫ�ֱ����;�̬������һ���ڴ�����

	/* ���ز���֤BPF���� */
	err = minimal_bpf__load(skel);
	if (err) {
		fprintf(stderr, "Failed to load and verify BPF skeleton\n");
		goto cleanup;
	}

	/* ���Ӹ��ٵ㴦����� */
	err = minimal_bpf__attach(skel);
	if (err) {
		fprintf(stderr, "Failed to attach BPF skeleton\n");
		goto cleanup;
	}

	printf("Successfully started! Please run `sudo cat /sys/kernel/debug/tracing/trace_pipe` "
	       "to see output of the BPF programs.\n");

	for (;;) {
		/* �������ǵ�BPF�ƻ� */
		fprintf(stderr, ".");
		sleep(1);
	}

/* ж��BPF���� */
cleanup:
	minimal_bpf__destroy(skel);
	return -err;
}
