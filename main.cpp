#include <ThING/core.h>
#include <iostream>
#include "ui.h"
#include "update.h"

int main() {
    ThING::API api;
    api.setUICallback(UI);
    api.setUpdateCallback(update);
    try {
        api.run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}