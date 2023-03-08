`timescale 1ns / 1ps
//////////////////////////////////////////////////////////////////////////////////
// Company: 
// Engineer: 
// 
// Create Date: 2023/03/01 19:10:04
// Design Name: 
// Module Name: FP16_MAC_pipeline
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


module FP16_MAC_pipeline(
    input data_k,
    input data_x,
    input data_b,
    input rst,
    input input_valid,
    input clk,
    input opcode,
    output data_o,
    output output_up,
    output opcode_o
    );
    
    wire [15:0]data_k;
    wire [15:0]data_x;
    wire [15:0]data_b;
    wire rst;
    wire clk;
    wire input_valid;
    wire [1:0]opcode;
    reg [15:0]data_o;
    reg output_up;
    reg [1:0]opcode_o;
    
    //step1 : input_check
    reg [1:0]opcode1;
    reg input_check_over;
    reg sign_k;
    reg sign_x;
    reg sign_b;
    reg signed[6:0]exp_k;
    reg signed[6:0]exp_x;
    reg signed[6:0]exp_b;
    reg [10:0]rm_k;
    reg [10:0]rm_x;
    reg [10:0]rm_b;
    
    always@(posedge clk)    begin
        if(rst) begin                           //reset
            opcode1 <= 2'd0;
            input_check_over <= 1'b0;
            sign_k <= 1'b0;
            sign_x <= 1'b0;
            sign_b <= 1'b0;
            exp_k <= 7'd0;
            exp_x <= 7'd0;
            exp_b <= 7'd0;
            rm_k <= 11'd0;
            rm_x <= 11'd0;
            rm_b <= 11'd0;    
        end
        else if(input_valid)    begin           //work
            input_check_over <= 1'b1;
            opcode1 <= opcode;
            sign_k <= data_k[15];
            sign_x <= data_x[15];
            sign_b <= data_b[15];
            rm_k[9:0] <= data_k[9:0];
            rm_x[9:0] <= data_x[9:0];
            rm_b[9:0] <= data_b[9:0];
            if(data_k[14:10] == 5'b00000)   begin
                rm_k[10] <= 1'b0;
                if(data_k[9:0] == 10'd0)        //exp=0,rm=0,0
                    exp_k <= 7'd0;
                else                            //exp=0,rm!=0,denormal
                    exp_k <= 7'd1;
            end
            else    begin                       //exp!=0,normal
                rm_k[10] <= 1'b1;
                exp_k <= data_k[14:10];
            end
            if(data_x[14:10] == 5'b00000)   begin
                rm_x[10] <= 1'b0;
                if(data_x[9:0] == 10'd0)        //exp=0,rm=0,0
                    exp_x <= 7'd0;
                else                            //exp=0,rm!=0,denormal
                    exp_x <= 7'd1;
            end
            else    begin                       //exp!=0,normal
                rm_x[10] <= 1'b1;
                exp_x <= data_x[14:10];
            end
            if(data_b[14:10] == 5'b00000)   begin
                rm_b[10] <= 1'b0;
                if(data_b[9:0] == 10'd0)        //exp=0,rm=0,0
                    exp_b <= 7'd0;
                else                            //exp=0,rm!=0,denormal
                    exp_b <= 7'd1;
            end
            else    begin                       //exp!=0,normal
                rm_b[10] <= 1'b1;
                exp_b <= data_b[14:10];
            end
        end
        else    begin                           //idle
            input_check_over <= 1'b0;
            opcode1 <= opcode1;
            sign_k <= sign_k;
            sign_x <= sign_x;
            sign_b <= sign_b;
            exp_k <= exp_k;
            exp_x <= exp_x;
            exp_b <= exp_b;
            rm_k <= rm_k;
            rm_x <= rm_x;
            rm_b <= rm_b;
        end
    end
    
    //step2 : mcl
    reg mcl_over;
    reg [1:0]opcode2;
    reg sign_mcl;
    reg signed[6:0]exp_mcl;
    reg [21:0]rm_mcl;
    reg sign_b2;
    reg signed[6:0]exp_b2;
    reg [10:0]rm_b2;
    
    always@(posedge clk)    begin
        if(rst) begin                           //reset
            mcl_over <= 1'b0;
            opcode2 <= 2'b0;
            sign_mcl <= 1'b0;
            exp_mcl <= 7'd0;
            rm_mcl <= 22'd0;
            sign_b2 <= 1'b0;
            exp_b2 <= 7'd0;
            rm_b2 <= 11'd0;
        end
        else if(input_check_over)   begin       //work
            mcl_over <= 1'b1;
            opcode2 <= opcode1;
            sign_mcl <= sign_k ^ sign_x;
            exp_mcl <= exp_k + exp_x - 7'd15;
            rm_mcl <= rm_k * rm_x;
            sign_b2 <= sign_b;
            exp_b2 <= exp_b;
            rm_b2 <= rm_b;
        end
        else    begin                           //idle
            mcl_over <= 1'b0;
            opcode2 <= opcode2;
            sign_mcl <= sign_mcl;
            exp_mcl <= exp_mcl;
            rm_mcl <= rm_mcl;
            sign_b2 <= sign_b2;
            exp_b2 <= exp_b2;
            rm_b2 <= rm_b2;
        end
    end
    
    //step3 : carry1
    reg carry1_over;
    reg [1:0]opcode3;
    reg sign_mcl2;
    reg signed[6:0]exp_mcl2;
    reg [21:0]rm_mcl2;
    reg sign_b3;
    reg signed[6:0]exp_b3;
    reg [10:0]rm_b3;
    
    always@(posedge clk)    begin
        if(rst) begin                           //reset
            carry1_over <= 1'b0;
            opcode3 <= 2'b0;
            sign_mcl2 <= 1'b0;
            exp_mcl2 <= 7'd0;
            rm_mcl2 <= 22'd0;
            sign_b3 <= 1'b0;
            exp_b3 <= 7'd0;
            rm_b3 <= 11'd0;
        end
        else if(mcl_over)   begin                   //work
            carry1_over <= 1'b1;
            opcode3 <= opcode2;
            rm_b3 <= rm_b2;
            exp_b3 <= exp_b2;
            sign_mcl2 <= sign_mcl;
            sign_b3 <= sign_b2;
            if(rm_mcl[21])  begin
                rm_mcl2 <= rm_mcl >> 1;
                exp_mcl2 <= exp_mcl + 7'd1;
            end
            else    begin
                rm_mcl2 <= rm_mcl;
                exp_mcl2 <= exp_mcl;
            end
        end
        else    begin                               //idle
            carry1_over <= 1'b0;
            opcode3 <= opcode3;
            sign_mcl2 <= sign_mcl2;
            exp_mcl2 <= exp_mcl2;
            rm_mcl2 <= rm_mcl2;
            sign_b3 <= sign_b3;
            exp_b3 <= exp_b3;
            rm_b3 <= rm_b3;
        end
    end
    
    //step4 : compare
    reg compare_over;
    reg [1:0]opcode4;
    reg sign_h;
    reg signed[6:0]exp_h;
    reg [32:0]rm_h;
    reg [32:0]rm_l;
    reg addorsub;
    reg [4:0]shift_bits;
    
    always @(posedge clk)   begin
        if(rst) begin
            compare_over <= 1'b0;
            opcode4 <= 2'b0;
            sign_h <= 1'b0;
            exp_h <= 7'd0;
            rm_h <= 33'd0;
            rm_l <= 33'd0;
            addorsub <= 1'b0;
            shift_bits <= 5'd0;
        end
        else if(carry1_over)    begin
            compare_over <= 1'b1;
            opcode4 <= opcode3;
            addorsub <= sign_mcl2 ^ sign_b3;    //1=sub,0=add
            if((exp_b3 == 7'd0) || (exp_mcl2 > exp_b3)) begin
                sign_h <= sign_mcl2;
                exp_h <= exp_mcl2;
                shift_bits <= exp_mcl2 - exp_b3;
                rm_h <= {1'b0,rm_mcl2[20:0],11'd0};
                rm_l <= {1'b0,rm_b3,21'd0};
            end
            else    begin
                sign_h <= sign_b3;
                exp_h <= exp_b3;
                shift_bits <= exp_b3 - exp_mcl2;
                rm_h <= {1'b0,rm_b3,21'd0};
                rm_l <= {1'b0,rm_mcl2[20:0],11'd0};
            end
        end
        else    begin
            compare_over <= 1'b0;
            opcode4 <= opcode4;
            sign_h <= sign_h;
            exp_h <= exp_h;
            rm_h <= rm_h;
            rm_l <= rm_l;
            addorsub <= addorsub;
            shift_bits <= shift_bits;
        end
    end
    
    //step5:shift to align
    reg shift_over;
    reg [1:0]opcode5;
    reg sign_h2;
    reg signed[6:0]exp_h2;
    reg [32:0]rm_h2;
    reg [32:0]rm_l2;
    reg addorsub2;
    
    always@(posedge clk)    begin
        if(rst) begin                           //rst
            shift_over <= 1'b0;
            opcode5 <= 2'b0;
            sign_h2 <= 1'b0;
            exp_h2 <= 7'd0;
            rm_h2 <= 33'd0;
            rm_l2 <= 33'd0;
            addorsub2 <= 1'b0;
        end
        else    if(compare_over)    begin       //work
            shift_over <= 1'b1;
            opcode5 <= opcode4;
            sign_h2 <= sign_h;
            exp_h2 <= exp_h;
            rm_h2 <= rm_h;
            addorsub2 <= addorsub;
            case(shift_bits)
                5'd0 : rm_l2 <= rm_l;
                5'd1 : rm_l2 <= rm_l >> 1;
                5'd2 : rm_l2 <= rm_l >> 2;
                5'd3 : rm_l2 <= rm_l >> 3;
                5'd4 : rm_l2 <= rm_l >> 4;
                5'd5 : rm_l2 <= rm_l >> 5;
                5'd6 : rm_l2 <= rm_l >> 6;
                5'd7 : rm_l2 <= rm_l >> 7;
                5'd8 : rm_l2 <= rm_l >> 8;
                5'd9 : rm_l2 <= rm_l >> 9;
                5'd10 : rm_l2 <= rm_l >> 10;
                5'd11 : rm_l2 <= rm_l >> 11;
                5'd12 : rm_l2 <= rm_l >> 12;
                5'd13 : rm_l2 <= rm_l >> 13;
                5'd14 : rm_l2 <= rm_l >> 14;
                5'd15 : rm_l2 <= rm_l >> 15;
                5'd16 : rm_l2 <= rm_l >> 16;
                5'd17 : rm_l2 <= rm_l >> 17;
                5'd18 : rm_l2 <= rm_l >> 18;
                5'd19 : rm_l2 <= rm_l >> 19;
                5'd20 : rm_l2 <= rm_l >> 20;
                5'd21 : rm_l2 <= rm_l >> 21;
                default : rm_l2 <= 33'd0;
            endcase
        end
        else    begin
            shift_over <= 1'b0;
            opcode5 <= opcode5;
            sign_h2 <= sign_h2;
            exp_h2 <= exp_h2;
            rm_h2 <= rm_h2;
            rm_l2 <= rm_l2;
            addorsub2 <= addorsub2;
        end
    end
    
    //step6 : add
    reg add_over;
    reg [1:0]opcode6;
    reg sign_add;
    reg signed[6:0]exp_add;
    reg [33:0]rm_add;                               //MSB is sign
    
    always@(posedge clk)    begin
        if(rst) begin                               //rst
            add_over <= 1'd0;
            opcode6 <= 2'b0;
            sign_add <= 1'b0;
            exp_add <= 7'd0;
            rm_add <= 33'd0;
        end
        else    if(shift_over)  begin               //work
            add_over <= 1'b1;
            opcode6 <= opcode5;
            sign_add <= sign_h2;
            exp_add <= exp_h2;
            if(addorsub2)
                rm_add <= rm_h2 - rm_l2;
            else
                rm_add <= rm_h2 + rm_l2;
        end
        else    begin
            add_over <= 1'b0;
            opcode6 <= opcode6;
            sign_add <= sign_add;
            exp_add <= exp_add;
            rm_add <= rm_add;
        end
    end
    
    //step7:abs
    reg [32:0] rm_add2;
    reg [6:0] exp_add2;
    reg sign_add2;
    reg abs_over;
    reg [1:0]opcode7;
    always @(posedge clk)   begin
        if(rst) begin
            abs_over <= 1'b0;
            opcode7 <= 2'd0;
            exp_add2 <= 0;
            rm_add2 <= 33'd0;
            sign_add2 <= 1'b0;
        end
        else if(add_over)   begin
            abs_over <= 1'b1;
            opcode7 <= opcode6;
            exp_add2 <= exp_add;
            if(rm_add[33])  begin//-
                rm_add2 <= 1 + (~rm_add);
                sign_add2 <= ~sign_add;
            end
            else    begin
                rm_add2 <= rm_add;
                sign_add2 <= sign_add;
            end
        end
        else    begin
            abs_over <= 1'b0;
            opcode7 <= opcode7;
            exp_add2 <= exp_add2;
            rm_add2 <= rm_add2;
            sign_add2 <= sign_add2;
        end
    end
    
    //step8:nor_pre
    reg nor_pre_over;
    reg [1:0]opcode8;
    reg sign_new;
    reg signed[6:0]exp_new;
    reg [11:0]rm_new;
    reg round_flag;
    
    always @(posedge clk)   begin
        if(rst) begin                           //rst
            nor_pre_over <= 1'b0;
            opcode8 <= 2'b0;
            sign_new <= 1'b0;
            exp_new <= 7'd0;
            rm_new <= 12'd0;
            round_flag <= 1'b0;
        end
        else if(abs_over)   begin                   //work
            nor_pre_over <= 1'b1;
            opcode8 <= opcode7;
            sign_new <= sign_add2;
            casex(rm_add2)
                33'b1_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 + 7'd1;
                    rm_new <= rm_add2[32:22];
                    round_flag <= rm_add2[21];
                end
                33'b0_1xxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2;
                    rm_new <= rm_add2[31:21];
                    round_flag <= rm_add2[20];
                end
                33'b0_01xx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd1;
                    rm_new <= rm_add2[30:20];
                    round_flag <= rm_add2[19];
                end
                33'b0_001x_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd2;
                    rm_new <= rm_add2[29:19];
                    round_flag <= rm_add2[18];
                end
                33'b0_0001_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd3;
                    rm_new <= rm_add2[28:18];
                    round_flag <= rm_add2[17];
                end
                33'b0_0000_1xxx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd4;
                    rm_new <= rm_add2[27:17];
                    round_flag <= rm_add2[16];
                end
                33'b0_0000_01xx_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd5;
                    rm_new <= rm_add2[26:16];
                    round_flag <= rm_add2[15];
                end
                33'b0_0000_001x_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd6;
                    rm_new <= rm_add2[25:15];
                    round_flag <= rm_add2[14];
                end
                33'b0_0000_0001_xxxx_xxxx_xxxx_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd7;
                    rm_new <= rm_add2[24:14];
                    round_flag <= rm_add2[13];
                end
                33'b0_0000_0000_1xxx_xxxx_xxxx_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd8;
                    rm_new <= rm_add2[23:13];
                    round_flag <= rm_add2[12];
                end
                33'b0_0000_0000_01xx_xxxx_xxxx_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add - 7'd9;
                    rm_new <= rm_add[22:12];
                    round_flag <= rm_add[11];
                end
                33'b0_0000_0000_001x_xxxx_xxxx_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd10;
                    rm_new <= rm_add2[21:11];
                    round_flag <= rm_add2[10];
                end
                33'b0_0000_0000_0001_xxxx_xxxx_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd11;
                    rm_new <= rm_add2[20:10];
                    round_flag <= rm_add2[9];
                end
                33'b0_0000_0000_0000_1xxx_xxxx_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd12;
                    rm_new <= rm_add2[19:9];
                    round_flag <= rm_add2[8];
                end
                33'b0_0000_0000_0000_01xx_xxxx_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd13;
                    rm_new <= rm_add2[18:8];
                    round_flag <= rm_add2[7];
                end
                33'b0_0000_0000_0000_001x_xxxx_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd14;
                    rm_new <= rm_add2[17:7];
                    round_flag <= rm_add2[6];
                end
                33'b0_0000_0000_0000_0001_xxxx_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd15;
                    rm_new <= rm_add2[16:6];
                    round_flag <= rm_add2[5];
                end
                33'b0_0000_0000_0000_0000_1xxx_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd16;
                    rm_new <= rm_add2[15:5];
                    round_flag <= rm_add2[4];
                end
                33'b0_0000_0000_0000_0000_01xx_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd17;
                    rm_new <= rm_add2[14:4];
                    round_flag <= rm_add2[3];
                end
                33'b0_0000_0000_0000_0000_001x_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd18;
                    rm_new <= rm_add2[13:3];
                    round_flag <= rm_add2[2];
                end
                33'b0_0000_0000_0000_0000_0001_xxxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd19;
                    rm_new <= rm_add2[12:2];
                    round_flag <= rm_add2[1];
                end
                33'b0_0000_0000_0000_0000_0000_1xxx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd20;
                    rm_new <= rm_add2[11:1];
                    round_flag <= rm_add2[0];
                end
                33'b0_0000_0000_0000_0000_0000_01xx_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd21;
                    rm_new <= rm_add2[10:0];
                    round_flag <= 1'b0;
                end
                33'b0_0000_0000_0000_0000_0000_001x_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd22;
                    rm_new <= rm_add2[9:0] << 1;
                    round_flag <= 1'b0;
                end
                33'b0_0000_0000_0000_0000_0000_0001_xxxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd23;
                    rm_new <= rm_add2[9:0] << 2;
                    round_flag <= 1'b0;
                end
                33'b0_0000_0000_0000_0000_0000_0000_1xxx_xxxx : begin
                    exp_new <= exp_add2 - 7'd24;
                    rm_new <= rm_add2[9:0] << 3;
                    round_flag <= 1'b0;
                end
                default : begin
                    exp_new <= 12'd0;
                    rm_new <= 7'd0;
                    round_flag <= 1'b0;
                end
            endcase
        end
        else    begin                           //idle
            nor_pre_over <= 1'b0;
            opcode8 <= opcode8;
            sign_new <= sign_new;
            exp_new <= exp_new;
            rm_new <= rm_new;
            round_flag <= round_flag;
        end
    end
    
    //step9 : carry2
    reg carry2_over;
    reg [1:0]opcode9;
    reg sign_new2;
    reg signed [6:0]exp_new2;
    reg [11:0]rm_new2;
    reg [11:0]rm_new2_cache;
    reg round_cache;
    
    always@(posedge clk)    begin
        if(rst) begin                       //rst
            carry2_over <= 1'b0;
            opcode9 <= 2'd0;
            sign_new2 <= 1'b0;
            exp_new2 <= 7'd0;
            rm_new2 <= 12'd0;
            
        end
        else if(nor_pre_over)   begin           //work
            carry2_over <= 1'b1;
            sign_new2 <= sign_new;
            opcode9 <= opcode8;
            if((exp_new < 1)/* || exp_new[6]*/) begin               //denormal
                case(exp_new)
                    0 : begin
                        round_cache = rm_new[0];
                        rm_new2_cache = rm_new >> 1;
                    end
                    -1 : begin
                        round_cache = rm_new[1];
                        rm_new2_cache = rm_new >> 2;
                    end
                    -2 : begin
                        round_cache = rm_new[2];
                        rm_new2_cache = rm_new >> 3;
                    end
                    -3 : begin
                        round_cache = rm_new[3];
                        rm_new2_cache = rm_new >> 4;
                    end
                    -4 : begin
                        round_cache = rm_new[4];
                        rm_new2_cache = rm_new >> 5;
                    end
                    -5 : begin
                        round_cache = rm_new[5];
                        rm_new2_cache = rm_new >> 6;
                    end
                    -6 : begin
                        round_cache = rm_new[6];
                        rm_new2_cache = rm_new >> 7;
                    end
                    -7 : begin
                        round_cache = rm_new[7];
                        rm_new2_cache = rm_new >> 8;
                    end
                    -8 : begin
                        round_cache = rm_new[8];
                        rm_new2_cache = rm_new >> 9;
                    end
                    -9 : begin
                        round_cache = rm_new[9];
                        rm_new2_cache = rm_new >> 10;
                    end
                    -10 : begin
                        round_cache = rm_new[10];
                        rm_new2_cache = rm_new >> 11;
                    end
                    default : begin
                        round_cache = 1'b0;
                        rm_new2_cache = 12'd0;
                    end
                endcase
                rm_new2 <= round_cache ? rm_new2_cache + 1 : rm_new2_cache;
                if((rm_new2_cache == 12'b001111111111) && round_cache)  //denormal to normal
                    exp_new2 <= 7'd1;
                else
                    exp_new2 <= 7'd0;
            end   
            else                      begin              //normal
                rm_new2_cache = round_flag ? rm_new + 1 : rm_new;
                if(rm_new2_cache[11])   begin
                    rm_new2 <= rm_new2_cache >> 1;
                    exp_new2 <= exp_new + 7'd1;
                end
                else    begin
                    rm_new2 <= rm_new2_cache;
                    exp_new2 <= exp_new;
                end
            end
        end 
        else    begin
            carry2_over <= 1'b0;
            opcode9 <= opcode9;
            sign_new2 <= sign_new2;
            exp_new2 <= exp_new2;
            rm_new2 <= rm_new2;
        end
    end
    
    
    //step10 : output
    always@(posedge clk)    begin
        if(rst) begin                       //rst
            data_o <= 16'd0;
            output_up <= 1'b0;
            opcode_o <= 1'b0;
        end
        else if(carry2_over)    begin           //work
            output_up <= 1'b1;
            opcode_o <= opcode9;
            if(exp_new2 > 30)                //overflow
                data_o <= {sign_new2,15'h7bff};
            else if(rm_new2 == 12'd0)           //0
                data_o <= 16'h0000;
            else
                data_o <= {sign_new2,exp_new2[4:0],rm_new2[9:0]};
        end
        else    begin                   //idle
            data_o <= data_o;
            output_up <= 1'b0;
            opcode_o <= opcode_o;
        end
    end
    
endmodule
