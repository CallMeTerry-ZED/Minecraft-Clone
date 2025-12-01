/*
 * © 2025 ZED Interactive. All Rights Reserved.
 */

#include "Core/Application.h"
#include <spdlog/spdlog.h>

int main()
{
    MinecraftClone::Application app;

    if (!app.Initialize())
    {
        spdlog::error("Failed to initialize application!");
        return 1;
    }

    app.Run();

    return 0;
}