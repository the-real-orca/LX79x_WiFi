# Funktionale Spezifikation (FSD) - LX790 WiFi Erweiterung

## 1. Übersicht

### 1.1 Zweck
Das LX790 WiFi Erweiterungsprojekt dient der Nachrüstung eines LandXcape LX79x Mähroboters mit einer WLAN-Schnittstelle. Ziel ist es, den Mähroboter aus der Ferne zu überwachen, zu steuern und in bestehende Smart-Home-Systeme zu integrieren.

### 1.2 Umfang
Enthalten sind die Echtzeit-Überwachung des Displays, die Fernsteuerung der Tasten, WLAN-Konnektivität und OTA-Updates. Ausgeschlossen ist der direkte Eingriff in die Motorsteuerung oder Sensordaten des Mähers.

### 1.3 Benutzer / Rollen
- Admin: Konfiguriert WLAN-Einstellungen und führt Firmware-Updates durch.
- Nutzer: Überwacht den Status des Mähers und nutzt die Fernsteuerung.

## 2. Funktionale Zusammenfassung

### 2.1 Hauptablauf / Betriebsmodi
- Normaler Modus: ESP32 ist mit dem Heim-WLAN verbunden und überträgt Daten.
- Konfigurations-Modus: Captive Portal zur Ersteinrichtung bei fehlender Verbindung.
- Debug-Modus: Erweiterte Protokollierung von Hardware-Signalen.

### 2.2 Systemarchitektur
```
┌──────────────────────────────────────────────────────────────────────────┐
│                          Netzwerk (WLAN)                                 │
└──────────────────────────────────────────────────────────────────────────┘
       │                                           │
       ▼                                           ▼
┌────────────────────────┐              ┌────────────────────┐
│  Mähroboter (LX790)    │              │  Benutzeroberfläche│
│  ┌──────────────────┐  │              └────────────────────┘
│  │ Display / Tasten │  │
│  └──────────────────┘  │
│          ▲             │
│          │ Sniffing    │
│  ┌──────────────────┐  │
│  │ ESP32 Erweiterung│  │
│  └──────────────────┘  │
└────────────────────────┘
```

### 2.3 Hardwarebeschreibung
- Zentraleinheit: ESP32 Mikrocontroller.
- Display-Schnittstelle: TM1668 Controller (3-Draht-SPI).
- Tasten-Interface: GPIO-Anbindung zur Simulation von Tastendrücken.

### 2.4 Kernkomponenten
- Display-Decoder: Dekodiert TM1668 Segmente in Klartext.
- Button-Injector: Steuert die GPIOs zur Tasten-Simulation.
- Web-Server: Stellt das Interface und die REST-API bereit.

## 3. Definitionen

### 3.1 Begriffe und Abkürzungen

| Name | Beschreibung |
|------|--------------|
| TM1668 | LED-Controller für Anzeige und Tasten. |
| Sniffing | Passives Mitlesen des Datenbusses. |
| OTA | Firmware-Update über WLAN (Over-the-Air). |

## 4. Funktionale Anforderungen

### 4.1 Status-Überwachung (FR-001)
Das System zeigt den aktuellen Inhalt des 7-Segment-Displays und die Symbole in Echtzeit an.

 - **Auslöser:** kontinuierlich

 - **Verhalten:**
   - Der ESP32 dekodiert die Segmente und Symbole (Batterie, Uhr, Schloss)
   - Der ESP32 aktualisiert den internen System-Status

 - **Erwartetes Ergebnis:**
   - Die Weboberfläche spiegelt den exakten Inhalt des physischen Displays wider

### 4.2 Fernsteuerung der Tasten (FR-002)
Simulation der Tasten Start, Home, OK und Stop.

 - **Auslöser:** Benutzerinteraktion in der Weboberfläche oder API-Aufruf.

 - **Verhalten:**
   - Der entsprechende GPIO wird kurzzeitig als Ausgang (LOW) geschaltet
   - Das System simuliert einen physischen Tastendruck

 - **Erwartetes Ergebnis:**
   - Der Mähroboter reagiert so, als wäre die physische Taste manuell gedrückt worden

### 4.3 Automatisches Entsperren (FR-003)
Automatische Eingabe des PIN-Codes, wenn der Mäher gesperrt ist.

 - **Auslöser:** Detektion des Sperr-Symbols oder des PIN-Eingabe-Displays ("0---").

 - **Verhalten:**
   - Das System prüft ob der Mäher automatisch entsperrt werden soll (`config.json`)
   - Wenn ja, liest es die hinterlegte PIN aus der Konfiguration
   - Das System sendet die PIN als Sequenz virtueller Tastendrücke

 - **Erwartetes Ergebnis:**
   - Der Mäher wechselt automatisch in den entsperrten Zustand

### 4.4 WLAN-Konfiguration (FR-004)
Verwaltung der WLAN-Zugangsdaten und des Captive Portals.

 - **Auslöser:** 
   - Erststart oder fehlende Verbindung zum Netzwerk.
   - press "HOME" button for 10 seconds to start captive portal

 - **Verhalten:**
   - Start des Access Points
   - DNS-Umleitung aller Anfragen zur Konfigurationsseite

 - **Erwartetes Ergebnis:**
   - Nutzer kann neue Zugangsdaten speichern und das System verbindet sich neu

## 5. Nicht-funktionale Anforderungen

### 5.1 Echtzeitfähigkeit (NFR-001)
Die Verzögerung zwischen physischem Display und Web-Anzeige muss minimal sein.

**Erwartetes Ergebnis:**
- Die Latenz der Statusaktualisierung liegt unter 500ms

### 5.2 Zuverlässigkeit (NFR-002)
Das System darf den normalen Betrieb des Mähers bei Fehlfunktion des ESP32 nicht stören.

**Erwartetes Ergebnis:**
- Der Mäher bleibt auch bei Ausfall des ESP32 manuell bedienbar


## 6. Daten
- Quelle: TM1668 Bus via ESP32 Sniffing
- Format: JSON Objekt via /status API
- Speicherung: Konfiguration in config.json auf SPIFFS

## 7. Benutzeroberfläche

### 7.1 Dashboard
Hauptseite zur Überwachung und Steuerung.
- Replikation des Displays
- Steuerungstasten und Makros
- Statusanzeigen für Batterie und System

## 8. Testfälle

### 8.1 Display-Replikation (TC-001)
1. Ändere den Zustand des Mähers (z.B. Stop drücken)
2. Überprüfe die Anzeige in der Weboberfläche

 - **Erfolgskriterien:**
   - Die Weboberfläche zeigt den korrekten Status (z.B. -E1-)

 - **Verknüpfte Anforderungen:** FR-001

### 8.2 Remote-Start (TC-002)
1. Betätige den Button "Mähen starten" in der Weboberfläche

 - **Erfolgskriterien:**
   - Der Mähroboter piept und startet den Mähvorgang

 - **Verknüpfte Anforderungen:** FR-002

## 9. Offene Punkte / Annahmen

### 9.1 Offene Fragen
- Ladeerkennung: Unterscheidung zwischen DOCKED und CHARGING unzuverlässig.
- Auto-Unlock: Validierung bei unterschiedlichen Firmware-Ständen des Mähers.

### 9.2 Annahmen
- Stabile Stromversorgung über die interne 5V Schiene.
- TM1668 Protokoll ist konsistent über die gesamte LX79x Serie.

## Anhang A: Technische Details
Detaillierte Informationen zur Architektur, den Quellcodedateien und der Pin-Belegung finden Sie in der technischen Spezifikation:
[LX790 Technical Specification.md](LX790%20Technical%20Specification.md)

## Anhang B: Implementierungsplan
Den aktuellen Status der Umsetzung sowie geplante Aufgaben finden Sie im Implementierungsstatus:
[LX790 Implementation Status.md](LX790%20Implementation%20Status.md)
