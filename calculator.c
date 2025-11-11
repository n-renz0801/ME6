#include <stdio.h>
#include <string.h>
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

// Initials for 4-digit display (positive values for SSD driver)
#define INITIAL_NR 			  -16  			// e.g., 'NR' encoded for SSD
#define INITIAL_CD 			  -17  			// e.g., 'CD' encoded for SSD
// #define INITIAL_NR 			  0x1F0  			// e.g., 'NR' encoded for SSD
// #define INITIAL_CD 			  0x1EF  			// e.g., 'CD' encoded for SSD


// Global SSD devices
XGpio ssd_driver_0_device, ssd_driver_1_device;

// Function prototypes
void display_initials();
void init_calculator_platform();
void display_result();

int main() {
	char expr[32];
    int a, b, result;
	
	// Initialize platform and SSDs
    init_calculator_platform();

	while (1) {
		print("Enter expression: ");
		scanf("%s", expr);

		int valid = 1;  // flag to check if input is valid
		result = 0;

		if (strstr(expr, ">>")) {
			if (sscanf(expr, "%d>>%d", &a, &b) != 2) valid = 0;
			else result = a >> b;
		} else if (strstr(expr, "<<")) {
			if (sscanf(expr, "%d<<%d", &a, &b) != 2) valid = 0;
			else result = a << b;
		} else if (strchr(expr, '+')) {
			if (sscanf(expr, "%d+%d", &a, &b) != 2) valid = 0;
			else result = a + b;
		} else if (strchr(expr, '-')) {
			if (sscanf(expr, "%d-%d", &a, &b) != 2) valid = 0;
			else result = a - b;
		} else if (strchr(expr, '*')) {
			if (sscanf(expr, "%d*%d", &a, &b) != 2) valid = 0;
			else result = a * b;
		} else if (strchr(expr, '/')) {
			if (sscanf(expr, "%d/%d", &a, &b) != 2) valid = 0;
			else if (b == 0) {  // divide by zero
				print("Error: divide by zero!\n\r");
				display_initials();
				continue;
			} else result = a / b;
		} else if (strchr(expr, '%')) {
			if (sscanf(expr, "%d%%%d", &a, &b) != 2) valid = 0;
			else if (b == 0) {  // remainder by zero
				print("Error: divide by zero!\n\r");
				display_initials();
				continue;
			} else result = a % b;
		} else {
			valid = 0;  // operator not found
		}

		// invalid input
		if (!valid || a < 0 || b < 0) {
			print("Error: Invalid operator or input!\n\r");
			display_initials();
			continue;
		}

		// overflow
		if (result < -0xFFF || result > 0xFFFF) {
			print("Error: Overflow! Result: %d\n\r", result);
			display_initials();
			continue;
		}

		// display result on terminal
		print("Result (decimal): %d\n\r", result);
		print("Result (hex): 0x%X\n\r", result);

		// Display result on SSDs
        display_result(result);
	}

	cleanup_platform();  // clean up at program exit
	return 0;
}



void display_initials() {
    XGpio_DiscreteWrite(&ssd_driver_0_device, SSD_DRIVER_0_CHANNEL, INITIAL_NR & SSD_DRIVER_0_MASK);
    XGpio_DiscreteWrite(&ssd_driver_1_device, SSD_DRIVER_1_CHANNEL, INITIAL_CD & SSD_DRIVER_1_MASK);
}


void init_calculator_platform() {
    XGpio_Config *cfg_ptr;

    // Initialize SSD Driver 0
    cfg_ptr = XGpio_LookupConfig(SSD_DRIVER_0_ID);
    XGpio_CfgInitialize(&ssd_driver_0_device, cfg_ptr, cfg_ptr->BaseAddress);

    // Initialize SSD Driver 1
    cfg_ptr = XGpio_LookupConfig(SSD_DRIVER_1_ID);
    XGpio_CfgInitialize(&ssd_driver_1_device, cfg_ptr, cfg_ptr->BaseAddress);

    // Set both SSDs as output
    XGpio_SetDataDirection(&ssd_driver_0_device, SSD_DRIVER_0_CHANNEL, 0);
    XGpio_SetDataDirection(&ssd_driver_1_device, SSD_DRIVER_1_CHANNEL, 0);

    // Initialize platform
    init_platform();

    // Welcome message
    print("Hello, Calculator!\n\r");

    // Clear display
    XGpio_DiscreteWrite(&ssd_driver_0_device, SSD_DRIVER_0_CHANNEL, 0 & SSD_DRIVER_0_MASK);
    XGpio_DiscreteWrite(&ssd_driver_1_device, SSD_DRIVER_1_CHANNEL, 0 & SSD_DRIVER_1_MASK);
    usleep(1000000);

    // Show initials on startup
    display_initials();
}


void display_result(int result) {
    signed char left;
    unsigned char right;
    int mag = (result < 0) ? -result : result;

    // Left PMOD: signed MSB
    left = (result < 0) ? -(mag >> 8) : (mag >> 8);

    // Right PMOD: last two digits
    right = mag & 0xFF;

    XGpio_DiscreteWrite(&ssd_driver_0_device, SSD_DRIVER_0_CHANNEL, left);
    XGpio_DiscreteWrite(&ssd_driver_1_device, SSD_DRIVER_1_CHANNEL, right);
}
