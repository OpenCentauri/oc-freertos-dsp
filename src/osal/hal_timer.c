#include <hal_timer.h>
#include <delay.h>

int hal_sleep(unsigned int secs)
{
	msleep(secs * 1000);

	return 0;
}

int hal_usleep(unsigned int usecs)
{
	udelay(usecs);

	return 0;
}

int hal_msleep(unsigned int msecs)
{
	msleep(msecs);

	return 0;
}
