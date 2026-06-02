module packet_router #(
    parameter int DATA_WIDTH = 16
) (
    input  logic                         clk,
    input  logic                         rst,
    input  logic [1:0]                   in_valid,
    output logic [1:0]                   in_ready,
    input  logic [1:0]                   in_dest,
    input  logic [(2*DATA_WIDTH)-1:0]    in_data,
    output logic [1:0]                   out_valid,
    input  logic [1:0]                   out_ready,
    output logic [1:0]                   out_src,
    output logic [(2*DATA_WIDTH)-1:0]    out_data
);
    logic [1:0] grant;
    logic [1:0] free_out;
    logic [DATA_WIDTH-1:0] in_word [2];
    logic [DATA_WIDTH-1:0] out_word [2];

    assign in_word[0] = in_data[DATA_WIDTH-1:0];
    assign in_word[1] = in_data[(2*DATA_WIDTH)-1:DATA_WIDTH];
    assign out_data = {out_word[1], out_word[0]};

    always_comb begin
        free_out = (~out_valid) | out_ready;

        grant[0] = in_valid[0] & free_out[in_dest[0]];
        grant[1] = in_valid[1] & free_out[in_dest[1]];

        if (grant[0] && grant[1] && (in_dest[0] == in_dest[1])) begin
            grant[1] = 1'b0;
        end

        in_ready = grant;
    end

    always_ff @(posedge clk) begin
        if (rst) begin
            out_valid <= 2'b00;
            out_src <= 2'b00;
            out_word[0] <= '0;
            out_word[1] <= '0;
        end else begin
            for (int out = 0; out < 2; out++) begin
                if (out_valid[out] && out_ready[out]) begin
                    out_valid[out] <= 1'b0;
                end
            end

            for (int src = 0; src < 2; src++) begin
                if (grant[src]) begin
                    out_valid[in_dest[src]] <= 1'b1;
                    out_src[in_dest[src]] <= src[0];
                    out_word[in_dest[src]] <= in_word[src];
                end
            end
        end
    end
endmodule
