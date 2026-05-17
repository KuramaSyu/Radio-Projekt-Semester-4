# Softwareentwicklung

Als Entwicklungsumgebung wurde sich für Visual Studio Code mit der „Platform IO“-Extension entschieden. Das Programm wurde in C geschrieben und ist modular aufgebaut. Es wurde sich **gegen das Arduino Framework und stattdessen für ESP-IDF entschieden**, wodurch die verwendeten Bibliotheken reduziert wurden auf:
- offizielle ESP-IDF Bibliotheken welche mit der Installation vom Espressiv-Installation-Manager "EIM" verfürbar sind (siehe: [offizieller Espressiv-Installationsguide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/windows-setup.html)). Das Espressiv-Framework ist verglichen mit Arduino sehr minimalistisch. Driver für I2C und das Ansteuern der GPIO-Pins sind verfügbar; Driver spezifischer Teile, wie ein Push-Button, Display oder Radiotuner mussten selbst umgesetzt werden.
- `stdio.h` verwendet für Input/Output und somit String-Formatierunen. Vorallem für das Display-Output verwendet
- `math.h` verwendet für Float-Operationen


### Codestruktur
Der folgende Codeblock stellt den Aufbau des Projektes dar, einschließlich wichtiger Dateien, aber nicht aller Dateien.
```
.
├── README.md
├── include
│   ├── app_state.h
│   ├── config.h
│   ├── ... header files for all drivers
└── src
    ├── drivers
    ├── interrupts.c
    ├── main.c
    ├── timers.c
    └── view
```
#### Ordner
- `src/`: Ordner, in welchem die Implementierungen aller Komponenten und der Startpunkt des Programms (`src/main.c`) liegen
- `src/view`: Enthält Methoden, welche für die Display-Ausgabe verwendet werden (primär String-Formatierungen)
- `src/drivers`: Enthält die Driver für alle verwendeten Komponenten, da es in der Espressiv-IDF nur sehr wenige fertige Driver gibt
- `include/`: Zentraler Ordner für alle Header-Dateien. Diese definieren die Methoden der dazugehörigen `.c` Dateien und **enthalten die Doc-Strings der Methoden**. Es sind nur diejenigen Methoden definiert, welche auch außerhalb der jeweiligen `.c` Datei zu finden sind, also keine privaten Methoden sind.

#### Wichtige Dateien
- `src/main.c`: Der Startpunkt des Programms. Hier werden alle Komponenten initialisiert (Display, Radio-Tuner, ADC für das Potentiometer, GPIO-Konfiguration für den Push-Button). Nach der Initialisierung wird ein Timer und einen Interrupt registriert. Diese sind für den Program-Ablauf verantwortlich. Mehr dazu in `src/timers.c` und `src/interrupts.c`
- `src/timers.c`: Enthält das Callback (`pot_timer_task`) für den in der Main-Funktion registrierten Timer. Dieser überprüft den Potentiometer-Wert und den Zustand des Programms (Automatisch/Manuell) und aktualisiert anschließend die Radiofrequenz und die Display-Ausgabe. Mehr dazu in Abschnitt "Programmablauf"
- `src/interrupts.c`: Enthält den Callback für die, in `src/main.c:main` registrierten, Interrupt-Service-Routine (ISR). Diese löst aus, sobald der Push-Button betätigt wird (`src/drivers/button.c:button_init`). Der Kopfdruck ändert den Zustand des Programms zwischen automatisch und manuell. Da der Interrupt keine Queue nutzt, sondern direkt abläuft, muss dieser minimalistisch und kurz sein. Daher wird eine globale Variable `machine_state` geändert, welche vom, alle 100ms laufenden, Timer auf Änderung überprüft wird.
- `include/app_state.h`: Definiert ein Enum und die globalen Variablen für den Zustand. Dieser ist entweder `STATE_MANUAL` oder `STATE_HALF_AUTO`.
- `include/config.h`: Enthält Definitionen für die verwendeten GPIO-Pins, ADCs und I2C-Adressen.


### Programmablauf
Es wurde sich gegen einen klassischen while-Loop entschieden, stattdessen wird in der Main-Funktion ein Timer registriert, welcher alle 100ms ausgelöst wird und gegebenenfalls ein Event für Potentiometer-Änderung auslöst (src/timers.c:on_pot_change Notation: dateipfad:methode).

#### Auslösen der Display Updates
Das Display wird unter folgenden Bedingungen aktuallisiert:
- Das Potentiometer wurde stark genug verändert ODER
- Der verwendeten Modus wurde verändert (Manuel / Automatisch)
Für das Radio wurden zwei unterschiedliche Modi implementiert: der automatische Modus, bei dem nur zwischen fest definierten Sendern/Frequenzen umgeschalten werden kann und der freie Modus, bei dem beliebige Frequenzen eingestellt werden können. Beide Modi werden im Folgenden näher betrachtet.

#### Automatischer Modus
Der automatische Modus bietet die Möglichkeit zwischen festgelegten Sendern zu wechseln und dabei das störende Rauschen „zwischen“ den Sendern zu überspringen. Folgende Sender werden dabei unterstützt:


| Radiofrequenz | Sendername |
| --- | --- |
| 89.2 | R.SA |
| 90.1 | MDR Jump |
| 92.2 | MDR Sachsen |
| 95.4 | MDR Kultur |
| 97.3 | Deutschlandfunk |
| 100.2 | ENERGY Dresden |
| 102.4 | Radio PSR |
| 103.5 | Radio Dresden |
| 105.2 | HitRadio RTL |

Das Wechseln der Radiofrequenz und damit des Senders erfolgt durch Bedienung des Drehreglers des Potentiometers.

Der folgende Programmablaufplan gibt einen Überblick über den Ablauf der Main-Funktion sowie der Timer-Funktion:


