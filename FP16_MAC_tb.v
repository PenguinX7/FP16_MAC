`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2023/02/24 21:56:30
// Design Name: 
// Module Name: FP16_MAC_tb
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


module FP16_MAC_tb(

    );
    reg clk;
    reg rst;
    reg input_valid;
    reg [15:0]data_k;
    reg [15:0]data_x;
    reg [15:0]data_b;
    reg [1:0]opcode;
    wire idle;
    wire output_up;
    wire [15:0]data1;
    wire [1:0]opcode_o;
    
    always #5 clk = ~clk;
    
    initial begin
        clk = 1;
        rst = 1;
        input_valid = 0;
        opcode = 2'd3;
        data_k = 16'h1077;
        //data_k = 16'hffff;
        data_x = 16'h1006;
        data_b = 16'h0000;
        #10 rst = 0;
        #10 input_valid = 1;
//        #10 data_k = 16'h5432;
//        #10 data_b = 16'h1987;

        #150 rst = 1;
    end
    
//    FP16_MAC U1 (
//    .clk(clk),
//    .rst(rst),
//    .input_valid(input_valid),
//    .data_k(data_k),
//    .data_x(data_x),
//    .data_b(data_b),
//    .idle(idle),
//    .data_o(data1),
//    .output_up(output_up)
//    );

    FP16_MAC_pipeline U1 (
    .clk(clk),
    .rst(rst),
    .input_valid(input_valid),
    .data_k(data_k),
    .data_x(data_x),
    .data_b(data_b),
    .opcode(opcode),
    //.idle(idle),
    .data_o(data1),
    .output_up(output_up),
    .opcode_o(opcode_o)
    );
    
endmodule
