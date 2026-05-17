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
    ├── app_state.c
    ├── drivers
    │   ├── button.c
    │   ├── display.c
    │   ├── potentiometer.c
    │   └── tea5767.c
    ├── helpers.c
    ├── interrupts.c
    ├── main.c
    ├── timers.c
    └── view
        ├── half_automatic.c
        └── manual.c
```
#### Ordner
- `src/`: Ordner, in welchem die Implementierungen aller Komponenten und der Startpunkt des Programms (`src/main.c`) liegen
- `src/view`: Enthält Methoden, welche für die Display-Ausgabe verwendet werden (primär String-Formatierungen)
- `src/drivers`: Enthält die Driver für alle verwendeten Komponenten, da es in der Espressiv-IDF nur sehr wenige fertige Driver gibt
- `include/`: Zentraler Ordner für aller Header-Dateien. Diese definieren die Methoden der dazugehörigen `.c` Dateien und **enthalten die Doc-Strings der Methoden**. Es sind nur diejenigen Methoden definiert, welche auch außerhalb der jeweiligen `.c` Datei zu finden sind, also keine privaten Methoden sind.

#### Wichtige Dateien
- `src/main.c`: Der Startpunkt des Programms. Hier werden alle Komponenten initialisiert (Display, Radio-Tuner, ADC für das Potentiometer, GPIO-Konfiguration für den Push-Button). Nach der Initialisierung wird ein Timer und einen Interrupt registriert. Diese sind für den Program-Ablauf verantwortlich. Mehr dazu in `src/timers.c` und `src/interrupts.c`
- `src/timers.c`: Enthält das Callback (`pot_timer_task`) für den in der Main-Funktion registrierten Timer. Dieser überprüft den Potentiometer-Wert und den Zustand des Programms (Automatisch/Manuell) und aktualisiert anschließend die Radiofrequenz und die Display-Ausgabe. Mehr dazu in Abschnitt "Programmablauf" 


### Programmablauf
Es wurde sich gegen einen klassischen while-Loop entschieden, stattdessen wird in der Main-Funktion ein Timer gestartet, welcher alle 100ms ein Event auslöst.

Für das Radio wurden zwei unterschiedliche Modi implementiert: der automatische Modus, bei dem nur zwischen fest definierten Sendern/Frequenzen umgeschalten werden kann und der freie Modus, bei dem beliebige Frequenzen eingestellt werden können. Beide Modi werden im Folgenden näher betrachtet.

Automatischer Modus
Der automatische Modus bietet die Möglichkeit zwischen festgelegten Sendern zu wechseln und dabei das störende Rauschen „zwischen“ den Sendern zu überspringen. Folgende Sender werden dabei unterstützt:


