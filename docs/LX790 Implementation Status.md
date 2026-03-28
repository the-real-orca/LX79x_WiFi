# Abgleich FSD vs. Implementierung (LX790 WiFi Extension)

Diese Liste gibt einen Überblick über den Erfüllungsgrad der im Functional Specification Document (FSD) definierten Anforderungen.

## 1. Status-Überwachung
| Funktion | Status | Anmerkung |
|----------|--------|-----------|
| Display-Replikation | **OK** | 7-Segment Dekodierung in `LX790_util.cpp` implementiert. |
| Statussymbole (Uhr, Schloss) | **OK** | Symbole werden im Web UI und im State-Objekt korrekt verarbeitet. |
| WLAN-Status Symbol | **Entfällt** | Gemäß FSD (3.1) nicht angezeigt, da Web-Verbindung WLAN voraussetzt. |
| Batteriestand | **OK** | 4-stufige Anzeige im Web UI integriert. |
| Ladezustand (Laden) | **Fehlerhaft** | In `LX790_util.cpp:261` steht ein `FIXME`, das den Modus hart auf `DOCKED` überschreibt, da die Lade-Erkennung unzuverlässig ist. |
| Klartext-Status (Fehlercodes) | **OK** | Umfangreiche Mapping-Tabelle `LcdToMode` in `LX790_util.cpp`. |

## 2. Fernsteuerung
| Funktion | Status | Anmerkung |
|----------|--------|-----------|
| Virtuelle Tasten (Start, Home, OK, Stop) | **OK** | Alle Tasten über `/cmd?parm=...` erreichbar. |
| Makro "Mähen starten" | **OK** | START + OK Sequenz. |
| Makro "Zur Ladestation" | **OK** | HOME + OK Sequenz. |
| PIN Vergabe (Makro) | **OK** | START + HOME (8s) Makro zum Start der PIN-Änderung am Mäher. |

## 3. Konnektivität & Konfiguration
| Funktion | Status | Anmerkung |
|----------|--------|-----------|
| WLAN Client-Modus | **OK** | In `TaskHW.cpp` und `config.json` implementiert. |
| Captive Portal / AP | **OK** | In `TaskHW.cpp` und `config.json` implementiert. |
| Web-Interface | **OK** | Responsives UI (`index.html`, `config.html`) vorhanden. |
| mDNS Support | **OK** | `MDNS.begin()` mit konfigurierbarem Namen. |

## 3.4 System-Konfiguration (`config.html`)
| Funktion | Status | Anmerkung |
|----------|--------|-----------|
| Gerätename | **OK** | Hostname wird aus `config.json` geladen. |
| PIN-Hinterlegung | **OK** | PIN für Auto-Unlock wird in `config.json` gespeichert. |
| WLAN-Settings | **OK** | SSID/Passwort in `config.json`. |
| Portal-Settings | **OK** | Aktivierung, Passwort und Timeout in `config.json`. |

## 3.5 Betriebs- & Systemeinstellungen (`index.html`)
| Funktion | Status | Anmerkung |
|----------|--------|-----------|
| Auto-Unlock | **Teilweise** | Schalter vorhanden, Speicherung im EEPROM. PIN-Eingabe-Logik experimentell. |
| Mäher-Konfiguration | **OK** | Makros für Arbeitsbereich, Zeit/Datum, PIN-Änderung, Startzeit. |
| Reboot | **OK** | Über `/cmd?parm=reboot` möglich. |
| OTA-Updates | **OK** | Über `/update` implementiert. |
| Dateisystem (Upload) | **OK** | Upload von Web-Ressourcen in der `update.html` integriert. |
| Debug-Logging | **OK** | Schalter vorhanden, Speicherung im EEPROM. Log via `/debuglog`. |

## Liste der fehlenden oder inkorrekt implementierten Funktionen:

1.  **Ladezustands-Erkennung (Kritisch):** Die Unterscheidung zwischen `LX790_DOCKED` (nur in Station) und `LX790_CHARGING` (lädt aktiv) ist unzuverlässig und wird derzeit unterdrückt. (Fokus V1.1)
2.  **Auto-Unlock Robustheit:** Die Logik verlässt sich auf Display-Muster, die je nach Firmware variieren können.
3.  **TM1668 SPI-Sniffing Stabilität (Neu - Hohe Priorität):** Instabilitäten bei blinkenden Anzeigen (z.B. Laden). Frames gehen verloren oder Synchronisation (STB/CS) verschiebt sich.

## Roadmap für spätere Umsetzung

### Task 1: TM1668 Robustheit & SPI-Sniffing (V1.1 Spezifisch)
- **1.1 Rohdaten-Analyse:** Implementierung eines temporären "Raw-Dump"-Modus, um die exakten Byte-Sequenzen während des Blinkens zu loggen.
- **1.2 Synchronisations-Check:** Überprüfung, ob `ESP32SPISlave` korrekt auf die fallende Flanke von `STB` reagiert. Ggf. Hardware-Interrupt am `STB`-Pin.
- **1.3 Buffer-Management:** Evaluierung der `spi_slave_rx_buf_size` und sichere Identifikation des Command-Bytes am Frame-Anfang.
- **1.4 Validitäts-Prüfung:** Einführung einer Validierung für jeden empfangenen Frame.
- **1.5 Persistence Filter (Anti-Flicker):** Softwareseitiger Filter, der Zustände erst nach N stabilen Frames übernimmt.
- **1.6 Blink-Erkennung:** Spezielle Logik für das Batterie-Symbol zur stabilen `LX790_CHARGING` Erkennung.
- **1.7 CPU-Affinität & Interrupts:** Optimierung der Task-Prioritäten und Prüfung von SPI-DMA zur Lastreduzierung.
- **1.8 TM1668 Bit-Mapping:** Identifikation der exakten Bits für die Ladeanimation.
- **1.9 Status-Mapping:** Korrektur der `state.battery`-Logik zur korrekten Stufendarstellung (0-3).

### Task 2: Analyse und Verbesserung der Ladezustands-Erkennung (Kritisch)
Die aktuelle Implementierung in `LX790_util.cpp` überschreibt den Status hart mit `DOCKED`, da die Erkennung unzuverlässig ist.
- **2.1:** Untersuchen der Rohdaten vom TM1668/I2C-Bus während des Ladevorgangs und im gedockten Zustand.
- **2.2:** Identifizieren von Mustern (z.B. blinkendes Batterie-Symbol oder spezifische Spannungsänderungen, falls verfügbar).
- **2.3:** Implementierung einer robusteren Logik in `decodeDisplay` (Version 1.0) bzw. `decodeTM1668` (Version 1.1).
- **2.4:** Entfernen des `FIXME`-Kommentars und Validierung des Zustandsübergangs.

### Task 3: Optimierung der Auto-Unlock Logik
Die Auto-Unlock Logik muss zuverlässiger auf verschiedene Display-Zustände reagieren.
- **3.1:** Testen der PIN-Eingabe-Erkennung bei verschiedenen Startbedingungen (z.B. nach Manuellem Stop vs. nach Einschalten).
- **3.2:** Einführen eines Timeout-Mechanismus für Fehlversuche, um Endlosschleifen bei falscher PIN zu vermeiden.
- **3.3:** Sicherstellen, dass die Logik auch bei zeitversetzter Anzeige (Scrolling) korrekt greift.

### Task 4: Code-Qualität und Stabilität
- **4.1:** Überprüfung der Thread-Sicherheit beim Zugriff auf `LX790_State` zwischen `TaskHW` und `TaskWeb`.
- **4.2:** Implementierung eines Watchdog-Mechanismus für die HW-Kommunikation (besonders I2C Slave in V1.0), um Hänger abzufangen.
- **4.3:** Finalisierung der `@TODO`-Markierungen in `HAL_LX790_V1_0.cpp` bezüglich der Trennung von Read- und Write-Sektionen.

### Task 5: Erweiterung der Web-Oberfläche
- **5.1:** Anzeige der Signalstärke (RSSI) im Web-UI (Daten sind im JSON bereits vorhanden).
- **5.2:** Optionale Anzeige der letzten Log-Einträge direkt auf dem Dashboard (statt nur Download).
- **5.3:** Hinzufügen einer visuellen Rückmeldung (z.B. Spinner), wenn ein Makro (PIN, Startmow) ausgeführt wird.
