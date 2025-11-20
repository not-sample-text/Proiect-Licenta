# Macropad cu ESP32-S3 - Proiect de Licență

Un macropad mecanic personalizat cu 12 taste, bazat pe microcontrolerul ESP32-S3. Dispozitivul include un encoder rotativ, ecran OLED și conectivitate duală (USB/Bluetooth).

## Galerie

|                  Vedere de Sus                  |                     Vedere de Jos                     |
| :---------------------------------------------: | :---------------------------------------------------: |
|        ![PCB Top](images/render-top.png)        |        ![PCB Bottom](images/render-bottom.png)        |
| ![PCB Top Angled](images/render-top-angled.png) | ![PCB Bottom Angled](images/render-bottom-angled.png) |

---

## Despre Proiect

Acest depozit conține întreaga suită de inginerie (Hardware, Firmware și Software) pentru un macropad programabil, realizat ca proiect de licență. Dispozitivul este conceput pentru productivitate, permițând utilizatorilor să creeze scurtături personalizate, macro-uri și straturi (layers) de funcții pentru a eficientiza fluxul de lucru.

**Pentru documentația tehnică detaliată, vă rugăm să consultați [Fișierul de Documentație](Documentatie.md).**

## Structura Proiectului

Proiectul este organizat în trei domenii principale de inginerie:

- **`hardware/`**: Conține fișierele de proiectare PCB în KiCad (`pcb/`) și modelele 3D pentru carcasă (`case/`).
- **`firmware/`**: Codul sursă C++/Arduino care rulează pe microcontrolerul ESP32-S3.
- **`web-configurator/`**: O interfață grafică bazată pe browser pentru configurarea tastelor și a straturilor.

## Funcționalități

- **12 Taste Mecanice Hot-Swap:** Posibilitatea de a personaliza experiența de tastare folosind orice switch-uri compatibile MX.
- **Encoder Rotativ:** Include un buton integrat pentru navigarea intuitivă în meniuri, controlul volumului și alte funcții.
- **Ecran OLED:** Un afișaj SSD1306 de 0.91" oferă feedback în timp real despre stratul activ, starea conexiunii și nivelul bateriei.
- **Configurare Web:** Remaparea tastelor și crearea de macro-uri printr-o interfață vizuală drag-and-drop, fără a fi necesară scrierea de cod.
- **Conectivitate Duală:** Comutare fluidă între conexiunea prin cablu USB-C și cea wireless Bluetooth/BLE HID.

## Ecosistemul Software

Acest proiect utilizează o arhitectură compusă din 2 părți pentru a gestiona configurarea:

### 1. Configurator Web

Situat în [`web-configurator/`](web-configurator/), aceasta este o aplicație HTML/JS independentă.

- Vizualizează arhitectura cu 4 straturi (Taste F, Scurtături, Comenzi, Lansator).
- Generează un fișier `config.json` care conține configurația personalizată.
- Suportă salvarea automată în memoria locală și importul configurațiilor anterioare.

### 2. Firmware

Situat în [`firmware/`](firmware/), scris în C++ pentru ESP32-S3.

- Gestionează scanarea matricei fizice de taste și randarea pe ecranul OLED.
- Funcționează ca o tastatură HID standard (prin USB sau Bluetooth) pentru a trimite comenzi către sistemul de operare, pe baza configurației încărcate.

## Prezentare Generală Hardware

- **Microcontroler:** Unexpected Maker ProS3 (ESP32-S3)
- **Design:** Un design cu două plăci PCB care separă componentele de interfață (taste, encoder) de placa principală de control.
- **Componente:**
  - 12x Socluri Hot-Swap compatibile MX
  - Encoder rotativ compatibil EC11
  - Ecran OLED I2C SSD1306 128x32
  - Diode 1N4148 pentru anti-ghosting

Schemele complete și layout-urile PCB se află în folderul [`hardware/pcb`](hardware/pcb).

## Ghid de Utilizare

1.  **Fabricare PCB:** Utilizați fișierele KiCad din `hardware/pcb` pentru a genera fișierele Gerber și a comanda plăcile.
2.  **Asamblare Hardware:** Lipiți componentele conform layout-ului PCB. Un ghid detaliat de asamblare este disponibil în [documentație](Documentatie.md).
3.  **Scriere Firmware:** Încărcați codul din folderul `firmware/` pe placa ProS3.
4.  **Configurare:** Deschideți Configuratorul Web (găzduit prin GitHub Pages sau local), proiectați straturile dorite și exportați fișierul JSON.

## Mulțumiri

- **[Unexpected Maker](https://unexpectedmaker.com/)** pentru placa de dezvoltare ProS3.
- **[ScottoKeebs](https://github.com/joe-scotto/scottokeebs)** pentru bibliotecile KiCad și amprentele pentru tastaturi mecanice.
- Comunității de tastaturi mecanice pentru inspirație.

## Licență

Fișierele de design hardware sunt oferite "ca atare" (as-is). Acest proiect include componente din biblioteci terțe care sunt supuse propriilor licențe.
