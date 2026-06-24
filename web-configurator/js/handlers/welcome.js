import { dom } from "../dom.js";
import { TourHandler } from "./tour.js";
import { ModalHandler } from "./modal.js";

export const WelcomeHandler = {
    init: () => {
        if (!dom.welcome || !dom.welcome.overlay) return;

        const skipBtn = document.getElementById("btn-welcome-skip");
        const tourBtn = document.getElementById("btn-welcome-tour");

        if (skipBtn) {
            skipBtn.addEventListener("click", () => {
                WelcomeHandler.hide();
                // Short delay so the modal can fade out before showing the next one
                setTimeout(() => ModalHandler.showSamplePrompt(), 300);
            });
        }

        if (tourBtn) {
            tourBtn.addEventListener("click", () => {
                WelcomeHandler.hide();
                setTimeout(() => TourHandler.start(), 300);
            });
        }
    },
    show: () => {
        if (!dom.welcome || !dom.welcome.overlay) return;
        dom.welcome.overlay.classList.remove("hidden");
    },
    hide: () => {
        if (!dom.welcome || !dom.welcome.overlay) return;
        dom.welcome.overlay.classList.add("hidden");
    }
};
