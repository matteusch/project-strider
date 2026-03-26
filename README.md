# **Strider – autonomiczny łazik z manipulatorem**

---

### **Zespół projektowy**

| Imię i nazwisko        | Nr indeksu |
|------------------------|-----------|
| Mateusz Górczyński     | 278355    |
| Maciej Otręba          | 278297    |
| Mateusz Kargol         | 278290    |
| Jakub Adamowicz        | 278299    |
| Piotr Balon            | 278314    |

---

**Prowadzący:** dr inż. Witold Paluszyński  

**Wrocław, 26.03.2026**

---

# Opis projektu

Projekt zakłada stworzenie autonomicznego łazika wyposażonego w manipulator. Pojazd o napędzie kołowym będzie zdolny do samodzielnej nawigacji po zadanym terenie oraz wchodzenia w interakcje z otoczeniem. Ścieżka ruchu i decyzje o wykonywanych akcjach będą wyznaczane na bieżąco na podstawie danych z systemu wizyjnego (kamera) oraz czujników.

W docelowej konfiguracji łazik przeszuka otoczenie, zlokalizuje obiekty docelowe (np. czerwone sześciany), podjedzie do nich, a następnie – za pomocą manipulatora – umieści je w pojemniku transportowym, aż do zebrania wszystkich elementów.

---

# Założenia projektowe

## Obowiązkowe

### Platforma łazika
- Stworzenie konstrukcji nośnej łazika z profili aluminiowych  
- Sterowanie silnikami  
- Zamontowanie czujników odległości na platformie  
- Zaprogramowanie algorytmu wykrywania kolizji  

### Manipulator
- Stworzenie konstrukcji manipulatora  
- Wyznaczenie kinematyki odwrotnej dla manipulatora  
- Zaprogramowanie mikrokontrolera do sterowania serwami  

### Układ wizyjny
- Wykrywanie obiektów o zadanym kolorze  
- Przetwarzanie danych z kamery na sterowanie platformy  

---

## Opcjonalne

- Implementacja orientacji łazika w przestrzeni (mapowanie otoczenia)  
- Autonomiczne odwożenie zebranych przedmiotów w jedno miejsce  
- Dodanie większej liczby sensorów  

---

# Narzędzia wykorzystywane do realizacji projektu

## Oprogramowanie
- Fusion 360 – modelowanie 3D  
- STM32CubeIDE – programowanie mikrokontrolerów STM32  
- STM32CubeMX  
- VS Code + PlatformIO (ESP32)  

## Narzędzia fizyczne
- Drukarka 3D – prototypowanie  
- Lutownica  

---

# Metodologia projektowa

Jako metodologię zarządzania projektem wybrano połączenie Scrum i Kanban (Scrumban), które zapewnia zarówno strukturę pracy, jak i elastyczność. Ze Scruma wykorzystano iteracyjne planowanie oraz regularne przeglądy postępów, natomiast z Kanbana – wizualizację zadań oraz zarządzanie przepływem pracy (WIP).

---

## Narzędzia wspomagające pracę zespołową

- Notion – dokumentacja projektu  
- Trello – tablica Kanban:
  - Backlog  
  - To Do  
  - In Progress  
  - Done  
- Git – kontrola wersji  
- Discord – komunikacja  

---

# Podział ról w projekcie

Projekt podzielono na trzy sekcje:

- Główna platforma i napęd  
  *(Mateusz Kargol, Maciej Otręba)*  

- Manipulator  
  *(Mateusz Górczyński, Jakub Adamowicz)*  

- System wizyjny  
  *(Piotr Balon)*  

Na początku prace będą prowadzone równolegle, a następnie nastąpi integracja systemów.

---

# Ryzyka projektowe

Możliwe problemy:

- Integracja systemu wizyjnego ze sterowaniem  
- Niedokładna detekcja przy zmiennym oświetleniu  
- Problemy mechaniczne manipulatora  
- Ograniczenia czasowe  

W celu minimalizacji ryzyka planowane jest testowanie modułów przed integracją.

