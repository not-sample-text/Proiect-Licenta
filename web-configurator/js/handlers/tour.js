import { dom } from "../dom.js";
import { ModalHandler } from "./modal.js"; // ADD THIS IMPORT

export const TourHandler = {
    currentStep: 0,
    steps: [
        {
            target: "#main-sidebar ul",
            title: "Macro Layers",
            text: "Organize your workflow across 4 distinct layers. Note that Layer 1 is reserved for native F-Keys to ensure maximum OS compatibility."
        },
        {
            target: ".macropad-container",
            title: "The Visualizer",
            text: "Click any key on the board to map it to a keyboard shortcut, an absolute script path (.bat, .py), or an application executable."
        },
        {
            target: ".rotary-encoder",
            title: "Tactile Control",
            text: "Scroll the knob to adjust system volume. Click and drag left/right to toggle into Layer Navigation mode."
        },
        {
            target: "#sidebar-settings-btn",
            title: "Hardware Settings",
            text: "Access the lighting panel to customize RGB effects, breathing speeds, and brightness directly on the PCB."
        },
        {
            target: "#btn-export-json",
            title: "Daemon Sync",
            text: "Once finished, export your config to your Downloads folder. The Background Daemon will automatically detect it and sync it to the hardware!"
        }
    ],

    init: () => {
        document
            .getElementById("btn-tour-next")
            .addEventListener("click", () => {
                TourHandler.currentStep++;
                if (TourHandler.currentStep >= TourHandler.steps.length) {
                    TourHandler.end();
                } else {
                    TourHandler.renderStep();
                }
            });

        document
            .getElementById("btn-tour-close")
            .addEventListener("click", TourHandler.end);
    },

    start: () => {
        TourHandler.currentStep = 0;
        document.getElementById("tour-overlay").classList.remove("hidden");
        TourHandler.renderStep();
    },

    renderStep: () => {
        const step = TourHandler.steps[TourHandler.currentStep];
        const targetEl = document.querySelector(step.target);
        const highlight = document.getElementById("tour-highlight");
        const tooltip = document.getElementById("tour-tooltip");

        if (!targetEl) return;

        const rect = targetEl.getBoundingClientRect();
        const padding = 10;

        highlight.style.top = `${rect.top - padding}px`;
        highlight.style.left = `${rect.left - padding}px`;
        highlight.style.width = `${rect.width + padding * 2}px`;
        highlight.style.height = `${rect.height + padding * 2}px`;

        document.getElementById("tour-title").innerText = step.title;
        document.getElementById("tour-text").innerText = step.text;
        document.getElementById("tour-progress").innerText =
            `${TourHandler.currentStep + 1} / ${TourHandler.steps.length}`;

        document.getElementById("btn-tour-next").innerText =
            TourHandler.currentStep === TourHandler.steps.length - 1
                ? "Finish"
                : "Next";

        let tooltipTop = rect.bottom + 20;
        let tooltipLeft = rect.left;

        if (tooltipTop + 200 > window.innerHeight) {
            tooltipTop = rect.top - 200;
        }
        if (tooltipLeft + 300 > window.innerWidth) {
            tooltipLeft = window.innerWidth - 320;
        }

        tooltip.style.top = `${tooltipTop}px`;
        tooltip.style.left = `${tooltipLeft}px`;
    },

    end: () => {
        document.getElementById("tour-overlay").classList.add("hidden");
        // FIX: Trigger the sample prompt immediately after the tour concludes
        setTimeout(() => ModalHandler.showSamplePrompt(), 300);
    }
};
