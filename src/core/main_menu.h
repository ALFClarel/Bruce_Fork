#ifndef __MAIN_MENU_H__
#define __MAIN_MENU_H__

#include <MenuItemInterface.h>

#include "menu_items/BleMenu.h"
#include "menu_items/ClockMenu.h"
#include "menu_items/ConfigMenu.h"
#include "menu_items/ConnectMenu.h"
#if !defined(BRUCE_NO_ETHERNET)
#include "menu_items/EthernetMenu.h"
#endif
#if !defined(BRUCE_NO_FM)
#include "menu_items/FMMenu.h"
#endif
#include "menu_items/FileMenu.h"
#if !defined(BRUCE_NO_GPS)
#include "menu_items/GpsMenu.h"
#endif
#include "menu_items/IRMenu.h"
#if !defined(BRUCE_NO_LORA)
#include "menu_items/LoRaMenu.h"
#endif
#if !defined(BRUCE_NO_NRF24)
#include "menu_items/NRF24.h"
#endif
#include "menu_items/OthersMenu.h"
#include "menu_items/RFIDMenu.h"
#include "menu_items/RFMenu.h"
#include "menu_items/ScriptsMenu.h"
#include "menu_items/WifiMenu.h"
class MainMenu {
public:
    FileMenu fileMenu;
    BleMenu bleMenu;
    ClockMenu clockMenu;
    ConnectMenu connectMenu;
    ConfigMenu configMenu;
#if !defined(BRUCE_NO_FM)
    FMMenu fmMenu;
#endif
#if !defined(BRUCE_NO_GPS)
    GpsMenu gpsMenu;
#endif
    IRMenu irMenu;
#if !defined(BRUCE_NO_NRF24)
    NRF24Menu nrf24Menu;
#endif
    OthersMenu othersMenu;
    RFIDMenu rfidMenu;
    RFMenu rfMenu;
    ScriptsMenu scriptsMenu;
    WifiMenu wifiMenu;
#if !defined(LITE_VERSION)
#if !defined(BRUCE_NO_LORA)
    LoRaMenu loraMenu;
#endif
#if !defined(BRUCE_NO_ETHERNET)
    EthernetMenu ethernetMenu;
#endif
#endif

    MainMenu();
    ~MainMenu();

    void begin(void);
    std::vector<MenuItemInterface *> getItems(void) { return _menuItems; }
    void hideAppsMenu();

private:
    int _currentIndex = 0;
    int _totalItems = 0;
    std::vector<MenuItemInterface *> _menuItems;
};
extern MainMenu mainMenu;

#endif
