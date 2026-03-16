# glennergy
Hämtar spotpris och optimerar elförbrukning.

## Installation

1. Klona ner projektet:
```bash
git clone https://github.com/keseboleliasteliacom/glennergy.git
cd glennergy
```
2. Skapa en ny grupp för "glennergy" gruppen finns när installations-skriptet skapat och tilldelar permissions
```
newgrp glennergy
```

3. Kör installationsskriptet:
```bash
./glennergy_install.sh
```

Installationen skapar följande struktur(TODO):

```
/usr/local/bin/                 # Executable binaries
/var/log/glennergy/             # Log files
/etc/Glennergy-Fastigheter.json # System configuration
/tmp                            # FIFO files
```

Visa loggar(TODO):

```bash
tail -f /var/log/glennergy/*.log
```

---

# Usage

Starta programmet genom att köra den installerade binären:

```bash
Glennergy-Main
```

OBS: Kör **inte** `./Glennergy-Main`. Kör den installerade binären `Glennergy-Main`.

### Arguments(TODO)

- `port` (optional): Server port number (default: `8080`)
- `log_level` (optional): Logging level (default: `1`)

Log levels:

```
0 - DEBUG    Detailed debug information
1 - INFO     General information messages
2 - WARNING  Warning messages
3 - ERROR    Error messages only
```

---

# Development Notes

### Vid kodändringar

Om kod ändras ska **installationsskriptet köras igen**.

```bash
./glennergy_install.sh
```

Kör **inte** `make` manuellt.

---

# Viktig information

### Avsluta programmet korrekt

När programmet ska avslutas:

```
Ctrl + C
```

Använd **inte**:

```
Ctrl + Z
```

Cronjobben tas endast bort när programmet avslutas med **Ctrl + C**.

---

# Troubleshooting

## HTTPRequesten som efterfrågar data med hjälp av ID via browser eller Postman väntar i evighet, dvs får aldrig något svar
Detta sker på att cronjobben inte försvinnner.

Om programmet inte avslutas korrekt kommer cronjobben ligga kvar.

Varje minut startas då nya processer som väntar på att en läsarmodul (`InputCache`) ska ansluta. Detta leder till att läsare/skrivare hamnar i **desynk**, vilket gör att HTTP-requests slutar fungera.

Lösning:

```bash
sudo pkill -9 glenn
```

---

## Fel permissions efter installation

Om man råkat köra:

- `make`
- `sudo make`
- `sudo ./glennergy_install.sh`

kan permissions eller mappar ha skapats fel.

Vanligaste problemet är:

```
/var/log/glennergy
```

Snabbaste lösningen:

```bash
cd /var/log
sudo rm -rf glennergy
```

Kör sedan installationsskriptet igen:

```bash
./glennergy_install.sh
```

---

## Script körs inte / crontab_inst.sh ger error

Detta kan bero på **line endings** (Windows vs Linux):

- Windows: `CRLF`
- Linux: `LF`

### Lösning i VS Code

1. Tryck `Ctrl + ,`
2. Sök efter **"end of line"**
3. Ändra:

```
Files: Eol → \n
```

### Alternativt ändra per fil

1. Öppna filen i VS Code
2. Klicka på `CRLF` nere till höger
3. Ändra till `LF`
4. Spara filen

### Alternativ via terminal

```bash
dos2unix glennergy_install.sh
```

(kan kräva installation av `dos2unix`)
