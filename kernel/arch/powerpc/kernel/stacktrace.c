

#include <linux/module.h>
#include <linux/sched.h>
#include <linux/stacktrace.h>
#include <asm/ptrace.h>
#include <asm/processor.h>

static void save_context_stack(struct stack_trace *trace, unsigned long sp,
			struct task_struct *tsk, int savesched)
{
	for (;;) {
		unsigned long *stack = (unsigned long *) sp;
		unsigned long newsp, ip;

		if (!validate_sp(sp, tsk, STACK_FRAME_OVERHEAD))
			return;

		newsp = stack[0];
		ip = stack[STACK_FRAME_LR_SAVE];

		if (savesched || !in_sched_functions(ip)) {
			if (!trace->skip)
				trace->entries[trace->nr_entries++] = ip;
			else
				trace->skip--;
		}

		if (trace->nr_entries >= trace->max_entries)
			return;

		sp = newsp;
	}
}

void save_stack_trace(struct stack_trace *trace)
{
	unsigned long sp;

	asm("mr %0,1" : "=r" (sp));

	save_context_stack(trace, sp, current, 1);
}
EXPORT_SYMBOL_GPL(save_stack_trace);

void save_stack_trace_tsk(struct task_struct *tsk, struct stack_trace *trace)
{
	save_context_stack(trace, tsk->thread.ksp, tsk, 0);
}
EXPORT_SYMBOL_GPL(save_stack_trace_tsk);
