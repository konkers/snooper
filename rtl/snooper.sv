module snooper (
	input logic snes_clk,
	input logic snes_latch,
	input logic snes_data,
	output logic [15:0] state,
	output logic valid
	);
	
	logic [3:0] counter;

	always_ff @(negedge snes_clk, posedge snes_latch) begin
		if (snes_latch) begin
			counter <= 0;
		end else	if (~snes_latch & ~snes_clk) begin
			state[counter] = snes_data;
			counter = counter + 1;
		end
	end
	
	always_comb begin
		valid = counter == 4'b0000;
	end
endmodule: snooper