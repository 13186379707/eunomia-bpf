#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>
#include <bpf/libbpf.h>
#include <signal.h>
#include "cs_delay.skel.h"	//������ BPF �ֽ������صĹ�����
#include "cs_delay.h"

static volatile bool exiting = false;

static void sig_handler(int sig)
{
	exiting = true;
}

static int handle_event(void *ctx, void *data,unsigned long data_sz)
{
	const struct event *e = data;
	printf("pid:%-7d  t1:%lu  t2:%lu  delay:%lu\n",e->pid,e->t1,e->t2,e->delay);
	
	return 0;
}
	

static int libbpf_print_fn(enum libbpf_print_level level, const char *format, va_list args)
{
	return vfprintf(stderr, format, args);
}

int main(int argc, char **argv)
{
	struct ring_buffer *rb = NULL;
	struct cs_delay_bpf *skel;
	int err;

	libbpf_set_strict_mode(LIBBPF_STRICT_ALL);
	/* ����libbpf����͵�����Ϣ�ص� */
	libbpf_set_print(libbpf_print_fn);

	/* ���ɾ��ش���Ctrl-C
	   SIGINT����Interrupt Key������ͨ����CTRL+C����DELETE�����͸�����ForeGround Group�Ľ���
       SIGTERM��������ֹ���̣�kill�����
	*/
	signal(SIGINT, sig_handler);		//signal����ĳһ�źŵĶ�Ӧ����
	signal(SIGTERM, sig_handler);

	/* ��BPFӦ�ó��� */
	skel = cs_delay_bpf__open();
	if (!skel) {
		fprintf(stderr, "Failed to open BPF skeleton\n");
		return 1;
	}
	
	/* ���ز���֤BPF���� */
	err = cs_delay_bpf__load(skel);
	if (err) {
		fprintf(stderr, "Failed to load and verify BPF skeleton\n");
		goto cleanup;
	}
	
	/* ���Ӹ��ٵ㴦����� */
	err = cs_delay_bpf__attach(skel);
	if (err) {
		fprintf(stderr, "Failed to attach BPF skeleton\n");
		goto cleanup;
	}
	
	/* ���û��λ�������ѯ */
	rb = ring_buffer__new(bpf_map__fd(skel->maps.rb), handle_event, NULL, NULL);	//ring_buffer__new() API�������ڲ�ʹ�ö���ѡ�����ݽṹ��ָ���ص�
	if (!rb) {
		err = -1;
		fprintf(stderr, "Failed to create ring buffer\n");
		goto cleanup;
	}
	
	/* �����¼� */
	while (!exiting) {
		err = ring_buffer__poll(rb, 100 /* timeout, ms */);		//ring_buffer__poll(),��ѯ��ringbuf��������������¼���handle_event������ִ��
		/* Ctrl-C will cause -EINTR */
		if (err == -EINTR) {
			err = 0;
			break;
		}
		if (err < 0) {
			printf("Error polling perf buffer: %d\n", err);
			break;
		}
	}
	
/* ж��BPF���� */
cleanup:
	ring_buffer__free(rb);
	cs_delay_bpf__destroy(skel);
	
	return err < 0 ? -err : 0;
}
