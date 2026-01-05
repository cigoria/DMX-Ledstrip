# DMX RGB LED Strip Controller

## Hardver összetevők

A projekt megépítéséhez az alábbi főbb alkatrészekre van szükség:

* **Arduino nano:** [Aliexpress link](https://www.aliexpress.com/item/1005007475356474.html) - Arduino Nano v3 usb-c
* **RGB LED Szalag:** [Aliexpress link](https://www.aliexpress.com/item/1005006766819624.html) - WS2813 5V 2m 60led IP30 Black

---

## DMX Csatorna Kiosztás

A rendszer két választható üzemmódot támogat a rugalmas vezérlés érdekében.

### "A" Üzemmód: 6-Csatornás Funkció (Teljes kontroll)
| Csatorna | Funkció | Értéktartomány |
| :--- | :--- | :--- |
| **CH1** | Piros (Red) | 0 - 255 |
| **CH2** | Zöld (Green) | 0 - 255 |
| **CH3** | Kék (Blue) | 0 - 255 |
| **CH4** | Fő fényerő (Master Dimmer) | 0 - 255 |
| **CH5** | Villogás (Strobe) | 0 (Ki) - 255 (Gyors) |
| **CH6** | Mode (Speciális effektek) | Lásd az "[Effektek](#Beépített-Effektek)" részt |

### "B" Üzemmód: 5-Csatornás Funkció (Szegmens vezérlés)
| Csatorna | Funkció | Értéktartomány |
| :--- | :--- | :--- |
| **CH1** | LED választó / Szegmens | 0 - 255 |
| **CH2** | Piros (Red) | 0 - 255 |
| **CH3** | Zöld (Green) | 0 - 255 |
| **CH4** | Kék (Blue) | 0 - 255 |
| **CH5** | Fő fényerő (Master Dimmer) | 0 - 255 |

---

## Beépített Effektek

A 6-csatornás mód (CH6) aktiválásával az alábbi programok futtathatók:

1.  **RGB Szivárvány:** Lassú, folyamatos színátmenet a teljes skálán (mindegyik Led más színű).
2.  **Instant Színváltás:** Hirtelen, ugrásszerű váltás az alapszínek között.
3.  **Fade Színváltás:** Lágy elhalványulással kísért automatikus színváltás.

---
