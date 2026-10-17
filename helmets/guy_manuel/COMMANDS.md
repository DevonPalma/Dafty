# Guy Manuel BLE controls

Guy Manuel is controlled **entirely through BLE**. Thomas retains his physical button.

Send ASCII text to BLE device `Dafty_Marquee`, characteristic `19B10001-E8F2-537E-4F6C-D104768A1214` (up to 63 printable characters per write).

| Send | Action |
|---|---|
| `&Idle` | Resume Dream → Spectrum → Sweep automatic cycling |
| `&Dream`, `&Spectrum`, `&Sweep` | Hold the selected ambient look |
| `&Party`, `&Chaos`, `&Alert` | Hold the selected energetic look |
| `Hello!` | Scroll in cyan, then resume previous base look |
| `$Red *Static HELLO` | Show centered red text briefly, then resume previous look |
| `$Purple &Party *Scroll PARTY!` | Scroll in purple, then hold Party |
| `^blink`, `^wink` | One-shot facial expression, then resume previous look |
| `^heart_eyes`, `^hearts` | One-shot heart eyes |
| `^heartbeat`, `^glitch`, `^work_it` | One-shot performance animations |
| `^angry_eyes`, `^sleep`, `^loading` | Persistent expression until the next BLE command |

**Syntax:** `$Color &NextState *DrawAnimation message` (prefixes optional and order-independent), or `&State` by itself, or `^special_name`.

**Text drawing:** `*Scroll` (default), `*Static` / `*Hold`, `*Blink`. Colors: Red, Green, Blue, Cyan, Purple, Yellow, White, Black, Orange, Pink.

**Rules:** Text and temporary special animations resume the last selected base state. An explicit `&NextState` after text changes that base state. New BLE commands interrupt ongoing messages or special effects. Unknown commands are ignored and don't interrupt the current show. There is no command queue.