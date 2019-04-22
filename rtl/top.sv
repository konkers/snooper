module top (
	input logic snes_clk,
	input logic snes_latch,
	input logic snes_data,


    output logic valid,
    output [11:0] out
	);
	
    wire [15:0] state;

    snooper snooper(
        .snes_clk(snes_clk),
        .snes_latch(snes_latch),
        .snes_data(snes_data),
        .state(state),
        .valid(valid)
    );

    always_comb begin
        out = state[11:0];
    end
endmodule: top