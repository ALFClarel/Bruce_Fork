# T-Embed CC1101 base — audit before implementation

Upstream: BruceDevices/firmware, a59213f3ec6cd302fa32c2042b2700c9c24d991a.
License: AGPL-3.0; retain LICENSE and THIRD_PARTY.md.

## Existing implementations and decisions

| Request | Existing overlap | Decision |
|---|---|---|
| Hardware trim | platformio.ini; boards/lilygo-t-embed-cc1101/{lilygo-t-embed-cc1101.ini,pins_arduino.h,interface.cpp}; boards/_boards_json/lilygo-t-embed-cc1101.json | Edit target profile/pinmap; keep native mic and speaker pins/types. Other board files are already outside the selected source filter; no invented binary saving from deleting them. |
| Menus | src/core/main_menu.{h,cpp}; menu_items/{RFIDMenu,RFMenu,OthersMenu,ConfigMenu}.cpp; core/startup_app.cpp; core/sd_functions.cpp | Compile out unsupported registrations and their sources, including alternate CLI/JS/startup routes. |
| LoRa / nRF / FM / GPS / LF / iButton / Ethernet | src/modules/{lora,NRF24,fm,gps,ethernet}; others/ibutton.cpp; rfid/rfid125.cpp | Exclude sources and dependencies; guard references. Base has no nRF24. Plus capability must be explicit. |
| NFC drivers | rfid/{PN532,RFID2,ST25R3916,tag_o_matic}.cpp; serial_commands/rfid_commands.cpp | Use existing PN532 only; remove alternate driver factories and config choices. |
| NFC peripherals | rfid/{amiibo,chameleon,pn532ble,PN532KillerTools}.cpp | Amiibo here means an external BLE Amiibolink client, NOT native PN532 NTAG215 emulation. Do not label it native Amiibo support. Peripheral-only menus are incompatible with onboard-only scope. |
| Native audio | others/{mic,audio,audio_player}.cpp; OthersMenu::micMenu; playTone/_tone; MIC_SPM1423 and HAS_NS4168_SPKR | Preserve micMenu and native capture/playback implementations. No new FFT, recorder, I2S driver or beep generator. |
| A tuning / frames | RFScan::{loop,fast_scan}; rf_utils::{setMHZ,setMHZMenu}; rf_spectrum::{rf_spectrum,rf_CC1101_rssi}; rf_send::{readSubFile,loopEmulate,txSubFile}; drainRotarySteps | Edit existing screens. Reuse lists of decoded keys and RAW lines; frame means a stored record/RAW line, not an inferred RF packet boundary. |
| B meter | core/led_control::{beginLed,ledEffectTask,ledEffects}; BLE AdvertisedDeviceCallbacks; WiFi scan; RFScan; TagOMatic::{read_card,scan_cards} | Add shared meter to existing LED driver, not another task/driver. PN532 has no exposed RSSI: presence/success only. |
| C homing | rf_CC1101_rssi, ELECHOUSE_cc1101.getRssi, playTone, existing LED driver | Extend RSSI screen with homing toggle, no parallel scanner or audio implementation. Relative strength is not calibrated range. |
| D power | board interface::getBattery/isCharging/_setBrightness; core/powerSave::checkPowerSaveTime; display::{drawStatusBar,drawBatteryStatus}; BQ27220::{getChargePcnt,getVolt} | Extend existing indicator/governor. Never reset charger/gauge calibration to assume the installed battery capacity. No claim to prevent arbitrary brownouts. |
| E NFC macro | TagOMatic::read_card; rf_send::{readSubFile,txSubFile}; ir/custom_ir::sendIRCommand | Add bounded SD mapping dispatcher, reuse actual send implementations; no background reader. UID mappings are convenience, not authentication. |
| F metadata | rf_scan::rfSaveSignal; rf/save::rf_raw_save; IrRead::write_file; PN532::save; sd_functions file menu; qrcode_menu | Add shared sidecar writer/reader; preserve Flipper interchange. Timestamp must distinguish unset RTC. QR rendered on demand in existing SD manager, not duplicated in every capture screen. |
| JS registry | bjs_interpreter/{mqjs_stdlib.c,nrf24_js.cpp}; gen_mqjs_headers.py | Strip nRF symbols from generated ROM, not just C++ wrapper. Generator signature must include capabilities. |
| Optimisation | existing -Os/-flto/section GC; target overrides -O2; flto_prep.py; radio_mem.h | Remove -O2 override; preserve radio DMA internal RAM. PSRAM only for non-DMA allocations. No blanket PROGMEM conversion of mutable String/std::function menus. |
| UI fonts | in-tree display HAL/TFT_eSPI; no LVGL found | Keep font 1/2 required by existing calls; inspect runtime-loaded themes/fonts before stripping. No fictitious LVGL widget savings. |
| Flash/OTA | build.py, custom_16Mb.csv, webInterface.cpp | Keep partition layout/WebUI and existing merged binary builder; package manifest separately for future HTTPS VPS host. |

## Baseline status

`python -m platformio run -e lilygo-t-embed-cc1101` attempted before edits.
Platform download timed out at release-assets.githubusercontent.com before C++ compilation.
No measured flash/RAM/boot/build-time baseline exists yet. Never report estimates as measured deltas.

## Validation gates

Require a clean target build, linked-symbol check for excluded drivers, matching-baseline
ELF/bin comparison, then hardware smoke tests (SD, mic record/spectrum, playback, RF,
PN532, encoder, BLE, USB, WebUI/OTA). Neither native Amiibo emulation nor NFC RSSI can
be fabricated by software. A strict two-level UI conflicts with preserving micMenu
as-is; retain its existing Spectrum/Record selector instead of duplicating it.

## Correction issue du lien ESP32

Le dossier `modules/ethernet` contient aussi les implémentations esp-netif utilisées par le Wi-Fi : `ARPScanner`, `ARPSpoofer`, `ARPoisoner` et `HostInfo`. Elles sont conservées en place. Seules les branches et pilotes ETH/W5500 sont exclus ; aucun scanner ARP parallèle n’est ajouté.
