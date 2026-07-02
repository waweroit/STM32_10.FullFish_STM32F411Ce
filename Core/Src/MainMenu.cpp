/*
 * MainMenu.cpp
 *
 *  Created on: Jan 13, 2026
 *      Author: wawer
 */

#include "MainMenu.hpp"

static constexpr uint8_t LCD_COLS  = 16;
static constexpr uint8_t LCD_LINES = 2;

static void PrintLineFixed(uint8_t row, const char* text, bool selected)
{
    char line[LCD_COLS + 1];
    std::memset(line, ' ', LCD_COLS);
    line[LCD_COLS] = '\0';

    // marker wyboru
    line[0] = selected ? '>' : ' ';

    if (text)
    {
        // kopiujemy maks 15 znaków (bo 1 znak na marker)
        const size_t maxCopy = LCD_COLS - 1;
        const size_t len = std::min(std::strlen(text), maxCopy);
        std::memcpy(&line[1], text, len);
    }

    // Twoja biblioteka przyjmuje char[]
    HD44780_SetCursor(0, row);
    HD44780_PrintStr(line);
}

void Render(MenuState& st)
{
    if (!st.current)
    {
        PrintLineFixed(0, "Menu: NULL", false);
        PrintLineFixed(1, "", false);
        return;
    }

    const size_t count = st.current->children.size();
    if (count == 0)
    {
        PrintLineFixed(0, "(puste)", false);
        PrintLineFixed(1, "", false);
        return;
    }

    // zabezpieczenia indeksów
    if (st.selected >= count) st.selected = 0;
    if (st.top >= count) st.top = 0;

    // korekta top tak, by selected był widoczny w 2-liniowym oknie
    if (st.selected < st.top) st.top = st.selected;
    if (st.selected >= st.top + LCD_LINES) st.top = static_cast<uint8_t>(st.selected - (LCD_LINES - 1));

    const uint8_t idx0 = st.top;
    const uint8_t idx1 = (st.top + 1 < count) ? (st.top + 1) : 0xFF;

    // linia 0
    PrintLineFixed(
        0,
        st.current->children[idx0]->text.c_str(),
        (st.selected == idx0)
    );

    // linia 1
    if (idx1 != 0xFF)
    {
        PrintLineFixed(
            1,
            st.current->children[idx1]->text.c_str(),
            (st.selected == idx1)
        );
    }
    else
    {
        PrintLineFixed(1, "", false);
    }
}

void MoveDown(MenuState& st) {
    if (!st.current) return;
    auto n = st.current->children.size();
    if (n == 0) return;

    st.selected = (st.selected + 1) % n;
    if (st.selected >= st.top + 2) st.top = st.selected - 1;
}

void MoveUp(MenuState& st) {
    if (!st.current) return;
    auto n = st.current->children.size();
    if (n == 0) return;

    st.selected = (st.selected == 0) ? (n - 1) : (st.selected - 1);
    // dopasuj top do okna 2-liniowego
    if (st.selected < st.top) st.top = st.selected;
}

//void MoveDown(MenuState& st)
//{
//    if (!st.current) return;
//    const uint8_t n = static_cast<uint8_t>(st.current->children.size());
//    if (n == 0) return;
//
//    const uint8_t pageTop = st.top;
//    const uint8_t pageBottom = (pageTop + 1 < n) ? (pageTop + 1) : pageTop; // jeśli brak, to to samo
//
//    // jeśli jesteśmy na górze strony -> zejdź na dół (klasycznie)
//    if (st.selected == pageTop && pageBottom != pageTop)
//    {
//        st.selected = pageBottom;
//        return;
//    }
//
//    // jeśli jesteśmy na dole strony -> przejdź na następną stronę (o 2)
//    uint8_t newTop = pageTop + 2;
//    if (newTop >= n) newTop = 0;            // wrap do początku
//
//    st.top = newTop;
//    st.selected = st.top;                   // po zmianie strony zaznacz górę
//}
//
//void MoveUp(MenuState& st)
//{
//    if (!st.current) return;
//    const uint8_t n = static_cast<uint8_t>(st.current->children.size());
//    if (n == 0) return;
//
//    const uint8_t pageTop = st.top;
//    const uint8_t pageBottom = (pageTop + 1 < n) ? (pageTop + 1) : pageTop;
//
//    // jeśli jesteśmy na dole strony -> wejdź na górę (klasycznie)
//    if (st.selected == pageBottom && pageBottom != pageTop)
//    {
//        st.selected = pageTop;
//        return;
//    }
//
//    // jeśli jesteśmy na górze strony -> przejdź na poprzednią stronę (o 2)
//    uint8_t newTop;
//    if (pageTop >= 2) newTop = pageTop - 2;
//    else
//    {
//        // wrap do ostatniej strony: ((n-1)/2)*2
//        newTop = static_cast<uint8_t>(((n - 1) / 2) * 2);
//    }
//
//    st.top = newTop;
//    st.selected = (st.top + 1 < n) ? (st.top + 1) : st.top; // po wejściu "od góry" często wygodniej zaznaczyć dół
//    // jeśli chcesz zawsze górę po page-up, zmień na: st.selected = st.top;
//}


void Enter(MenuState& st) {
    if (!st.current) return;
    auto* item = st.current->children[st.selected];

    if (item->hasChildren()) {
        st.current = item;
        st.selected = 0;
        st.top = 0;
    } else if (item->isAction()) {
        item->onEnter();
    }
}

void Back(MenuState& st) {
    if (st.current && st.current->parent) {
        st.current = st.current->parent;
        st.selected = 0;
        st.top = 0;
    }
}

void GoRoot(MenuState& st, MenuItem* root)
{
    st.current  = root;
    st.selected = 0;
    st.top      = 0;
}


void Link(MenuItem& parent, MenuItem& child)
{
    child.parent = &parent;
    parent.children.push_back(&child);
}
