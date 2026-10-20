struct ControllerProbe {
    SDL_Gamepad* pad = nullptr;
    bool active = false, restoreNavigation = false, simultaneous = false;
    unsigned down = 0, released = 0, lastButtons = 0, sticks = 0, centered = 0;
    bool triggerDown[2] = {}, triggerReleased[2] = {}, triggerNeutral[2] = {};
    Uint64 nextScan = 0;
};
static ControllerProbe sControllerProbe;

static void StopControllerProbe() {
    if (sControllerProbe.restoreNavigation) ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    sControllerProbe.active = false;
    sControllerProbe.restoreNavigation = false;
}

static void ControllerProbeTick() {
    ControllerProbe& p = sControllerProbe;
    if (p.pad && !SDL_GamepadConnected(p.pad)) {
        SDL_CloseGamepad(p.pad); p.pad = nullptr;
        StopControllerProbe();
        Notice(T("Mando desconectado; prueba interrumpida.", "Controller disconnected; test interrupted."), true);
    }
    if (!p.pad && SDL_GetTicks() >= p.nextScan) {
        int count = 0;
        SDL_JoystickID* ids = SDL_GetGamepads(&count);
        if (count > 0) p.pad = SDL_OpenGamepad(ids[0]);
        SDL_free(ids);
        p.nextScan = SDL_GetTicks() + 1000;
    }
    if (!p.pad || !p.active) return;
    unsigned buttons = 0;
    for (int b = 0; b <= SDL_GAMEPAD_BUTTON_DPAD_RIGHT; ++b)
        if (KhPadButtonDown(p.pad, b, Ini("pad_layout"))) buttons |= 1u << b;
    p.down |= buttons;
    p.released |= p.lastButtons & ~buttons;
    p.lastButtons = buttons;
    int triggers[2];
    for (int t = 0; t < 2; ++t) {
        triggers[t] = SDL_GetGamepadAxis(p.pad, (SDL_GamepadAxis)(SDL_GAMEPAD_AXIS_LEFT_TRIGGER + t));
        if (triggers[t] < 2000) {
            p.triggerNeutral[t] = true;
            if (p.triggerDown[t]) p.triggerReleased[t] = true;
        }
        if (p.triggerNeutral[t] && triggers[t] > 16000) p.triggerDown[t] = true;
    }
    if (triggers[0] > 16000 && triggers[1] > 16000) p.simultaneous = true;
    for (int a = 0; a < 4; ++a) {
        int value = SDL_GetGamepadAxis(p.pad, (SDL_GamepadAxis)a);
        if (value < -16000) p.sticks |= 1u << (a * 2);
        if (value > 16000) p.sticks |= 2u << (a * 2);
        if (std::abs(value) < 4000 && ((p.sticks >> (a * 2)) & 3) == 3) p.centered |= 1u << a;
    }
}

static void ControllerProbePanel() {
    ControllerProbe& p = sControllerProbe;
    BeginCard("physical-controller");
    Eyebrow(T("COMPROBAR MANDO", "CONTROLLER CHECK"));
    if (!p.pad) {
        ImGui::TextWrapped("%s", T("SDL no detecta ningún mando. Conecta uno por USB o Bluetooth; aparecerá aquí automáticamente.",
                                  "SDL does not detect a controller. Connect one over USB or Bluetooth; it will appear here automatically."));
        ImGui::EndChild();
        return;
    }
    bool virtualPad = SDL_IsJoystickVirtual(SDL_GetGamepadID(p.pad));
    ImGui::Text("%s%s", SDL_GetGamepadName(p.pad), virtualPad ? " (SDL virtual)" : "");
    ImGui::PushFont(sFontSmall);
    ImGui::Text("VID %04X · PID %04X · SDL %d.%d.%d", SDL_GetGamepadVendor(p.pad), SDL_GetGamepadProduct(p.pad),
                SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
    ImGui::PopFont();
    if (!p.active) {
        if (ImGui::Button(T("Iniciar prueba de botones y gatillos", "Start button and trigger test"))) {
            SDL_Gamepad* pad = p.pad;
            p = ControllerProbe{}; p.pad = pad; p.active = true;
            p.restoreNavigation = (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) != 0;
            ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;
        }
    } else {
        ImGui::TextWrapped("%s", T("Pulsa y suelta cada botón, mueve ambos sticks en todas las direcciones y pulsa/suelta LT y RT, juntos y separados."
            " Usa el ratón para terminar.",
            "Press and release every button, move both sticks in every direction, and press/release LT and RT, together and separately."
            " Use the mouse to finish."));
        unsigned tested = p.down & p.released;
        int count = 0;
        for (int b = 0; b <= SDL_GAMEPAD_BUTTON_DPAD_RIGHT; ++b) if (tested & (1u << b)) ++count;
        ImGui::Text(T("Botones pulsados y liberados: %d / 15", "Buttons pressed and released: %d / 15"), count);
        for (int t = 0; t < 2; ++t) {
            char label[80];
            int value = SDL_GetGamepadAxis(p.pad, (SDL_GamepadAxis)(SDL_GAMEPAD_AXIS_LEFT_TRIGGER + t));
            std::snprintf(label, sizeof(label), "%s: %d · %s", t ? "RT" : "LT", value,
                p.triggerReleased[t] ? T("comprobado", "checked") : T("pendiente", "pending"));
            ImGui::ProgressBar((float)std::max(0, value) / 32767.0f, ImVec2(-1, Px(22)), label);
        }
        ImGui::Text(T("Sticks y centrado: %s · LT+RT simultáneos: %s", "Sticks and centering: %s · LT+RT together: %s"),
            p.centered == 15 ? "OK" : "...", p.simultaneous ? "OK" : "...");
        if (ImGui::Button(T("Guardar informe y terminar", "Save report and finish"))) {
            std::ofstream file(sRoot / "controller_report.txt");
            file << "ReKHCoM controller input observation\nDevice: " << SDL_GetGamepadName(p.pad)
                 << "\nSDL virtual: " << virtualPad << "\nVID: " << SDL_GetGamepadVendor(p.pad)
                 << "\nPID: " << SDL_GetGamepadProduct(p.pad)
                 << "\nButton down mask: " << p.down << "\nButton released mask: " << p.released
                 << "\nStick direction mask: " << p.sticks << "\nStick recentered mask: " << p.centered
                 << "\nLT press/release: " << p.triggerDown[0] << '/' << p.triggerReleased[0]
                 << "\nRT press/release: " << p.triggerDown[1] << '/' << p.triggerReleased[1]
                 << "\nLT+RT simultaneous: " << p.simultaneous
                 << "\nOnly observed inputs are recorded; this is not an automatic physical-device certification.\n";
            bool ok = (bool)file;
            StopControllerProbe();
            Notice(ok ? T("Informe guardado: controller_report.txt", "Report saved: controller_report.txt") :
                        T("No se pudo guardar el informe.", "Could not save the report."), !ok);
        }
        ImGui::SameLine();
        if (ImGui::Button(T("Cancelar prueba", "Cancel test"))) StopControllerProbe();
    }
    ImGui::EndChild();
}
