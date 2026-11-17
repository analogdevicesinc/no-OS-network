# iperf-tcp

lwIP's `lwiperf` on CAPI: `no_os_lwip_init()` brings up lwIP on the board's
Ethernet MAC through `lwip_socket`, and `lwiperf` serves iperf2 TCP tests on
port 5001. The console reports each finished test with its byte count,
duration and bandwidth. `lwiperf` runs on the lwIP raw API, not on the
`tcp_socket` API.

The address is static, `192.168.10.10/24`, set with the no-OS network
Kconfig values in `CMakeLists.txt`, as in [tcp-echo](../tcp-echo/README.md).

`src/main.c` does not depend on the MAC; `boards/<BOARD>/` holds it, as in
tcp-echo:

| Board | MAC | Preset |
|---|---|---|
| EVAL-ADIN1110EBZ | ADIN1110 on SPI2, from the no-os-adin package (no-OS-adin) | `eval-adin1110ebz` |

## Build and run

As for tcp-echo:

```bash
cmake --preset eval-adin1110ebz
cmake --build build-stm32/eval-adin1110ebz --target flash
```

Then run an iperf2 client (iperf 2.x, not iperf3) on the link partner,
from it to the board:

```bash
iperf -C -c 192.168.10.10 -t 10
```

`lwiperf` speaks the iperf 2.0 protocol; `-C` keeps iperf 2.1 and later to
it. Their `-r` and `-d` are not available in that mode, so for the other
direction the board is the client: run `iperf -s` on the link partner and
build with its address,

```bash
cmake --preset eval-adin1110ebz -DIPERF_CLIENT_TO=192.168.10.1
```

and the board sends it one 10 s test after it starts.
