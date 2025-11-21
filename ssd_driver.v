`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 22.10.2025 13:11:53
// Design Name: 
// Module Name: ssd_driver
// Project Name: 
// Target Devices: 
// Tool Versions: 
// Description: 
// 
// Dependencies: 
// 
// Revision:
// Revision 0.01 - File Created
// Additional Comments:
// 
//////////////////////////////////////////////////////////////////////////////////


module ssd_driver(
    input clk,
    input nrst,
    input [8:0] data_in,
    output reg segA, segB, segC, segD, segE, segF, segG,
    output reg sel
    );


    reg  [20:0] delay_ctr2;                         // sel counter
    // localparam [20:0] delay2 = 21'd4;       // update only the sel every 1/(100MHz/1e6cycles) = 10 ms
    localparam [20:0] delay2 = 21'd1_000_000;       // update only the sel every 1/(100MHz/1e6cycles) = 10 ms

    always @(posedge clk) begin
        if (!nrst) begin
            delay_ctr2 <= 0;
            sel       <= 0;
        end else begin
            if (delay_ctr2 == delay2 - 1) begin
                delay_ctr2 <= 0;
                sel       <= ~sel; 
            end else begin
                delay_ctr2 <= delay_ctr2 + 1;
            end
        end
    end


    reg [4:0] display;                                                      // left/right digit to display

    always @(*) begin
        case (display)
            5'h00: {segA, segB, segC, segD, segE, segF, segG} = 7'b1111110;   // 0
            5'h01: {segA, segB, segC, segD, segE, segF, segG} = 7'b0110000;   // 1 
            5'h02: {segA, segB, segC, segD, segE, segF, segG} = 7'b1101101;   // 2
            5'h03: {segA, segB, segC, segD, segE, segF, segG} = 7'b1111001;   // 3
            5'h04: {segA, segB, segC, segD, segE, segF, segG} = 7'b0110011;   // 4
            5'h05: {segA, segB, segC, segD, segE, segF, segG} = 7'b1011011;   // 5
            5'h06: {segA, segB, segC, segD, segE, segF, segG} = 7'b1011111;   // 6
            5'h07: {segA, segB, segC, segD, segE, segF, segG} = 7'b1110000;   // 7
            5'h08: {segA, segB, segC, segD, segE, segF, segG} = 7'b1111111;   // 8
            5'h09: {segA, segB, segC, segD, segE, segF, segG} = 7'b1111011;   // 9
            5'h0A: {segA, segB, segC, segD, segE, segF, segG} = 7'b1110111;   // A
            5'h0B: {segA, segB, segC, segD, segE, segF, segG} = 7'b0011111;   // b
            5'h0C: {segA, segB, segC, segD, segE, segF, segG} = 7'b1001110;   // C
            5'h0D: {segA, segB, segC, segD, segE, segF, segG} = 7'b0111101;   // d
            5'h0E: {segA, segB, segC, segD, segE, segF, segG} = 7'b1001111;   // E
            5'h0F: {segA, segB, segC, segD, segE, segF, segG} = 7'b1000111;   // F
            5'h10: {segA, segB, segC, segD, segE, segF, segG} = 7'b0000001;   // negative sign
            5'h11: {segA, segB, segC, segD, segE, segF, segG} = 7'b0010101;   // n
            5'h12: {segA, segB, segC, segD, segE, segF, segG} = 7'b0000101;   // r
            5'h13: {segA, segB, segC, segD, segE, segF, segG} = 7'b0001101;   // c
            default: 
                  {segA, segB, segC, segD, segE, segF, segG} = 7'b0000001;   // negative sign
        endcase
    end

        // Interpret data_in as a 9-bit signed two's-complement integer from C
    wire signed [8:0] data_s = data_in;
    reg [7:0] abs_val;

    always @(*) begin
        // ==== Custom codes first ====
        if (data_s == -16) begin
            // "nr" : left digit = 'n', right digit = 'r'
            if (sel)
                display = 5'h11;   // 'n'
            else
                display = 5'h12;   // 'r'

        end else if (data_s == -17) begin
            // "cd" : left digit = 'c', right digit = 'd'
            if (sel)
                display = 5'h13;   // 'c'
            else
                display = 5'h0D;   // 'd' (you already mapped 5'h0D as 'd')

        // ==== Generic signed number handling ====
        end else if (data_s < 0) begin
            // Negative value: show "-X" where X is hex digit 0..F
            abs_val = -data_s;  // magnitude: 1..255

            // Only support -0 .. -F (magnitude < 16) nicely
            if (abs_val[7:4] == 4'h0) begin
                if (sel)
                    display = 5'h10;                 // minus sign
                else
                    display = {1'b0, abs_val[3:0]};  // hex digit 0..F
            end else begin
                // Magnitude too big for single hex digit: show "--" (both minus)
                display = 5'h10;
            end

        end else begin
            // Non-negative: show 8-bit value as two hex digits
            if (sel)
                display = {1'b0, data_in[7:4]};  // high nibble
            else
                display = {1'b0, data_in[3:0]};  // low nibble
        end
    end
endmodule
