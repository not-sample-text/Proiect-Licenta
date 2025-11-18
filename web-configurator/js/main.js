import { dom } from "./dom.js";
import { SidebarHandler } from "./handlers/sidebar.js";
import { ModalHandler } from "./handlers/modal.js";
import { KnobHandler } from "./handlers/knob.js";
import { IOHandler } from "./handlers/io.js";
import { ResizerHandler } from "./handlers/resizer.js";
import { OLEDHandler } from "./handlers/oled.js";
import { LightingHandler } from "./handlers/lighting.js";
import { PersistenceHandler } from "./utils/storage.js";
import { state } from "./state.js";

document.addEventListener("DOMContentLoaded", () => {
	// 1. Render Initial Sidebar
	SidebarHandler.render();

	// 2. Bind Listeners
	const headers = document.querySelectorAll(
		".sidebar .category-header[data-layer]"
	);
	headers.forEach((header, index) => {
		header.addEventListener("click", () =>
			SidebarHandler.toggleLayer(index, header)
		);
	});

	dom.keys.forEach((key) => {
		key.addEventListener("click", () => {
			// ! FIX: Prevent editing on Layer 1
			if (state.activeLayerIndex === 0) return;

			const text = key.innerText;
			OLEDHandler.updateMain(text);
			// Only open modal if keys are visible
			if (
				!document
					.querySelector(".macropad-container")
					.classList.contains("hidden")
			) {
				ModalHandler.open(key.getAttribute("data-id"), text);
			}
		});
	});

	if (dom.hamburger) {
		dom.hamburger.addEventListener("click", SidebarHandler.toggleSidebar);
	}

	// 3. Initialize Sub-Systems
	ModalHandler.init();
	KnobHandler.init();
	IOHandler.init();
	ResizerHandler.init();
	LightingHandler.init();

	// 4. Load Data
	PersistenceHandler.loadFromLocal();

	// 5. Auto-open first layer
	if (headers.length > 0) headers[0].click();
});
