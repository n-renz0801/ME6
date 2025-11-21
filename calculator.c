#include <stdio.h>
#include "xparameters.h"
#include "xgpio.h"
#include "xil_printf.h"
#include "xil_types.h"
#include "sleep.h"
#include <string.h>

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// Get device IDs from xparameters.h
#define SSD_DRIVER_0_ID       XPAR_AXI_GPIO_0_BASEADDR
#define SSD_DRIVER_1_ID       XPAR_AXI_GPIO_1_BASEADDR

#define SSD_DRIVER_0_CHANNEL  1
#define SSD_DRIVER_1_CHANNEL  1

#define SSD_DRIVER_0_MASK     0b111111111
#define SSD_DRIVER_1_MASK     0b111111111

// Initials to display
#define INITIAL_NR 			  (-16)  			
#define INITIAL_CD 			  (-17)  		

typedef enum {
    OP_INVALID = 0,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_SHL,
    OP_SHR
} OpKind;

// Function prototypes
void display_initials();
void display_result(int result);
void get_input_expr(char *buf);

const char *skip_spaces(const char *p);
int  parse_uint(const char *p, int *out, const char **rest);
int  parse_expr(const char *expr, int *a, int *b, OpKind *op);

// Global SSD devices
XGpio ssd_driver_0_device, ssd_driver_1_device;

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

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
        int a = 0, b = 0, result = 0;
        OpKind op = OP_INVALID;
        int ok;

        print("Enter expression: ");
        get_input_expr(expr);          // read a line from UART

        // Parse "<uint> op <uint>"
        ok = parse_expr(expr, &a, &b, &op);
        if (!ok) {
            print("Error: Invalid operator or input!\n\r");
            display_initials();
            continue;
        }

        // Compute the result
        switch (op) {
        case OP_ADD:  result = a + b; break;
        case OP_SUB:  result = a - b; break;
        case OP_MUL:  result = a * b; break;

        case OP_DIV:
            if (b == 0) {
                print("Error: divide by zero!\n\r");
                display_initials();
                continue;
            }
            result = a / b;
            break;

        case OP_MOD:
            if (b == 0) {
                print("Error: divide by zero!\n\r");
                display_initials();
                continue;
            }
            result = a % b;
            break;

        case OP_SHL:
            result = a << b;
            break;

        case OP_SHR:
            result = a >> b;
            break;

        default:
            print("Error: Invalid operator!\n\r");
            display_initials();
            continue;
        }

        // Overflow check for 4-digit display: -FFF .. FFFF
        if (result < -0xFFF || result > 0xFFFF) {
            print("Error: Overflow! Result: %d\n\r", result);
            display_initials();
            continue;
        }

        // Success: print to terminal
        print("Result (decimal): %d\n\r", result);
        print("Result (hex): 0x%X\n\r", result);

        // Show result on SSDs
        display_result(result);
    }

    cleanup_platform();
    return 0;
}

// ---------------------------------------------------------------------------
// ---------------------------------------------------------------------------

// Show initials "nr" and "cd" (Verilog codes: -16 & -17)
void display_initials(void)
{
    XGpio_DiscreteWrite(&ssd_driver_0_device,
                        SSD_DRIVER_0_CHANNEL,
                        INITIAL_NR & SSD_DRIVER_0_MASK);

    XGpio_DiscreteWrite(&ssd_driver_1_device,
                        SSD_DRIVER_1_CHANNEL,
                        INITIAL_CD & SSD_DRIVER_1_MASK);
}

void display_result(int result)
{
    unsigned int u = (unsigned int)result;      // keep low 16 bits
    unsigned int hi = (u >> 8) & 0xFF;          // high byte
    unsigned int lo = u & 0xFF;                 // low byte

    XGpio_DiscreteWrite(&ssd_driver_0_device,
                        SSD_DRIVER_0_CHANNEL,
                        hi & SSD_DRIVER_0_MASK);

    XGpio_DiscreteWrite(&ssd_driver_1_device,
                        SSD_DRIVER_1_CHANNEL,
                        lo & SSD_DRIVER_1_MASK);
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

const char *skip_spaces(const char *p)
{
    while (*p == ' ' || *p == '\t')
        p++;
    return p;
}

// Parse a non-negative decimal integer.
// Returns 1 on success, 0 on failure. *rest points to first char after the number.
int parse_uint(const char *p, int *out, const char **rest)
{
    int val = 0;
    const char *s = p;

    if (*s < '0' || *s > '9')
        return 0;   // must start with a digit

    while (*s >= '0' && *s <= '9') {
        val = val * 10 + (*s - '0');
        s++;
    }

    *out  = val;
    *rest = s;
    return 1;
}

// expr format:  <uint> [spaces] op [spaces] <uint>
// op ∈ { +, -, *, /, %, <<, >> }
int parse_expr(const char *expr, int *a, int *b, OpKind *op)
{
    const char *p = expr;

    *op = OP_INVALID;

    // First operand
    p = skip_spaces(p);
    if (!parse_uint(p, a, &p))
        return 0;

    // Operator
    p = skip_spaces(p);
    if (p[0] == '<' && p[1] == '<') {
        *op = OP_SHL;
        p += 2;
    } else if (p[0] == '>' && p[1] == '>') {
        *op = OP_SHR;
        p += 2;
    } else {
        switch (*p) {
        case '+': *op = OP_ADD; break;
        case '-': *op = OP_SUB; break;
        case '*': *op = OP_MUL; break;
        case '/': *op = OP_DIV; break;
        case '%': *op = OP_MOD; break;
        default:  return 0;   // unknown operator
        }
        p++;  // consume single-char operator
    }

    // Second operand
    p = skip_spaces(p);
    if (!parse_uint(p, b, &p))
        return 0;

    // Must reach end (no extra junk)
    p = skip_spaces(p);
    if (*p != '\0')
        return 0;

    return 1;
}
