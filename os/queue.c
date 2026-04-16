#include "queue.h"
#include "defs.h"

static const Stride_t BIG_STRIDE = (1ULL << 32);

static int cmp_stride(Stride_t a, Stride_t b)
{
	if (a == b)
		return 0;
	return a > b ? 1 : -1;
}

static int find_low_stride_queue(struct queue *q, struct proc *p)
{
	if (q->empty)
		return -1;

	int count = (q->tail - q->front + NPROC) % NPROC;
	if (count == 0)
		count = NPROC;

	int low_index = q->front;
	int index = q->front;
	for (int i = 1; i < count; i++) {
		index = (index + 1) % NPROC;
		if (cmp_stride(p[q->data[low_index]].stride,
			       p[q->data[index]].stride) > 0)
			low_index = index;
	}

	if (p[q->data[low_index]].priority <= 1)
		p[q->data[low_index]].priority = 2;
	p[q->data[low_index]].stride +=
		BIG_STRIDE / p[q->data[low_index]].priority;
	return low_index;
}

void init_queue(struct queue *q)
{
	q->front = q->tail = 0;
	q->empty = 1;
}

void push_queue(struct queue *q, int value)
{
	if (!q->empty && q->front == q->tail) {
		panic("queue shouldn't be overflow");
	}
	q->empty = 0;
	q->data[q->tail] = value;
	q->tail = (q->tail + 1) % NPROC;
}

int pop_queue(struct queue *q)
{
	if (q->empty)
		return -1;
	int value = q->data[q->front];
	q->front = (q->front + 1) % NPROC;
	if (q->front == q->tail)
		q->empty = 1;
	return value;
}

int pop_priority_queue(struct queue *q, struct proc *p)
{
	if (q->empty)
		return -1;

	int low_index = find_low_stride_queue(q, p);
	if (low_index < 0)
		return -1;

	int value = q->data[q->front];
	if (low_index != q->front) {
		int pop_value = q->data[low_index];
		q->data[low_index] = value;
		value = pop_value;
	}

	q->front = (q->front + 1) % NPROC;
	if (q->front == q->tail)
		q->empty = 1;
	return value;
}
