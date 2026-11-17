# no-OS-network

The no-OS network code, ported from
[no-OS](https://github.com/analogdevicesinc/no-OS) to
[CAPI](https://github.com/analogdevicesinc/capi): a socket API with an lwIP
backend on any CAPI Ethernet MAC, and CAPI applications that use it.

## Modules

| Directory | Contents | From no-OS |
|---|---|---|
| `socket/` | The socket API applications use (`socket_init()`, `socket_bind()`, `socket_send()`, ...) and the `network_interface` its backends implement | `network/tcp_socket.c/h`, `network/network_interface.h` |
| `lwip_socket/` | The socket backend on lwIP's raw API, with its netdev on a CAPI Ethernet MAC (`netdevs/eth_mac`) | `network/lwip_raw_socket/` |
| `lwip/` | lwIP for bare-metal CAPI applications: fetched at build time at the commit no-OS uses, with no-OS's port and configuration | `libraries/lwip-glue/` |
| `util/` | The no-OS utilities the code above uses (`no_os_util`, `no_os_error`) | `include/`, `util/` |

The code uses CAPI for delays, time, memory allocation and the Ethernet MAC.
The history starts with the unchanged no-OS sources, then replaces the no-OS
APIs with CAPI, then fixes the no-OS bugs it found, one commit each.

The socket API supports TCP. UDP sockets are not supported yet, and neither
are the secure (TLS) sockets, which needed mbedTLS and the no-OS TRNG and are
removed. lwIP itself has UDP: DHCP, AutoIP and the mDNS responder are on.

## Using it

An application adds this directory with `add_subdirectory()`, links
`no_os_network`, and gives the netdev a CAPI Ethernet MAC: its
`capi_eth_mac_init_config`, in `lwip_network_param`. For example, with the
ADIN1110 from [no-OS-adin](https://github.com/analogdevicesinc/no-OS-adin):

```c
struct capi_eth_mac_init_config mac_cfg = {
	.ops = &adin1110_eth_mac_ops,
	.extra = &adin1110_param,
};
struct lwip_network_param lwip_param = {
	.platform_ops = &eth_mac_lwip_ops,
	.mac_param = &mac_cfg,
	.hwaddr = {0x00, 0xE0, 0x22, 0x66, 0xAA, 0x01},
};
struct lwip_network_desc *lwip;

no_os_lwip_init(&lwip, &lwip_param);
for (;;)
	no_os_lwip_step(lwip, NULL);
```

The no-OS network Kconfig values are CMake variables set before
`add_subdirectory()`: `CONFIG_NO_OS_IP`, `CONFIG_NO_OS_NETMASK` and
`CONFIG_NO_OS_GATEWAY` for a static address (DHCP otherwise), and
`CONFIG_NO_OS_DOMAIN_NAME` for the mDNS host name.

## Examples

| Directory | What it does |
|---|---|
| `examples/tcp-echo/` | A TCP echo server on port 10000, on the socket API; see [examples/tcp-echo/README.md](examples/tcp-echo/README.md) |
| `examples/iperf-tcp/` | lwIP's `lwiperf`, an iperf2 TCP server on port 5001, on the lwIP raw API; see [examples/iperf-tcp/README.md](examples/iperf-tcp/README.md) |

Both run on the EVAL-ADIN1110EBZ, with the ADIN1110 from no-OS-adin. On its
10BASE-T1L link, iperf measures about 9 Mb/s each way.

## CAPI setup

You need CMake 3.27+, Ninja, Python 3 and network access. Clone capi and this
repository into one workspace:

```bash
mkdir ~/workspace && cd ~/workspace
git clone https://github.com/analogdevicesinc/capi
git clone https://github.com/analogdevicesinc/no-OS-network
```

```bash
export CAPI_WORKSPACE=~/workspace
export CAPI_DIR=~/workspace/capi
```

The first configure of an example clones its platform (capi-stm32) and its
MAC driver (no-OS-adin) into the workspace; the build fetches lwIP.

## Platform setup

**STM32.** Install STM32CubeIDE and STM32CubeMX; if they are not in the
default paths, set `STM32CUBEIDE` and `STM32CUBEMX`. See
[capi-stm32](https://github.com/analogdevicesinc/capi-stm32) for the
EVAL-ADIN1110EBZ, which needs an external ST-LINK on its SWD header.

The link partner needs a 10BASE-T1L port. A Linux host with a USB adapter
built on a LAN9500 and an ADIN1100 PHY needs the kernel's `adin1100` PHY
driver (`CONFIG_ADIN1100_PHY`): without it the adapter runs half duplex and
drops frames that arrive while it sends. With it, `dmesg` reports the link
as `10Mbps/Full`.

## License

BSD-3-Clause; see [LICENSE](LICENSE).
