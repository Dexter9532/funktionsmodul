# funktionsmodul

This is a project i did in school using many diffrent technics on a ATmega 328p, here is my repport for the project

Funktionsmodul 360


















Student: Bobo Bäck Engström
Datum: 
Examinator: Martin Dean
Sammanfattning
I detta projekt har jag konstruerat en funktionsmodul bestående av en Arduino Uno, en MAX7221-displaymodul, ett 1x4 knappmembran och en breadboard-shield. Modulen har två huvudsakliga funktioner: en true randomizer som genererar slumpmässiga tal genom ADC-omvandling och bitvis manipulation samt en timer som räknar upp i realtid.
Den slumpmässiga talgeneratorn använder sig av brus från en konstant 5V-ingång och har nio olika inställningar för talintervall, från 1–2 upp till 1–9999. En timerfunktion implementeras också, där tiden visas på en display och kan startas, pausas och återställas, med en maxgräns på 99 minuter och 59 sekunder.
För att implementera dessa funktioner används flera tekniker:
ADC-omvandling för att generera slumpmässiga tal.
Pin Change Interrupts för att hantera knapptryckningar i realtid.
SPI-kommunikation med MAX7221 för att styra 7-segmentsdisplayer.
Timers för att hålla reda på tid och uppdatera displayen.
UART för att debugga och skicka data via seriell kommunikation.
EEPROM för att spara inställningar mellan strömavbrott.
Hårdvaran består av färdiga moduler sammanlänkade med kablar och tejp. På grund av tidsbrist utvecklades inget eget PCB. Projektet har testats genom att köra timern i 99 minuter och analysera slumpgeneratorns prestanda. En utmaning var att hantera brus i MAX-modulen, vilket löstes genom att förbättra kablaget.
Slutligen var projektet både lärorikt och utmanande. Trots en misslyckad PCB-beställning i ett tidigare projekt kunde jag snabbt utveckla en fungerande lösning. Jag har fått djupare förståelse för registerhantering, bit manipulation och samverkan mellan flera tekniker i system.





Innehållsförteckning

Sammanfattning	2
Innehållsförteckning	3
1. Introduktion	4
2  Enkel manual	5
Spelregler:	5
Punish wheel:	5
Funktioner:	5
3 Konstruktion	5
3.1  MAX7219	6
3.2 Atmega328p	7
3.3 Knappar	8
3.4 Strömförsörjning & Programmering	9
3.5 Mjukvara	9
4 Miljö	10
5 Design	10
5.1 Schema	10
5.2 Pcb	11
5.3 Fysisk Produkt	12
6 Verifiering	14
7 Slutsatser och diskussion	14
7.1 Resultat	14
8 Referenser	15
9 Appendix	16
Källkod	16
Sammanfattning	29
Innehållsförteckning	30
2. Introduktion	32
2  Enkel manual	33
3 Konstruktion	34
3.1  MAX7221´/MAX7219	34
3.3 Knappar	35
3.5 Mjukvara	35
5 Design	35
5.3 Fysisk Produkt	36
6 Verifiering	37
7 Slutsatser och diskussion	37
7.1 Resultat	37
8 Referenser	38
9 Appendix	39
Källkod	39


Introduktion
I detta projekt så har jag byggt en funktionsmodul som kan göra 2 större funktioner. Jag har byggt en “breadboard” hårdvara som innehåller:
MAX 7221-Färdig modul
1x4 knappmembran
Breadboard shield
Arduino Uno
Funktionsmodellen har en “true” randomizer som använder brus för att generera ett slumpmässigt tal. Talen som slumpas har 9 olika inställningar och de är:
1-2
1-5
1-10
1-50
1-100
1-500
1-1000
1-5000
1-9999
För att slumpa fram talet så använder jag mig av ADC-omvandling då jag mäter en konstant spänning på 5V från Arduinon och använder bruset för att generera och mappa slumpmässiga tal. För att öka slumpmässigheten på grund av att brus inte alltid uppstår så har jag även en counter som räknar hela tiden i program som sedan kör en bitvis manipulation Xor tillsammans med ADC omvandlingen.
Funktionsmodellen har även en timer som räknar upp i realtid och visas på displayen. Denna funktion går att starta och stoppa samt att reseta. Max uppräkning är 99 minuter och 59 sekunder.
Tekniker jag använder för min funktionsmodul
ADC-Omvandlare - Används för att få fram “true random” då jag läser av en 5V ingång hela tiden som jag sedan tar decimalerna av.
Pin Change Interrupts- Används för att spelet ska läsa av knappar som trycks och att de ska göras direkt.
Spi (MAX7221/MAX7219)- Används för att kunna köra mina displayer där jag visar information om uppräkning eller slumpmässiga tal
Timmer-Används för att räkna upp tiden i realtid och visa på displayen.
Uart-Används för att debugga koden och kunna skriva ut på seriel-port.
EEprom - Används för att spara setting inställningen som va vald innan ström bröts.





2 	Enkel manual

Knapp 1 - Startar timer från början
Knapp 2 - pausar och startar timer
Knapp 3 - Slumpar fram tall enligt settings
Knapp 4 - Ändrar settings
setting 0 — 1-2
Setting 1 — 1-5
Setting 2—1-10
Setting 3—1-25
Setting 4—1-50
Setting 5—1-100
Setting 6—1-500
Setting 7—1-1000
Setting 8—1-5000
Setting 9—1-9999
Funktionen som används visas på display 2 från vänster  (F(nummer).
Setting som används visas på display 4 från vänster (S(nummer).


3	Konstruktion

Jag använder mig av en “breadboard” hårdvara med färdiga moduler som jag lappar ihop med hjälp av tejp och kablar.



3.1 	MAX7221´/MAX7219 

Hur en MAX 7221/19 funkar. 

Bild 1.0 MAX7219
På min MAX7221 så sitter det 8 7segment displayer som MAX driver med hjälp av SPI-komunikation. Det enda som behöver för att styra allt detta är 5 pinnar som går till arduinon.







3.3	Knappar

Jag använder mig av 1x4 knappar i membran från electrokit för att den modul ska fungera som jag vill.
https://www.electrokit.com/tangentbord-membran-1x4

3.5	Mjukvara

Mjukvaran är en kod skriven i C bare metal och använder inga färdiga komponent-bibliotek eller annat liknande.
koden har varit utmanande och skriva men fortfarande väldigt rolig. Som hjälpmedel har jag använt boken [1] och även lite AI för att få förståelse för alla register.
Min kod är uppbyggd i min while loop med 4 olika “main” if-satser som läser av vilken funktion som används. När en funktion har utförs så sparas den och du ser på skärmen vilken funktion som du använder. För att när som helst kunna byta funktion så kan du klicka på valfri knapp och en interrupt triggas (pin change interrupts) på hela PORTD. Jag läser sedan vad PIND är och ger dem en variabel. En switch-sats sätter funktioner till olika värden beroende på av vilken av knapparna som klickades in.
Kommentarsblock och förtydligande finns i koden, se appendix.

5	Design

På grund av tidsbrist har jag tyvärr inte skapat något eget PCB i detta projekt utan jag har byggt allt med lite tejp och klister. Detta för att ändå få en “hållbar” enhet som går att använda.



5.3	Fysisk Produkt







6	Verifiering
För att verifiera att detta funkar så har kört många tester inklusive att ha på timer i 99 minuter och 99 sekunder. Jag har även testat hur random min random generator är med hjälp av att logga ett flertal knapptryckningar och använt chat gpt för att få fram hur slumpmässigt min generator är.
Jag har även kört min timer bredvid min mobil och tiden är exakt samma.
Har haft mycket problem i början med MAX modulen men insåg att den är extremt känslig för brus. Bruset uppstår av dåliga hona till hane kablar. När en bit i kommunikationen försvinner så slutar den funka.

7	Slutsatser och diskussion
Jag är otroligt nöjd med detta projekt då jag hade extremt lite tid på grund av att jag tyvärr misslyckades med mitt första projekt då jag beställde PCB som ej fungerade. Men jag kom på en rolig idé och lyckades slänga ihop något som jag kan stå för. 
Jag hade roligt när jag skrev min kod och jag kom på nya lösningar på problem jag inte hade tänkt på förut. Det var samtidigt väldigt nyttigt att få jobba med alla olika tekniker på en och samma gång och få dem att samverka.
Jag tycker det var svårast att få till timers rätt och jag fick trixa lite innan jag fick fram det som jag hade det i huvudet. 
Jag har fått en stor förståelse för hur man skriver till register och hur man använder bitar för att välja olika “inställningar” i olika register.
Jag valde även att bojkotta Atmel-ice för att köra ren c med arduinon i Vs Code istället på grund av att jag vill undvika mängd kablar och enheter samt att jag kan printa i en seriell monitor vilket jag tycker är mycket bättre än att debugga.
7.1	Resultat

Resultatet blev mycket framgångsrikt med många roliga tekniker och funktioner. Jag hade självklart önskat att få göra detta på ett PCB men tyvärr så fanns inte tiden för det. Mitt projekt blev också väldigt lätthanterligt då jag valde att lappa ihop två stycken P0 breadboards för att allt skulle sitta ihop som en liten “enhet”.

8	Referenser

[1] B. N. Mazidi, S. N. Mazidi och R. D. McKinlay, The AVR Microcontroller and Embedded Systems: Using Assembly and C, 2nd ed. Pearson, 2017.
[2]M. Dean, Lektionsmaterial(timer & counters), Google Classroom, [Online]. Tillgänglig: https://classroom.google.com/c/NzMwNDE0MTQ4MzQx/m/MjEyNTA2NzEwNzNa/details.
[3].M. Dean, Lektionsmaterial om interrupts, Google Classroom, [Online]. Tillgänglig: https://classroom.google.com/c/NzMwNDE0MTQ4MzQx/m/MjEyNTkyNjQ3MTVa/details. 
[4]M. Dean, Lektionsmaterial om UART, Google Classroom, [Online]. Tillgänglig: https://classroom.google.com/c/NzMwNDE0MTQ4MzQx/m/MjMwODc5MzczOTZa/details.
[5]M. Dean, Lektionsmaterial om ADC, Google Classroom, [Online]. Tillgänglig: https://classroom.google.com/c/NzMwNDE0MTQ4MzQx/m/MjEzNTg2NTI0Njda/details.
[6]M. Dean, Lektionsmaterial om SPI, Google Classroom, [Online]. Tillgänglig: https://classroom.google.com/c/NzMwNDE0MTQ4MzQx/m/MjMxMjI2NzQ2MzZa/details.
