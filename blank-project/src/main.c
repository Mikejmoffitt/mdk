#include "md/megadrive.h"

void main(void)
{
	megadrive_init();

	while(1)
	{
		megadrive_finish();
	}
}
