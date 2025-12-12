# Macropad PROS3: O Tastatură Mecanică Personalizată, Complet Programabilă

Macropad-ul PROS3 este o tastatură mecanică (macropad) versatilă, open-source, cu 12 taste, construită în jurul puternicului microcontroler ESP32-S3. Proiectat ca un proiect de licență, acesta dispune de un encoder rotativ, un ecran OLED, iluminare RGB (underglow) și un ecosistem software complet pentru o personalizare profundă.

---

## Galerie

O colecție de randări și videoclipuri care prezintă hardware-ul.

|               Vedere de Sus               |                Vedere de Jos                 |
| :---------------------------------------: | :------------------------------------------: |
| ![Randare PCB Sus](images/render-top.png) | ![Randare PCB Jos](images/render-bottom.png) |

---

## Despre Proiect

Acest depozit (repository) conține fișierele complete de inginerie (hardware, firmware și software) pentru un macropad programabil, conceput pentru a eficientiza fluxurile de lucru digitale.

Proiectul este construit pe trei piloni:

1.  **Hardware:** Un design PCB personalizat, format din două plăci, cu o carcasă ce poate fi printată 3D. Este conceput pentru a fi asamblat de pasionați, folosind componente through-hole și socluri hot-swap.
2.  **Firmware:** Cod C++ personalizat care rulează pe ESP32-S3. Acesta gestionează input-ul, ecranul OLED, iluminarea și comunicarea cu computerul gazdă.
3.  **Suită Software:** O combinație puternică între un configurator web (pentru crearea de mapări de taste) și o aplicație gazdă bazată pe Python (pentru executarea de acțiuni complexe, cum ar fi rularea de scripturi sau lansarea de programe).

Această arhitectură permite macropad-ului să depășească simpla trimitere de taste și să acționeze ca o unealtă puternică de automatizare.

**Pentru o analiză tehnică completă, vă rugăm să consultați [Documentația Tehnică Completă (ENG)](Documentation.md).**
**(Pentru documentația în limba română, consultați [acest document](Documentatie.md)).**

---

## Funcționalități

- **12 Taste Mecanice Hot-Swap:** Personalizați-vă macropad-ul cu orice switch-uri stil MX, fără a necesita lipire.
- **Encoder Rotativ cu Buton:** Perfect pentru controlul volumului, navigarea prin cronologii video sau comutarea între straturi (layers).
- **Ecran OLED:** Obțineți feedback în timp real despre stratul curent, starea conexiunii și multe altele.
- **Sistem Puternic de Straturi (Layers):** Comutați între 4 straturi distincte pentru a multiplica numărul de taste disponibile.
- **Acțiuni în Mod Dual:**
  - **Mod HID:** Trimiteți apăsări de taste și scurtături standard care funcționează pe orice sistem de operare fără drivere.
  - **Mod Control Gazdă:** Declansați acțiuni complexe pe computerul dvs. — cum ar fi lansarea de aplicații sau rularea de scripturi — folosind aplicația de ascultare (listener) furnizată.
- **Configurator Bazat pe Web:** O interfață de utilizator prietenoasă, bazată pe browser, pentru remaparea tastelor, crearea de macrouri și configurarea iluminării. Nu este necesară programarea.
- **Iluminare RGB (Underglow):** 10 LED-uri RGB adresabile pentru efecte de iluminare strălucitoare, gestionate de biblioteca FastLED.
- **Conectivitate Duală:** Comutați fără probleme între o conexiune prin cablu USB-C și cea wireless Bluetooth/BLE HID.

---

## Ghid de Inițiere

Sunteți gata să vă construiți propriul macropad? Iată o prezentare generală a procesului:

1.  **Construiți Hardware-ul:** Folosiți fișierele KiCad din directorul [`hardware/`](../hardware/) pentru a fabrica PCB-urile și pentru a procura componentele din Lista de Materiale (BOM).
2.  **Instalați Firmware-ul:** Folosiți PlatformIO pentru a compila și încărca firmware-ul din directorul [`firmware/`](../firmware/) pe ESP32-S3.
3.  **Configurați Layout-ul:** Deschideți fișierul `web-configurator/index.html` într-un browser pentru a crea mapările de taste personalizate și pentru a exporta fișierul `config.json`.
4.  **Rulați Aplicația Gazdă:** Instalați dependențele Python și rulați scriptul `listener.py` din directorul [`host-listener/`](../host-listener/) pentru a debloca toate capabilitățile straturilor de comandă și lansator.

Pentru un ghid detaliat, pas cu pas, vă rugăm să consultați secțiunea **[Flux de Utilizare Complet din documentație](Documentatie.md#5-flux-de-utilizare-complet)**.

---

## Mulțumiri

Acest proiect a fost posibil datorită următoarelor proiecte open-source și comunități:

- **[ScottoKeebs](https://github.com/joe-scotto/scottokeebs)**: Pentru bibliotecile KiCad și amprentele pentru tastaturi mecanice.
- **Comunitatea de Tastaturi Mecanice**: Pentru că a oferit o sursă constantă de inspirație și cunoștințe.

## Licență

Hardware-ul și software-ul sunt furnizate "ca atare" (as-is) sub o licență permisivă. Acest proiect include componente din biblioteci terțe care sunt supuse propriilor licențe. Vă rugăm să consultați fișierele sursă individuale pentru mai multe detalii.
