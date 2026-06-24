import { configData, defaultLayer1Keys } from "../state.js";
import { KeyHandler } from "../handlers/key.js";
import { SidebarHandler } from "../handlers/sidebar.js";
import { LightingHandler } from "../handlers/lighting.js";
import { state } from "../state.js";

export const PersistenceHandler = {
    STORAGE_KEY: "macropad_config_v1",

    saveToLocal: () => {
        try {
            localStorage.setItem(
                PersistenceHandler.STORAGE_KEY,
                JSON.stringify(configData)
            );
        } catch (e) {
            console.warn("LocalStorage failed:", e);
        }
    },

    loadFromLocal: () => {
        const saved = localStorage.getItem(PersistenceHandler.STORAGE_KEY);
        if (saved) {
            try {
                let parsed = JSON.parse(saved);

                if (Array.isArray(parsed)) {
                    console.log("Migrating legacy config...");
                    parsed = {
                        layers: parsed,
                        lighting: {
                            mode: "SOLID",
                            brightness: 128,
                            speed: 10,
                            color: "#00e5ff"
                        }
                    };
                }

                PersistenceHandler.applyConfig(parsed);
                return true;
            } catch (e) {
                console.error("Corrupt data, clearing storage.", e);
                localStorage.removeItem(PersistenceHandler.STORAGE_KEY);
                return false;
            }
        }
        return false;
    },

    applyConfig: (newData) => {
        if (!newData.layers || !newData.lighting) {
            throw new Error("Invalid Config Structure");
        }

        configData.layers = newData.layers;
        configData.lighting = newData.lighting;

        const incomingLayer0 = newData.layers[0]?.keys || {};
        configData.layers[0].keys = {};

        for (const keyId in defaultLayer1Keys) {
            const customLabel = incomingLayer0[keyId]?.label;
            configData.layers[0].keys[keyId] = {
                label: customLabel
                    ? customLabel
                    : defaultLayer1Keys[keyId].label,
                value: defaultLayer1Keys[keyId].value, // FIX: Injects "F13", "F14", etc.
                type: "KEY"
            };
        }

        SidebarHandler.render();
        LightingHandler.refresh();

        if (state.activeLayerIndex !== -1) {
            KeyHandler.updateFromData(state.activeLayerIndex);
        }
    }
};
