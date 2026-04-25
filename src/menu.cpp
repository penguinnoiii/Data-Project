#include <iostream>
#include "menu.h"

void showMenu() {
    int choice;
    do {
        std::cout << "\n=== Project Name ===\n";
        std::cout << "1. Add\n";
        std::cout << "2. Delete\n";
        std::cout << "3. Search\n";
        std::cout << "4. Display\n";
        std::cout << "0. Exit\n";
        std::cout << "Choice: ";
        std::cin >> choice;

        switch (choice) {
            case 1: /* TODO */ break;
            case 2: /* TODO */ break;
            case 3: /* TODO */ break;
            case 4: /* TODO */ break;
            case 0: std::cout << "Goodbye!\n"; break;
            default: std::cout << "Invalid choice.\n";
        }
    } while (choice != 0);
}
