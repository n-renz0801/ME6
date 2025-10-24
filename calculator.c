#include <stdio.h>
#include "xparameters.h"
#include "xgpio.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "sleep.h"

// Get device IDs from xparameters.h
#define SSD_DRIVER_0_ID       XPAR_AXI_GPIO_0_BASEADDR
#define SSD_DRIVER_1_ID       XPAR_AXI_GPIO_1_BASEADDR
#define SSD_DRIVER_0_CHANNEL  1
#define SSD_DRIVER_1_CHANNEL  1
#define SSD_DRIVER_0_MASK     0b111111111
#define SSD_DRIVER_1_MASK     0b111111111

int main() {
	XGpio_Config *cfg_ptr;
	XGpio ssd_driver_0_device, ssd_driver_1_device;
	
	// int32_t initial_cd = -14; //-2
    // int32_t initial_nr = -15; //-1
	// int32_t initial_cd = -17; //-0
	// int32_t initial_nr = -16; //-f
	// int32_t initial_cd = -19; //-e
    // int32_t initial_nr = -18; //-d
	char a;
	
	// Initialize SSD Driver 0 Device
	cfg_ptr = XGpio_LookupConfig(SSD_DRIVER_0_ID);
	XGpio_CfgInitialize(&ssd_driver_0_device, cfg_ptr, cfg_ptr->BaseAddress);
	
	// Initialize SSD Driver 1 Device
	cfg_ptr = XGpio_LookupConfig(SSD_DRIVER_1_ID);
	XGpio_CfgInitialize(&ssd_driver_1_device, cfg_ptr, cfg_ptr->BaseAddress);
	
	// Set SSD Driver 0 Tristate
	XGpio_SetDataDirection(&ssd_driver_0_device, SSD_DRIVER_0_CHANNEL, 0);
	
	// Set SSD Driver 1 Tristate
	XGpio_SetDataDirection(&ssd_driver_1_device, SSD_DRIVER_1_CHANNEL, 0);
	
	init_platform();

    print("Hello, Calculator!\n\r");

	XGpio_DiscreteWrite(&ssd_driver_0_device, SSD_DRIVER_0_CHANNEL, 0 & SSD_DRIVER_0_MASK);
	XGpio_DiscreteWrite(&ssd_driver_1_device, SSD_DRIVER_1_CHANNEL, 0 & SSD_DRIVER_1_MASK);
	usleep(1000000);

	XGpio_DiscreteWrite(&ssd_driver_0_device, SSD_DRIVER_0_CHANNEL, initial_nr & SSD_DRIVER_0_MASK);
	XGpio_DiscreteWrite(&ssd_driver_1_device, SSD_DRIVER_1_CHANNEL, initial_cd & SSD_DRIVER_1_MASK);
    
	cleanup_platform();

	while (1) {
		print("Enter: ");
		a = inbyte();
	}
}
