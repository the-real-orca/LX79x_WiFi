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

1.  **Ladezustands-Erkennung (Kritisch):** Die Unterscheidung zwischen `LX790_DOCKED` (nur in Station) und `LX790_CHARGING` (lädt aktiv) ist laut Code-Kommentar unzuverlässig und wird derzeit unterdrückt.
2.  **Auto-Unlock Robustheit:** Die Logik verlässt sich darauf, dass das Display im PIN-Modus bestimmte Muster (`0---`) zeigt. Dies könnte bei unterschiedlichen Firmware-Versionen des Roboters variieren.
