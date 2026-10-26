# Bruce_Fork — T-Embed CC1101 base

Dépôt : https://github.com/ALFClarel/Bruce_Fork

## Cible et compilation

Une seule cible par défaut : `lilygo-t-embed-cc1101`, ESP32-S3, 16 Mo flash,
8 Mo PSRAM, ST7789 170×320, CC1101, PN532 I2C, IR, encodeur, huit WS2812,
SD, BQ25896/BQ27220 et audio natif.

```sh
python -m pip install platformio==6.1.19
# Linux : gcc-multilib et g++-multilib sont nécessaires au générateur JS natif.
python tools/check_tembed.py
python -m platformio run -e lilygo-t-embed-cc1101
python tools/tembed_size.py .pio/build/lilygo-t-embed-cc1101
python tools/package_tembed.py
```

Profil complet : `boards/lilygo-t-embed-cc1101/lilygo-t-embed-cc1101.ini`.
Pinmap : `boards/lilygo-t-embed-cc1101/pins_arduino.h`.
Les options `-Os`, LTO, sections individuelles et `--gc-sections` sont conservées.
L'override `-O2` est supprimé. **Ne pas activer `LITE_VERSION`** : il supprimerait
SSH, WireGuard, l'interpréteur et d'autres fonctions demandées.

## Retraits

| Groupe | Sources / entrées retirées de cette compilation | Dépendances retirées |
|---|---|---|
| LoRa | `modules/lora`, `LoRaMenu`, choix et sérialisation associés | RadioLib |
| nRF24 / Mousejack | unités `modules/NRF24` vides sous garde, `NRF24Menu`, appels depuis BLE Suite, module ROM JS | RF24 |
| GPS | `modules/gps`, `GpsMenu`, applications de démarrage, envoi des CSV à Wigle/WDG | TinyGPSPlus |
| FM | `modules/fm`, `FMMenu`, définition SI4713 | Adafruit Si4713 |
| iButton | `others/ibutton.cpp`, entrée Others | OneWire |
| RFID LF | `rfid125.cpp`, entrée 125 kHz et pinmap | — |
| NFC externe | RFID2/RC522, ST25R3916, PN532 BLE/UART/Killer, Chameleon, client Amiibolink | MFRC522v2, ST25R3916/RFAL, ESP-ChameleonUltra, ESP-Amiibolink, ESP-PN532BLE/UART/Killer |
| Ethernet externe | `modules/ethernet`, menu et pinmap W5500 | bibliothèques internes rendues inaccessibles |
| Autres cartes | seuls le `.ini` et `interface.cpp` ciblés sont sélectionnés | aucun gain fictif annoncé pour des fichiers déjà exclus |
| Wi-Fi 5/6 GHz | tables, regroupements de bandes et libellés spécifiques | — |
| CI multicarte | cinq workflows amont remplacés par la validation T-Embed | matrice de 61 jobs supprimée |

La pinmap conserve **exactement** `MIC_SPM1423`, `PIN_CLK=39`, `PIN_DATA=42`,
`HAS_NS4168_SPKR`, `BCLK=46`, `WCLK=40`, `DOUT=7`, `MCLK=PIN_CLK`.
Les alias SPI inversés `SCK` / `MISO` sont corrigés. Les GPIO nRF24 43/44 ne sont
plus initialisés sur la base. La capacité batterie n'est plus réécrite à 1 300 mAh.

Les autres pinmaps restent dans l'historique/source amont pour la traçabilité ;
elles ne sont pas compilées. Aucune dépendance LVGL n'est présente dans cette
révision. TFT_eSPI utilise déjà la police GLCD par défaut ; aucune collection de
polices inutilisée n'est ajoutée. Les chaînes littérales ESP32 sont déjà placées
en mémoire flash ; les menus mutables `String`/`std::function` ne sont pas convertis
aveuglément en `PROGMEM`.

## Gardes ajoutées

`include/tembed_capabilities.h`, forcé pour toutes les unités, définit :

- `BRUCE_TEMBED_BASE` : profil choisi par le `.ini`.
- `BRUCE_NO_LORA`, `BRUCE_NO_NRF24`, `BRUCE_NO_GPS`, `BRUCE_NO_FM`.
- `BRUCE_NO_LF_RFID`, `BRUCE_NO_IBUTTON`, `BRUCE_NO_ETHERNET`.
- `BRUCE_PN532_ONLY`, `BRUCE_NO_EXTERNAL_NFC`, `BRUCE_NO_I2C_DISCOVERY`.
- `BRUCE_WIFI_24GHZ_ONLY`.

`BRUCE_TEMBED_PLUS` préserve les gardes et GPIO nRF24 pour un éventuel port Plus ;
**ce dépôt livre et teste uniquement la base**. Un port Plus doit aussi rétablir la
dépendance RF24 ; définir seulement ce drapeau ne constitue pas un firmware Plus validé.

## Patches A à F et accès

| Patch | Accès utilisateur | Implémentation partagée |
|---|---|---|
| A | RF → Scan/copy, Spectrum, RSSI Spectrum ; RF → Custom SubGhz | molette : pas de 10 kHz, saut des bandes impossibles, aucune écriture flash par cran ; écran de lecture existant : molette choisit un enregistrement, OK émet, Retour quitte |
| B | automatique pendant les scans existants | `signalMeter()` dans `led_control.cpp`, consommé par la tâche LED existante ; Wi-Fi/BLE/RF utilisent RSSI, NFC indique une lecture réussie |
| C | RF → RF Fox Hunt | même fonction et graphe que RSSI Spectrum ; même `playTone()` étendu au moteur audio asynchrone existant |
| D | automatique ; indicateur batterie existant | BQ27220 réel, 0 % affichable, lecture invalide = inconnue ; trois échantillons et hystérésis ; plafonds de luminosité 35/10 %, réduction des fenêtres BLE et cadence des balayages RF |
| E | RFID → NFC → RF / IR | même `TagOMatic`, une table `/BruceNFC/macros.tsv`, mêmes `readSubFile`/`txSubFile`/`txIrFile` ; aucun lecteur en arrière-plan |
| F | gestionnaire SD → File Info / Filename QR | fichiers originaux inchangés ; sidecar `.meta.json`, horloge non réglée explicitement signalée, QR du chemin produit avec l'encodeur existant |

Aucun FFT, enregistreur, pilote LED, émetteur RF/IR ou moteur NFC parallèle n'a été
créé. `OthersMenu::micMenu()` est comparé automatiquement au code amont et reste
inchangé. Les copies d'enregistrements RF utilisent la PSRAM avec repli DRAM ;
les buffers de réception RMT/DMA restent internes.

La fonction radio d'origine reste disponible par ses chemins existants, dont le
gestionnaire SD et les commandes/JS lorsqu'ils sont compatibles. Les nouveaux modes
ne dupliquent pas leurs moteurs. L'interface principale utilise la liste existante.
Les sous-menus complexes amont et le sélecteur micro restent présents : **la limite
absolue à deux niveaux n'est pas entièrement réalisée**, pour préserver les flux
natifs plutôt que recréer les mêmes fonctions.

## Mesures et validation

`tools/check_tembed.py` vérifie les limites de fréquence, le gouverneur, l'absence de
code émis par 33 unités désactivées, la conservation de micMenu et les dépendances.
Les tests hôtes passent. Ils ne remplacent pas une compilation ESP32 ou un test matériel.

La CI construit la cible et la référence `a59213f3ec6cd302fa32c2042b2700c9c24d991a`,
avec FastLED **3.10.3 dans les deux cas**. La contrainte amont `^3.10.3` résout
aujourd'hui vers 3.10.8, qui entre en collision avec la macro `FP` de Bruce.
La correction commune de dépendance doit être signalée dans toute comparaison.

Les artefacts `size-report.json` et `tembed-size-delta-*` donnent les valeurs réelles.
La CI échoue si le `.bin` ne diminue pas ou si la DRAM statique augmente.
Les durées de build sont enregistrées ; le cache et les téléchargements influencent
ces durées. Le temps de démarrage et la consommation doivent être mesurés sur la carte.

| Étape | Flash / RAM mesurées |
|---|---|
| Retrait matériel | pas encore de mesure validée au moment de rédaction |
| A | en attente de compilation comparable |
| B | en attente de compilation comparable ; service LED existant maintenu actif |
| C | en attente ; utilise la pile audio asynchrone native à la demande |
| D | en attente |
| E | en attente |
| F | en attente ; augmente légèrement les fichiers stockés sur SD |

Les ajouts fonctionnels peuvent augmenter une étape intermédiaire. Le critère
vérifié est le résultat final par rapport à la référence. Aucun gain estimé en
Ko, millisecondes ou pourcentage n'est présenté comme un résultat mesuré.

## Flash et futur VPS

Le build amont fournit `Bruce-lilygo-t-embed-cc1101.bin`, image fusionnée pour
ESP32-S3 à l'adresse **0x0**. `tools/package_tembed.py` prépare :

- `firmware-merged.bin` : installation série / ESP Web Tools à 0x0.
- `firmware-ota.bin` : application seule pour l'OTA WebUI existante.
- `manifest.json`, `SHA256SUMS`, licence et URL des sources du commit exact.

```sh
python -m esptool --chip esp32s3 --port PORT write-flash 0x0 dist/tembed/firmware-merged.bin
```

L'installation série initiale et une mise à jour OTA sont deux opérations distinctes.
Ne pas envoyer l'image fusionnée dans l'OTA. Le partitionnement 16 Mo existant reste
inchangé. L'hébergement HTTPS du manifeste et des binaires sur ton VPS sera décidé
plus tard ; aucun VPS n'est configuré ou modifié ici.

## Risques et essais matériels encore nécessaires

- Mic : spectre, WAV sur SD, relecture, sortie propre, puis reprise RF/NFC.
- RF : changement de bande SW0/SW1, précision du pas, sortie/reprise, RAW et décodé.
  Un segment RAW n'est pas nécessairement une trame ; aucun découpage inventé.
- Fox Hunt : RSSI relatif, orientation/atténuation de l'antenne et audibilité des bips.
  Ce mode ne mesure pas une distance.
- LED : arbitrage effets/indicateur et coexistence RMT avec RF et IR.
- PN532 : lecture, NDEF, clonage supporté, émulation supportée ; pas de RSSI
  analogique ni d'émulation NTAG215/Amiibo complète inventée. L'ancien client
  Amiibolink nécessitait un périphérique externe et a été retiré.
- Pont NFC : UID clonable, donc pas un contrôle d'accès ; retirer le tag avant
  de relancer la même macro. Table limitée à 8 Kio, 64 lignes, 256 caractères/ligne,
  capture ciblée limitée à 64 Kio.
- Batterie : jauge dépendante de son calibrage et du pack réel ; aucune garantie
  contre une chute de tension brutale. Le gouverneur ne coupe pas une capture,
  une session WebUI/OTA ou le Wi-Fi actif pour simuler une économie.
- BadUSB, BLE clavier/spam, réseau, SD, JS et OTA doivent subir un essai matériel.
- Les dépendances Git amont restantes suivent parfois des branches mobiles ;
  un verrouillage complet de tous les SHA reste une amélioration de reproductibilité.
