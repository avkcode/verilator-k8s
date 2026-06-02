#include "Vpacket_router.h"

#include <array>
#include <cstdint>
#include <cstdlib>
#include <deque>
#include <iostream>
#include <random>
#include <stdexcept>

namespace {

struct Packet {
    uint8_t src;
    uint8_t dest;
    uint16_t data;
};

uint16_t make_data(uint8_t src, uint8_t dest, uint16_t seq) {
    return static_cast<uint16_t>((src << 15) | (dest << 14) | (seq & 0x3fff));
}

uint32_t env_seed() {
    const char *value = std::getenv("SIM_SEED");
    if (value == nullptr || *value == '\0') {
        return 1;
    }
    return static_cast<uint32_t>(std::strtoul(value, nullptr, 10));
}

void tick(Vpacket_router &dut) {
    dut.clk = 1;
    dut.eval();
    dut.clk = 0;
    dut.eval();
}

}  // namespace

int main() {
    const uint32_t seed = env_seed();
    std::mt19937 rng(seed);
    std::bernoulli_distribution inject(0.70);
    std::bernoulli_distribution ready(0.65);
    std::uniform_int_distribution<int> bit(0, 1);

    Vpacket_router dut;
    dut.clk = 0;
    dut.rst = 1;
    dut.in_valid = 0;
    dut.in_dest = 0;
    dut.in_data = 0;
    dut.out_ready = 0;
    dut.eval();

    for (int i = 0; i < 4; ++i) {
        tick(dut);
    }
    dut.rst = 0;

    std::array<bool, 2> pending_valid{false, false};
    std::array<Packet, 2> pending{};
    std::array<uint16_t, 2> next_seq{0, 0};
    std::array<std::deque<Packet>, 2> expected;

    int accepted = 0;
    int delivered = 0;
    int stalls = 0;
    int cycle = 0;

    for (; cycle < 2000; ++cycle) {
        uint8_t valid_mask = 0;
        uint8_t dest_mask = 0;
        uint32_t data_bus = 0;

        for (uint8_t src = 0; src < 2; ++src) {
            if (!pending_valid[src] && inject(rng)) {
                const auto dest = static_cast<uint8_t>(bit(rng));
                pending[src] = Packet{src, dest,
                                      make_data(src, dest, next_seq[src]++)};
                pending_valid[src] = true;
            }

            if (pending_valid[src]) {
                valid_mask |= static_cast<uint8_t>(1u << src);
                dest_mask |= static_cast<uint8_t>(pending[src].dest << src);
                data_bus |= static_cast<uint32_t>(pending[src].data)
                            << (src * 16);
            }
        }

        dut.in_valid = valid_mask;
        dut.in_dest = dest_mask;
        dut.in_data = data_bus;
        dut.out_ready = static_cast<uint8_t>((ready(rng) ? 1u : 0u)
                                             | (ready(rng) ? 2u : 0u));
        dut.eval();

        for (uint8_t out = 0; out < 2; ++out) {
            if (((dut.out_valid >> out) & 1u) &&
                ((dut.out_ready >> out) & 1u)) {
                if (expected[out].empty()) {
                    throw std::runtime_error("unexpected output packet");
                }

                const Packet got{
                    static_cast<uint8_t>((dut.out_src >> out) & 1u),
                    out,
                    static_cast<uint16_t>((dut.out_data >> (out * 16)) &
                                          0xffffu)};
                const Packet want = expected[out].front();
                expected[out].pop_front();

                if (got.src != want.src || got.dest != want.dest ||
                    got.data != want.data) {
                    throw std::runtime_error("scoreboard mismatch");
                }
                ++delivered;
            }
        }

        for (uint8_t src = 0; src < 2; ++src) {
            if (pending_valid[src]) {
                if ((dut.in_ready >> src) & 1u) {
                    expected[pending[src].dest].push_back(pending[src]);
                    pending_valid[src] = false;
                    ++accepted;
                } else {
                    ++stalls;
                }
            }
        }

        tick(dut);
    }

    for (; cycle < 2600; ++cycle) {
        dut.in_valid = 0;
        dut.in_dest = 0;
        dut.in_data = 0;
        dut.out_ready = 3;
        dut.eval();

        for (uint8_t out = 0; out < 2; ++out) {
            if (((dut.out_valid >> out) & 1u) &&
                ((dut.out_ready >> out) & 1u)) {
                if (expected[out].empty()) {
                    throw std::runtime_error("unexpected drain packet");
                }

                const Packet got{
                    static_cast<uint8_t>((dut.out_src >> out) & 1u),
                    out,
                    static_cast<uint16_t>((dut.out_data >> (out * 16)) &
                                          0xffffu)};
                const Packet want = expected[out].front();
                expected[out].pop_front();

                if (got.src != want.src || got.dest != want.dest ||
                    got.data != want.data) {
                    throw std::runtime_error("drain scoreboard mismatch");
                }
                ++delivered;
            }
        }

        tick(dut);

        if (expected[0].empty() && expected[1].empty() &&
            dut.out_valid == 0) {
            break;
        }
    }

    if (!expected[0].empty() || !expected[1].empty() || dut.out_valid != 0) {
        throw std::runtime_error("simulation did not drain");
    }

    std::cout << "PASS packet_router"
              << " seed=" << seed
              << " cycles=" << cycle
              << " accepted=" << accepted
              << " delivered=" << delivered
              << " stalls=" << stalls << '\n';
    return 0;
}
