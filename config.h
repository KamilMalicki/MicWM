/*
 * Copyright [2026] [KamilMalicki]
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// config.h - główny plik konfiguracyjny MicWM
// Aby zastosować zmiany, zapisz plik i przebuduj menedżer (make && sudo make install)

// --- KLAWISZ MODYFIKATORA ---
// Mod1Mask = Alt
// Mod4Mask = Super (klawisz Windows)
#define MOD_KEY       Mod4Mask

// --- WYGLĄD I KOLORY ---
#define KOLOR_ZWYKLY  0x1f1f1f  // Kolor ramki nieaktywnego okna (HEX)
#define KOLOR_FOCUS   0xffffff  // Kolor ramki aktywnego okna (HEX)

// --- WYMIARY ---
#define GRUBOSC_RAMKI 4         // Grubość obramowania
#define MIN_ROZMIAR   10        // Minimalny rozmiar okna

// --- PASEK STATUSU ---
#define WYSOKOSC_PASKA 20       // Wysokość paska w pikselach
#define TEKST_NA_PASKU " MicWM " // Tekst startowy

// --- APLIKACJE I SKRYPTY ---
#define MOJ_TERMINAL "st"       // Domyślny terminal
#define AUTOSTART_SCRIPT "bash $HOME/.autoconfigscriptmicwm &" // Skrypt autostartu
