#include <stdint.h>
#include <stdlib.h>

void *hal_malloc(uint32_t size)
{
	return malloc(size);
}

void hal_free(void *p)
{
	return free(p);
}

unsigned long hal_virt_to_phys(unsigned long virtaddr)
{
	return virtaddr;
}

unsigned long hal_phys_to_virt(unsigned long phyaddr)
{
	return phyaddr;
}
