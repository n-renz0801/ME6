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

    always @(*) begin
        if (data_in[8] == 1) begin
            case (data_in[7:0])
                8'h00: display = sel ? 5'h10 : 5'h00; // -0
                8'h01: display = sel ? 5'h10 : 5'h01; // -1
                8'h02: display = sel ? 5'h10 : 5'h02; // -2
                8'h03: display = sel ? 5'h10 : 5'h03; // -3
                8'h04: display = sel ? 5'h10 : 5'h04; // -4
                8'h05: display = sel ? 5'h10 : 5'h05; // -5
                8'h06: display = sel ? 5'h10 : 5'h06; // -6
                8'h07: display = sel ? 5'h10 : 5'h07; // -7
                8'h08: display = sel ? 5'h10 : 5'h08; // -8
                8'h09: display = sel ? 5'h10 : 5'h09; // -9
                8'h0A: display = sel ? 5'h10 : 5'h0A; // -A
                8'h0B: display = sel ? 5'h10 : 5'h0B; // -b
                8'h0C: display = sel ? 5'h10 : 5'h0C; // -C
                8'h0D: display = sel ? 5'h10 : 5'h0D; // -d
                8'h0E: display = sel ? 5'h10 : 5'h0E; // -E
                8'h0F: display = sel ? 5'h10 : 5'h0F; // -F

                // Custom initials examples:
                8'h10: display = sel ? 5'h11 : 5'h12; // "nr"
                8'h11: display = sel ? 5'h12 : 5'h13; // "cd"
                default: display = sel ? data_in[7:4] : data_in[3:0];    // data_in[8] = 0
            endcase
        end 
        else display = sel ? data_in[7:4] : data_in[3:0];    // data_in[8] = 0
    end
endmodule
