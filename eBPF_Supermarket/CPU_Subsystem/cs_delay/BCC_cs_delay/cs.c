#include <uapi/linux/ptrace.h>

BPF_HASH(start, u32);
BPF_HISTOGRAM(dist);

int do_entry(struct pt_regs *ctx)	//pt_regs�ṹ��������ϵͳ���û������ں���Ŀ�ڼ佫�Ĵ����洢���ں˶�ջ�ϵķ�ʽ
{
	u64 t1= bpf_ktime_get_ns()/1000;	//bpf_ktime_get_ns������ϵͳ����������������ʱ��(������Ϊ��λ)��������ϵͳ�����ʱ�䡣;
	u32 pid = bpf_get_current_pid_tgid();
	start.update(&pid,&t1);
	
	return 0;
}

int do_return(struct pt_regs *ctx)
{
	u64 t2= bpf_ktime_get_ns()/1000;
	u32 pid;
	u64 *tsp, delay;
	
	pid = bpf_get_current_pid_tgid();
	tsp = start.lookup(&pid);
	
	if (tsp != 0)
    {
        delay = t2 - *tsp;
        start.delete(&pid);
        dist.increment(bpf_log2l(delay));
	}
	
	return 0;
}
