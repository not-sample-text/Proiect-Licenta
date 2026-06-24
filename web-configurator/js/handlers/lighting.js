import { configData } from "../state.js";
import { PersistenceHandler } from "../utils/storage.js"; // ADD THIS IMPORT

export const LightingHandler = {
    init: () => {
        const els = {
            mode: document.getElementById("light-mode"),
            color: document.getElementById("light-color"),
            brightness: document.getElementById("light-brightness"),
            speed: document.getElementById("light-speed"),
            leds: document.querySelectorAll(".emitter"),
            valBright: document.getElementById("val-brightness"),
            valSpeed: document.getElementById("val-speed"),
            hexDisplay: document.getElementById("color-hex")
        };

        if (!els.mode) return;

        const updatePreview = () => {
            // ... [Keep your exact existing updatePreview code here] ...
            const cfg = configData.lighting;

            els.valBright.innerText =
                Math.round((cfg.brightness / 255) * 100) + "%";
            els.valSpeed.innerText = cfg.speed;
            els.hexDisplay.innerText = cfg.color.toUpperCase();

            els.leds.forEach((led, i) => {
                led.style.animation = "none";
                led.style.boxShadow = "none";
                led.style.backgroundColor = "#333";

                if (cfg.mode === "OFF" || cfg.brightness == "0") return;

                const opacity = cfg.brightness / 255;
                const hex = cfg.color;
                const r = parseInt(hex.slice(1, 3), 16);
                const g = parseInt(hex.slice(3, 5), 16);
                const b = parseInt(hex.slice(5, 7), 16);
                const rgba = `rgba(${r},${g},${b},${opacity})`;

                const blurRadius =
                    Math.max(5, (cfg.brightness / 255) * 20) + "px";
                const spreadRadius = (cfg.brightness / 255) * 15 + "px";

                if (cfg.mode === "SOLID") {
                    led.style.backgroundColor = rgba;
                    led.style.boxShadow = `0 0 ${blurRadius} ${spreadRadius} ${rgba}`;
                } else if (cfg.mode === "RAINBOW") {
                    led.style.backgroundColor = `hsl(${(i * 30) % 360}, 100%, 50%)`;
                    led.style.opacity = opacity;
                    led.style.boxShadow = `0 0 ${blurRadius} ${spreadRadius} currentColor`;
                } else if (cfg.mode === "BREATHING") {
                    led.style.backgroundColor = rgba;
                    led.style.boxShadow = `0 0 ${blurRadius} ${spreadRadius} ${rgba}`;
                    led.style.transition = `opacity ${2.1 - cfg.speed / 10}s ease-in-out`;
                } else if (cfg.mode === "REACTIVE") {
                    led.style.backgroundColor = "#444";
                    led.style.border = `1px solid ${rgba}`;
                }
            });
        };

        // FIX: Separate live UI updates from LocalStorage saves to prevent lag
        const applyToMemory = () => {
            configData.lighting = {
                mode: els.mode.value,
                color: els.color.value,
                brightness: els.brightness.value,
                speed: els.speed.value
            };
            updatePreview();
        };

        const saveToStorage = () => {
            PersistenceHandler.saveToLocal();
        };

        // 'input' fires continuously while dragging sliders (Updates UI smoothly)
        els.color.addEventListener("input", applyToMemory);
        els.brightness.addEventListener("input", applyToMemory);
        els.speed.addEventListener("input", applyToMemory);

        // 'change' fires only once when the user lets go of the mouse (Saves to storage)
        els.mode.addEventListener("change", () => {
            applyToMemory();
            saveToStorage();
        });
        els.color.addEventListener("change", saveToStorage);
        els.brightness.addEventListener("change", saveToStorage);
        els.speed.addEventListener("change", saveToStorage);

        // Initial Load
        const cfg = configData.lighting;
        els.mode.value = cfg.mode;
        els.color.value = cfg.color;
        els.brightness.value = cfg.brightness;
        els.speed.value = cfg.speed;

        updatePreview();
    },
    refresh: () => {
        const cfg = configData.lighting;
        const elMode = document.getElementById("light-mode");
        if (elMode) {
            elMode.value = cfg.mode;
            document.getElementById("light-color").value = cfg.color;
            document.getElementById("light-brightness").value = cfg.brightness;
            document.getElementById("light-speed").value = cfg.speed;
            elMode.dispatchEvent(new Event("change"));
        }
    }
};
