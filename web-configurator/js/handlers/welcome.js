import { dom } from "../dom.js";
import { PersistenceHandler } from "../utils/storage.js";
// import { sampleConfig } from "../sampleConfig.js";

export const WelcomeHandler = {
	init: () => {
		if (!dom.welcome || !dom.welcome.loadBtn) return;
		dom.welcome.loadBtn.addEventListener("click", async () => {
			try {
				const res = await fetch("./generated-config.json");
				if (!res.ok) throw new Error("Failed to fetch generated-config.json");
				const parsed = await res.json();
				PersistenceHandler.applyConfig(parsed);
				PersistenceHandler.saveToLocal();
			} catch (e) {
				console.error("Failed applying sample config", e);
			}
			WelcomeHandler.hide();
		});
		dom.welcome.skipBtn.addEventListener("click", () => {
			WelcomeHandler.hide();
		});
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
