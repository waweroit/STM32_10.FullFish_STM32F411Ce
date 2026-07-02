/*
 * MainMenu.hpp
 *
 *  Created on: Jan 13, 2026
 *      Author: wawer
 */

#ifndef INC_MAINMENU_HPP_
#define INC_MAINMENU_HPP_

#include <cstdint>
#include <functional>
#include <vector>
#include <string>
#include <cstring>     // memset, memcpy, strlen
#include <algorithm>   // std::min
#include "liquidcrystal_i2c.h"



struct MenuItem {
    std::string text;                  // co wyświetlić
    std::function<void()> onEnter;      // akcja (gdy klik/2x klik). Pusta => brak akcji
    std::vector<MenuItem*> children;    // podmenu
    MenuItem* parent = nullptr;

    bool isAction() const { return (bool)onEnter; }
    bool hasChildren() const { return !children.empty(); }
};


struct MenuState {
    MenuItem* current = nullptr;  // gdzie jestem (który "katalog"/ekran)
    uint8_t selected = 0;         // który element w current->children jest zaznaczony
    uint8_t top = 0;              // od którego elementu zaczyna się okno (dla 2 linii)
};


void Render(MenuState& st);
void MoveUp(MenuState& st);
void MoveDown(MenuState& st);
void Enter(MenuState& st);
void Back(MenuState& st);
void GoRoot(MenuState& st, MenuItem* root);

void Link(MenuItem& parent, MenuItem& child);

#endif /* INC_MAINMENU_HPP_ */
