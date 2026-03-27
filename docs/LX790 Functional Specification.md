# Functional Specification Document (FSD) - LX790 WiFi Extension

## 1. Einleitung
Dieses Dokument beschreibt die funktionalen Anforderungen für das LX790 WiFi Erweiterungsprojekt. Das Ziel des Projekts ist es, einen LandXcape LX79x Mähroboter mit einer WLAN-Schnittstelle nachzurüsten, um ihn aus der Ferne überwachen und steuern zu können.

## 2. Zielsetzung
- Überwachung des aktuellen Status des Mähroboters (Display-Inhalt, Batteriestand, Fehlermeldungen).
- Fernsteuerung der Tastenfunktionen des Roboters.
- Integration in Smart-Home-Systeme (z.B. FHEM) über HTTP-Schnittstellen.
- Einfache Konfiguration und Firmware-Updates über WLAN.

## 3. Funktionale Anforderungen

### 3.1 Status-Überwachung
- **Display-Replikation**: Das System muss den Inhalt des 7-Segment-Displays des Roboters auslesen und in Echtzeit auf einer Weboberfläche anzeigen.
- **Symbole**: Anzeige von Statussymbolen wie Uhr und PIN-Sperre. (WLAN Status wird nicht angezeigt, da eine Web Verbindung impliziert funktionierendes WLAN voraussetzt.)
- **Batterie**: Anzeige des Batteriestands (Leer, Niedrig, Mittel, Voll, Laden).
- **Klartext-Status**: Übersetzung von Fehlercodes (z.B. -E1-, -E8-) in menschenlesbare Texte.

### 3.2 Fernsteuerung
- **Virtuelle Tasten**: Simulation von Tastendrücken für Start, Home, OK und Stop.
- **Vordefinierte Aktionen**:
    - "Mähen starten"
    - "Zur Ladestation zurückkehren"
- **Konfigurations-Befehle**: Einstellen des Arbeitsbereichs, von Zeit und Datum sowie Vergabe eines neuen PINs über die Weboberfläche.

### 3.3 Konnektivität
- **WLAN-Modi**:
    - **Client-Modus**: Verbindung zu einem bestehenden Heimnetzwerk.
    - **Captive Portal (Access Point)**: Eigenes WLAN zur Erstkonfiguration, falls kein Netzwerk gefunden wird oder ein Timeout auftritt.
- **Web-Interface**: Bereitstellung einer responsiven Webseite zur Interaktion.
- **mDNS Support**: Erreichbarkeit über einen Hostnamen (z.B. `http://lx790.local`).

### 3.4 System-Konfiguration
Das Gerät verfügt über eine dedizierte Konfigurationsseite (`config.html`) für grundlegende Systemeinstellungen:
- **Geräteeinstellungen**:
    - **Name**: Festlegung des Hostnamens für mDNS und die Anzeige in der Weboberfläche.
    - **PIN**: Hinterlegung des 4-stelligen Geräte-PINs (erforderlich für die Fernsteuerung und Auto-Unlock Funktionen).
- **WLAN-Client-Modus**:
    - **SSID**: Name des Ziel-WLAN-Netzwerks.
    - **Passwort**: Sicherheitsschlüssel für den Netzwerkzugriff.
- **Captive Portal (Access Point Modus)**:
    - **Aktivierung**: Option zum Ein- oder Ausschalten des eigenen Access Points.
    - **Portal-Passwort**: Kennwort für den direkten Zugriff auf den ESP32-AP.
    - **Timeout**: Zeitspanne in Sekunden, nach der das Portal automatisch deaktiviert wird.

### 3.5 Betriebs- & Systemeinstellungen
Direkt über die Hauptoberfläche (`index.html`) können im Bereich "Einstellungen" folgende Parameter beeinflusst werden:
- **Auto-Unlock**: Optionale automatische PIN-Eingabe durch den ESP32, wenn der Roboter gesperrt ist (nur bei bestehender WLAN-Verbindung).
- **Mäher-Konfiguration**: Schnelleinstieg für Arbeitsbereich, Zeit/Datum, PIN-Änderung und Startzeit.
- **System-Wartung**:
    - **Reboot**: Neustart des ESP32-Moduls.
    - **OTA-Updates**: Möglichkeit, die Firmware (.bin Datei) über den Browser zu aktualisieren.
    - **Debug-Logging**: Aktivierung eines erweiterten Fehler-Loggings mit Einsicht in das Debug-Log über ein separates Interface.

## 4. Benutzeroberfläche (Web UI)
- **Hauptansicht**: Zeigt das replizierte Display, Statusmeldungen und die wichtigsten Steuerungstasten.
- **Konfigurationsbereiche**: 
    - **System**: Grundlegende WLAN- und Hostname-Einstellungen.
    - **Betrieb**: Auto-Unlock, Debug-Logging und System-Reboot.
- **Statusanzeige**: Detaillierte JSON-basierte Statusinformationen für Debugging-Zwecke.

## 5. Nicht-funktionale Anforderungen
- **Sicherheit**: Der Einsatz erfolgt auf eigene Gefahr; keine offizielle Unterstützung durch den Hersteller.
- **Echtzeitfähigkeit**: Die Display-Daten müssen mit minimaler Verzögerung übertragen werden, um das Timing des Roboters nicht zu stören.
- **Zuverlässigkeit**: Automatischer Wiederverbindungsversuch bei WLAN-Abbruch.
