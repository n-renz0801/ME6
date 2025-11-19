#include <stdio.h>
#include "xparameters.h"
#include "xgpio.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "sleep.h"
#include <string.h>

// Get device IDs from xparameters.h
#define SSD_DRIVER_0_ID       XPAR_AXI_GPIO_0_BASEADDR
#define SSD_DRIVER_1_ID       XPAR_AXI_GPIO_1_BASEADDR
#define SSD_DRIVER_0_CHANNEL  1
#define SSD_DRIVER_1_CHANNEL  1
#define SSD_DRIVER_0_MASK     0b111111111
#define SSD_DRIVER_1_MASK     0b111111111

// Initials to display
#define INITIAL_NR 			  0b100010000  			// 
#define INITIAL_CD 			  0b100010001  			// 
// #define INITIAL_NR 			  -14  			// -1
// #define INITIAL_CD 			  -15  			// -2
// #define INITIAL_NR 			  -16  			// -F
// #define INITIAL_CD 			  -17  			// -0

// Function prototypes
void display_initials();
void get_input_expr(char *buf);

// Global SSD devices
XGpio ssd_driver_0_device, ssd_driver_1_device;

int main() {
	XGpio_Config *cfg_ptr;
	
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

	display_initials();

	while (1) {
        char expr[32];
        int a, b, result;
        
        print("Enter expression: ");
        get_input_expr(expr);

        xil_printf("%s", expr);

        
	}

	cleanup_platform();  // clean up at program exit
	return 0;
}

void display_initials() {
    XGpio_DiscreteWrite(&ssd_driver_0_device, SSD_DRIVER_0_CHANNEL, INITIAL_NR & SSD_DRIVER_0_MASK);
    XGpio_DiscreteWrite(&ssd_driver_1_device, SSD_DRIVER_1_CHANNEL, INITIAL_CD & SSD_DRIVER_1_MASK);
}

void get_input_expr(char *buf)
{
    int i = 0;
    char c;

    while (1) {
        c = inbyte();        // read one character

        // Handle CR/LF (end of input)
        if (c == '\r' || c == '\n') {
            buf[i] = '\0';
            print("\n\r");   // echo newline
            return;
        }

        // Backspace handling
        if ((c == '\b' || c == 127) && i > 0) {
            i--;
            print("\b \b");  // erase last character on screen
            continue;
        }

	    buf[i++] = c;
        xil_printf("%c", c);
    }
}
