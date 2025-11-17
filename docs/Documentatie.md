# Documentație Macropad ESP32-S3 - Proiect de Licență

Acest document descrie în detaliu specificațiile tehnice, arhitectura software și design-ul hardware al proiectului de licență "Macropad ESP32-S3". Acesta este un dispozitiv periferic programabil, conceput pentru a optimiza fluxurile de lucru digitale prin automatizare și personalizare.

---

## 1. Arhitectura Sistemului

Proiectul este structurat în patru domenii inginerești distincte, care colaborează pentru a oferi funcționalitatea completă:

1.  **Hardware (PCB & Mecanică):** Baza fizică a dispozitivului, proiectată în KiCad.
2.  **Firmware (Embedded C++):** Codul care rulează pe microcontrolerul ESP32-S3, gestionând intrările fizice și comunicația.
3.  **Configurator Web (Frontend):** Interfața grafică pentru utilizator, utilizată pentru a defini comportamentul tastelor.
4.  **Host Listener (Backend Desktop):** Serviciul care rulează pe PC pentru a executa comenzi complexe.

## 2. Specificații Hardware

### Componente Principale

- **Microcontroler:** Unexpected Maker ProS3 (ESP32-S3)
  - Dual-core Xtensa LX7 @ 240 MHz
  - Wi-Fi & Bluetooth 5 (LE) nativ
- **Interfață Utilizator:**
  - 12x Taste Mecanice (Matrice 3x4)
  - 1x Encoder Rotativ EC11 (cu buton)
  - 1x Ecran OLED 0.91" (128x32 px, I2C)
- **Conectivitate:** USB-C (Wired) și Bluetooth Low Energy (Wireless).

### Bill of Materials (BOM)

Mai jos este lista completă a componentelor necesare, împărțită pe cele două plăci PCB.

#### Placa Superioară (Top Board - Interfață)

| Componentă              | Cantitate | Specificație      | Amprentă (Footprint) | Funcție                                 |
| :---------------------- | :-------: | :---------------- | :------------------- | :-------------------------------------- |
| **Taste (Keyswitches)** |    12     | Tactile Switch    | `MX_Hotswap_1.00u`   | Intrare utilizator (matrice 3x4).       |
| **Diode**               |    12     | 1N4148            | `D_DO-35_SOD27`      | Protecție anti-ghosting pentru matrice. |
| **Encoder Rotativ**     |     1     | EC11 cu buton     | `Encoder_EC11_MX`    | Navigare meniu și control volum.        |
| **Ecran OLED**          |     1     | SSD1306 128x32    | `OLED_128x32`        | Afișare stare sistem și layer activ.    |
| **Comutator**           |     1     | Slide Switch SPDT | `SLW8645745ARAND`    | Comutare mod / On-Off.                  |
| **Pin Header**          |     1     | 1x15 Male         | `PinHeader_1x15`     | Conexiune inter-plăci.                  |

#### Placa Inferioară (Bottom Board - Logică)

| Componentă             | Cantitate | Specificație      | Amprentă (Footprint) | Funcție                           |
| :--------------------- | :-------: | :---------------- | :------------------- | :-------------------------------- |
| **Socluri (Sockets)**  |     1     | 1x15 Female       | `PinSocket_1x15`     | Conexiune cu placa superioară.    |
| **Microcontroler**     |     1     | UM ESP32-S3 Pro   | `ProS3_TH`           | Unitatea centrală de procesare.   |
| **Antenă**             |     1     | 2.4 GHz Flexibilă | `U.FL`               | Comunicație Wireless.             |
| **LED-uri RGB**        |    10     | SK6812MINI        | `PLCC4_3.5x3.5mm`    | Iluminare ambientală (Underglow). |
| **Rezistori Pull-Up**  |    10     | 10k $\Omega$      | `0805_HandSolder`    | Stabilizare logică pini I/O.      |
| **Rezistori Limitare** |    10     | 470 $\Omega$      | `0805_HandSolder`    | Protecție curent LED-uri.         |
| **Condensator**        |     1     | 100nF             | `0805_HandSolder`    | Filtrare zgomot alimentare.       |

## 3. Ecosistemul Software

### 3.1. Configurator Web (Frontend)

Aceasta este o aplicație web statică (HTML/CSS/JS) găzduită local sau pe GitHub Pages.

- **Scop:** Permite utilizatorului să definească funcția fiecărei taste fără a reprograma firmware-ul.
- **Funcționalități:**
  - Editor vizual Drag-and-Drop.
  - Exportă configurația într-un fișier standardizat `config.json`.
  - Salvare automată în browser (LocalStorage).
- **Straturi (Layers):**
  1.  **FN Keys:** Taste funcționale standard (F13-F24).
  2.  **Shortcuts:** Scurtături de tastatură (ex: Ctrl+C).
  3.  **Commands:** Execuție de scripturi (ex: .bat, .py).
  4.  **Launcher:** Lansare de aplicații (ex: .exe).

### 3.2. Host Listener (Backend)

Un serviciu scris în Python care rulează în fundal pe calculatorul gazdă.

- **Rol:** Interceptează comenzile trimise de macropad pe canalul Serial USB pentru funcțiile avansate (Straturile 3 și 4).
- **Flux de date:**
  1.  Utilizatorul apasă o tastă pe macropad.
  2.  Firmware-ul trimite un cod unic (ex: `CMD_L3_K1`) prin Serial.
  3.  Listener-ul primește codul.
  4.  Listener-ul consultă fișierul `config.json` local pentru a vedea ce acțiune corespunde acelui cod.
  5.  Listener-ul execută acțiunea pe PC (ex: deschide Spotify).

### 3.3. Firmware (Embedded)

Software-ul care rulează pe ESP32-S3.

- **Mod HID (Human Interface Device):** Pentru Straturile 1 și 2, macropad-ul se comportă ca o tastatură standard (USB sau Bluetooth). Sistemul de operare recunoaște tastele nativ.
- **Mod Serial (CDC):** Pentru Straturile 3 și 4, macropad-ul trimite date seriale către Host Listener, ocolind limitările driver-ului standard de tastatură.

## 4. Design PCB și Galerie

Fișierele complete de design (Scheme electrice și Layout PCB) se găsesc în format PDF în acest folder:

- **Schematic Top:** [schematic-top.pdf.pdf](schematics/schematic-top.pdf)
- **Layout Top:** [layout-top.pdf](schematics/layout-top.pdf)
- **Schematic Bottom:** [schematic-bottom.pdf](schematics/schematic-bottom.pdf)
- **Layout Bottom:** [layout-bottom.pdf](schematics/layout-bottom.pdf)

### Galerie PCB

|                    Top Board                    |                   Bottom Board                   |
| :---------------------------------------------: | :----------------------------------------------: |
|    ![Top PCB View 1](images/render-top.png)     |    ![Bottom PCB 1](images/render-bottom.png)     |
| ![Top PCB View 2](images/render-top-angled.png) | ![Bottom PCB 2](images/render-bottom-angled.png) |

## 5. Ghid de Asamblare

1.  **PCB:** Comandați fabricarea plăcilor folosind fișierele Gerber din `hardware/pcb`.
2.  **Lipire:** Începeți cu componentele SMD de pe placa inferioară (rezistori, LED-uri). Continuați cu diodele THT și soclurile pe placa superioară.
3.  **Asamblare Mecanică:** Folosiți distanțierele M2.5 pentru a uni cele două plăci. Asigurați-vă că pinii de legătură (Pin Headers) sunt aliniați corect.
4.  **Componente Finale:** Introduceți switch-urile în socluri și montați capacele de taste (keycaps).
5.  **Software:** Conectați dispozitivul la PC, porniți Host Listener-ul și utilizați Configuratorul Web pentru prima setare.
