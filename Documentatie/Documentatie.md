# Proiect Macropad
---

## Prezentare generala

Acest proiect reprezinta o tastatura pe baza de ESP32 S3 cu 12 taste mecanice, un rotary encoder cu switch asemanator cu EC11, un ecran OLED SSD1306 si un switch de comutare intre moduri wired si bluetooth. Va fi folosita pentru productivitate.

## Componente principale

### Adafruit Feather V2

Toata baza proiectului.

### Rotary Encoder asemanator cu EC11

Folosit pentru volum / navigare pe OLED.

### Ecran OLED SSD1306 128 x 64

Afisaj cu diagonala de 1.3", folosit pentru informatii despre tastatura cum ar fi nivelul bateriei, layerul selectat, scurtatura apasata ultima data

### Switch comutare intre moduri

Switch SPDT de tip ON-OFF, folosit pentru comutarea intre comunicarea pe fir si comunicarea bluetooth.

### Componente pasive

Diode fast-switching 1n4148 pentru a preveni ghosting tht
Rezistente pull-up de 10k smd 0805
Condensator de decuplare 100nF smd 1206
