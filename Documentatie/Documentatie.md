# Proiect Macropad

## Prezentare generala

Acest proiect reprezinta o tastatura pe baza de ESP32 S3 cu 12 taste mecanice, un rotary encoder cu switch asemanator cu EC11, un ecran OLED SSD1306 si un switch de comutare intre moduri wired si bluetooth. Va fi folosita pentru productivitate.

## Componente principale

### Unexpected Maker ProS3[D]

- Toata baza proiectului.

### Rotary Encoder asemanator cu EC11

- Folosit pentru volum / navigare pe OLED.

### Ecran OLED SSD1306 128 x 32

- Afisaj cu diagonala de 0.91", folosit pentru informatii despre tastatura cum ar fi nivelul bateriei, layerul selectat, scurtatura apasata ultima data

### Switch comutare intre moduri

- Switch SPDT de tip ON-OFF, folosit pentru a intra in modul low-power (aproape oprit).

### Componente pasive

- Diode THT fast-switching 1n4148 pentru a preveni ghosting
- Rezistente pull-up de 10k SMD 0805
- Condensator de decuplare 100nF SMD 1206

## Comunicare

Metodele principale de comunicare vor fi USB HID si Bluetooth/BLE HID, comutarea intre ele fiind facuta cu encoderul si afisajul.
Cele 12 taste, impreuna cu encoderul, vor putea fi customizate de utilizator. In principal ar fi 3 "straturi":
1. Taste de fucnctii extinse (F13-F24) pentru scurtaturi in anumite programe
2. Teste de scurtaturi (CTRL+Z, ALT+TAB, etc...)
3. Taste ce ruleaza scripturi
