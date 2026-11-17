# tcp-echo

The no-OS network code on CAPI: `no_os_lwip_init()` brings up lwIP on the
board's Ethernet MAC through `lwip_socket`, and a `tcp_socket` server on
port 10000 sends back whatever a client writes.

The address is static, `192.168.10.10/24`, set with the no-OS network
Kconfig values in `CMakeLists.txt` (`CONFIG_NO_OS_IP`, `CONFIG_NO_OS_NETMASK`,
`CONFIG_NO_OS_GATEWAY`). Leave `CONFIG_NO_OS_IP` unset to use DHCP, falling
back to a 169.254.x.x address after about 20 s. The board also answers mDNS
as `analog.local`.

`src/main.c` does not depend on the MAC. `boards/<BOARD>/` holds it:

| File | What it holds |
|---|---|
| `board.c` | `board_eth_mac_config()`: set up the MAC's bus and pins, then fill in the `capi_eth_mac_init_config` that the netdev passes to `capi_eth_mac_init()` |
| `board.cmake` | the sources and packages the board's MAC needs |
| `board.conf` | the Kconfig the board's MAC needs |

| Board | MAC | Preset |
|---|---|---|
| EVAL-ADIN1110EBZ | ADIN1110 on SPI2, from the capi-adin1110 package (no-OS-adin) | `eval-adin1110ebz` |

## Build and run

With `CAPI_WORKSPACE` and `CAPI_DIR` set as in capi's README, and the
platform's own setup done:

```bash
cmake --preset eval-adin1110ebz
cmake --build build-stm32/eval-adin1110ebz --target flash
```

Put the link partner's host interface in the same subnet, for example
`192.168.10.1/24`, then:

```bash
ping 192.168.10.10
nc 192.168.10.10 10000
```
