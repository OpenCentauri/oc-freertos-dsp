#include <hal_thread.h>

void *kthread_create(void (*threadfn)(void *data), void *data, const char *namefmt, ...)
{
#if 0
	pthread_t thr;
	pthread_create(&thr, NULL, (void *)&threadfn, data);
	pthread_join(&thr, NULL);
#endif
	return NULL;
}


int kthread_stop(void *thread)
{
#if 0
	pthread_exit(NULL);
	return 0;
#endif
	return 0;
}
